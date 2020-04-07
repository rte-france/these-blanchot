#include "BendersMPI.h"

#define __DEBUG_BENDERS_MPI__ 0

BendersMpi::~BendersMpi() {

}

BendersMpi::BendersMpi(mpi::environment & env, mpi::communicator & world, BendersOptions const & options):_options(options) {

}

/*!
*  \brief Method to load each problem in a thread
*
*  The initialization of each problem is done sequentially
*
*  \param problem_list : map linking each problem name to its variables and their id
*
*  \param env : environment variable for mpi communication
*
*  \param world : communicator variable for mpi communication
*/

void BendersMpi::load(CouplingMap const & problem_list, mpi::environment & env, mpi::communicator & world) {
	StrVector names;
	_data.nslaves = -1;
	std::vector<CouplingMap::const_iterator> real_problem_list;
	if (!problem_list.empty()) {
		if (world.rank() == 0) {
			_data.nslaves = _options.SLAVE_NUMBER;
			if (_data.nslaves < 0) {
				_data.nslaves = problem_list.size() - 1;
			}
			std::string const & master_name(_options.MASTER_NAME);			
			auto const it_master(problem_list.find(master_name));
			if (it_master == problem_list.end()) {
				std::cout << "UNABLE TO FIND " << master_name << std::endl;
				std::exit(0);
			}
			// real problem list taking into account SLAVE_NUMBER
			
			real_problem_list.resize(_data.nslaves, problem_list.end());

			CouplingMap::const_iterator it(problem_list.begin());
			for (int i(0); i < _data.nslaves; ++it) {
				if (it != it_master) {
					real_problem_list[i] = it;
					_problem_to_id[it->first] = i;
					++i;
				}
			}
			_master.reset(new WorkerMaster(it_master->second, _options.get_master_path(), _options, _data.nslaves));

			if (_master->get_n_integer_vars() > 0) {
				if (_options.ALGORITHM == "IN-OUT") {
					std::cout << "ERROR : IN-OUT algorithm can not be used with integer problems." << std::endl;
					std::cout << "Please set alorithm to BASE." << std::endl;
					std::exit(0);
				}
			}

			std::cout << "nrealslaves is " << _data.nslaves << std::endl;
		}
		mpi::broadcast(world, _data.nslaves, 0);
		int current_worker(1);
		for (int islave(0); islave < _data.nslaves; ++islave, ++current_worker) {
			if (current_worker >= world.size()) {
				current_worker = 1;
			}
			if (world.rank() == 0) {
				CouplingMap::value_type kvp(*real_problem_list[islave]);
				//std::cout << "#" << world.rank() << " send " << kvp.first <<" | "<<islave<< std::endl;
				world.send(current_worker, islave, kvp);
			}
			else if (world.rank() == current_worker) {
				CouplingMap::value_type kvp;
				world.recv(0, islave, kvp);
				//std::cout << "#" << world.rank() << " recv " << kvp.first << " | " << islave << std::endl;
				_map_slaves[kvp.first] = WorkerSlavePtr(new WorkerSlave(kvp.second, _options.get_slave_path(kvp.first), _options.slave_weight(_data.nslaves, kvp.first), _options));
				_slaves.push_back(kvp.first);
			}
		}
	}
	std::cout << "#" << world.rank() << " : " << _map_slaves.size() << std::endl;
}

/*!
*  \brief Update the value of options RAND_AGGREGATION according to the number of slaves on each thread
*
*	Update the value of options RAND_AGGREGATION according to the number of slaves on each thread
* 
*  \param options : set of Benders options
*
*  \param options : set of Benders data
*
*  \param env : environment variable for mpi communication
*
*  \param world : communicator variable for mpi communication
*/
void BendersMpi::update_random_option(mpi::environment & env, mpi::communicator & world, BendersOptions const & options, BendersData & data) {
	int const n_thread(world.size() - 1);
	int const n_max_slave(_data.nslaves % n_thread);
	int const n_sup_rand(_options.RAND_AGGREGATION % n_thread);
	int const n_slave_by_thread(_data.nslaves / n_thread);
	int const n_rand_slave(_options.RAND_AGGREGATION / n_thread);
	int const n_add_rand(n_thread * !(n_slave_by_thread == n_rand_slave) + n_max_slave* (n_slave_by_thread == n_rand_slave));
	if (_options.RAND_AGGREGATION && world.rank() != 0) {
		_data.nrandom = n_rand_slave;
	}
	if (n_sup_rand) {
		std::set<int> set_rand_slave;
		if (world.rank() == 0) {
			for (int i(0); i < n_sup_rand;) {
				if (set_rand_slave.insert(std::rand() % n_add_rand + 1).second) {
					i++;
				}
			}
		}
		broadcast(world, set_rand_slave, 0);
		if (set_rand_slave.find(world.rank()) != set_rand_slave.end()) {
			data.nrandom++;
		}
	}
}

/*!
*  \brief Solve, get and send solution of the Master Problem to every thread
*
*  \param env : environment variable for mpi communication
*
*  \param world : communicator variable for mpi communication
*/
void BendersMpi::step_1(mpi::environment & env, mpi::communicator & world) {

	if (world.rank() == 0)
	{
		if (_options.ALGORITHM == "BASE" || _options.ALGORITHM == "IN-OUT") {
			get_master_value(_master, _data, _options);
		}
		else if (_options.ALGORITHM == "ENHANCED_MULTICUT") {
			if (_data.has_cut == true) {
				set_slaves_order(_data, _options);
				_data.n_slaves_no_cut = 0;
				get_master_value(_master, _data, _options);
				_data.has_cut = false;
			}
		}
		else {
			std::cout << "ALGORITHME NON RECONNU" << std::endl;
			std::exit(0);
		}
	}
	compute_x_cut(_options, _data);
	broadcast(world, _data.x_cut, 0);
	
	world.barrier();

}

/*!
*  \brief Get cut information from each Slave and add it to the Master problem
*
*	Get cut information of every Slave Problem in each thread and send it to thread 0 to build new Master's cuts
*
*  \param env : environment variable for mpi communication
*
*  \param world : communicator variable for mpi communication
*/
void BendersMpi::step_2(mpi::environment & env, mpi::communicator & world) {
	SlaveCutPackage slave_cut_package;
	if (world.rank() == 0) {
		AllCutPackage all_package;
		Timer timer_slaves;
		gather(world, slave_cut_package, all_package, 0);
		_data.timer_slaves = timer_slaves.elapsed();
		all_package.erase(all_package.begin());
		build_cut_full(_master, all_package, _problem_to_id, _slave_cut_id, _all_cuts_storage, _data, _options);
	}
	else {

		if (_options.ALGORITHM == "ENHANCED_MULTICUT") {
			get_random_slave_cut(slave_cut_package, _map_slaves, _slaves, _options, _data, _problem_to_id);
		}
		else {
			get_slave_cut(slave_cut_package, _map_slaves, _options, _data);
		}
		gather(world, slave_cut_package, 0);
	}
	broadcast(world, _options.RAND_AGGREGATION, 0);	
	world.barrier();
}

/*!
*  \brief Method to free the memory used by each problem
*
*  \param env : environment variable for mpi communication
*
*  \param world : communicator variable for mpi communication
*/
void BendersMpi::free(mpi::environment & env, mpi::communicator & world) {
	if (world.rank() == 0)
		_master->free();
	else {
		for (auto & ptr : _map_slaves)
			ptr.second->free();
	}
	world.barrier();
}

/*!
*  \brief Run Benders algorithm in parallel
*
*  Method to run Benders algorithm in parallel
*
*  \param env : environment variable for mpi communication
*
*  \param world : communicator variable for mpi communication
*
*  \param stream : stream to print the output
*/
void BendersMpi::run(mpi::environment & env, mpi::communicator & world, std::ostream & stream) {
	if (world.rank() == 0) {
		init_log(stream, _options.LOG_LEVEL, _options);
		for (auto const & kvp : _problem_to_id) {
			_all_cuts_storage[kvp.first] = SlaveCutStorage();
		}
	}
	init(_data, _options);
	world.barrier();

	while (!_data.stop) {
		Timer timer_master;
		update_random_option(env, world, _options, _data);
		++_data.it;
		_data.deletedcut = 0;

		/*Solve Master problem, get optimal value and cost and send it to Slaves*/
		step_1(env, world);

		/*Gather cut from each slave in master thread and add them to Master problem*/
		step_2(env, world);

		if (world.rank() == 0) {

			if (_options.ALGORITHM == "BASE" || _options.ALGORITHM == "IN-OUT") {
				compute_ub(_master, _data);
				update_in_out_stabilisation(_master, _data);
				update_best_ub(_data.best_ub, _data.ub, _data.bestx, _data.x_cut);
			}
			
			_data.timer_master = timer_master.elapsed();
			print_log(stream, _data, _options.LOG_LEVEL, _options);
			_data.stop = stopping_criterion(_data,_options);
			if (_data.stop) {
				std::cout << "ON A DIT STOP !" << std::endl;
			}
		}

		broadcast(world, _data.stop, 0);
		world.barrier();
	}

	if (world.rank() == 0) {
		print_solution(stream, _data.bestx, true, _data.global_prb_status);
	}
}
