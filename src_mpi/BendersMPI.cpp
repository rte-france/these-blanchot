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
			init_slave_weight(_data, _options, _slave_weight_coeff, _problem_to_id);
			_master.reset(new WorkerMaster(it_master->second, _options.get_master_path(), _slave_weight_coeff, _data.nslaves));
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
				_map_slaves[kvp.first] = WorkerSlavePtr(new WorkerSlave(kvp.second, _options.get_slave_path(kvp.first)));
			}
		}
	}
	std::cout << "#" << world.rank() << " : " << _map_slaves.size() << std::endl;
}


/*!
*  \brief Solve, get and send solution of the Master Problem to every thread
*/
void BendersMpi::step_1(mpi::environment & env, mpi::communicator & world) {

	if (world.rank() == 0)
	{
		get_master_value(_master, _data, _options);
		if (_options.TRACE) {
			_trace.push_back(WorkerMasterDataPtr(new WorkerMasterData));
		}
		if (_options.ACTIVECUTS) {
			update_active_cuts(_master, _active_cuts, _slave_cut_id, _data.it);
		}
	}
	broadcast(world, _data.x0, 0);
	if (_options.RAND_AGGREGATION) {
		if (world.rank() == 0) {
			select_random_slaves(_problem_to_id, _options, _random_slaves);
		}
		broadcast(world, _random_slaves, 0);
	}
	world.barrier();

}

/*!
*  \brief Get cut information from each Slave and add it to the Master problem
*
*	Get cut information of every Slave Problem in each thread and send it to thread 0 to build new Master's cuts
*/
void BendersMpi::step_2(mpi::environment & env, mpi::communicator & world) {
	SlaveCutPackage slave_cut_package;
	if (world.rank() == 0) {
		std::vector<SlaveCutPackage> all_package;
		Timer timer_slaves;
		gather(world, slave_cut_package, all_package, 0);
		_data.timer_slaves = timer_slaves.elapsed();
		all_package.erase(all_package.begin());
		build_cut_full(_master, _slave_weight_coeff, all_package, _problem_to_id, _random_slaves, _trace, _slave_cut_id, _all_cuts_storage, _dynamic_aggregate_cuts, _data, _options);
	}
	else {
		if (_options.RAND_AGGREGATION) {
			get_random_slave_cut(slave_cut_package, _map_slaves, _random_slaves, _options, _data);
		}
		else {
			get_slave_cut(slave_cut_package, _map_slaves, _options, _data);
		}
		gather(world, slave_cut_package, 0);
	}
	broadcast(world, _options.RAND_AGGREGATION, 0);
	world.barrier();
}

void BendersMpi::step_3(mpi::environment & env, mpi::communicator & world) {
	SimplexBasisPackage slave_basis_package;
	if (world.rank() == 0) {
		std::vector<SimplexBasisPackage> all_basis_package;
		gather(world, slave_basis_package, all_basis_package, 0);
		all_basis_package.erase(all_basis_package.begin());
		sort_basis(all_basis_package, _problem_to_id, _basis, _data);
	}
	else {
		get_slave_basis(slave_basis_package, _map_slaves);
		gather(world, slave_basis_package, 0);
	}
	world.barrier();
}

/*!
*  \brief Method to free the memory used by each problem
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
* \param stream : stream to print the output
*/
void BendersMpi::run(mpi::environment & env, mpi::communicator & world, std::ostream & stream) {
	if (world.rank() == 0) {
		WorkerMaster & master(*_master);
		init_log(stream, _options.LOG_LEVEL);
		for (auto const & kvp : _problem_to_id) {
			_all_cuts_storage[kvp.first] = SlaveCutStorage();
		}
	}
	init(_data);
	world.barrier();

	while (!_data.stop) {
		Timer timer_master;
		++_data.it;
		_data.deletedcut = 0;

		/*Solve Master problem, get optimal value and cost and send it to Slaves*/
		step_1(env, world);

		/*Gather cut from each slave in master thread and add them to Master problem*/
		step_2(env, world);

		if (_options.BASIS) {
			step_3(env, world);
		}

		if (world.rank() == 0) {
			update_best_ub(_data.best_ub, _data.ub, _data.bestx, _data.x0);
			if (_options.TRACE) {
				update_trace(_trace, _data);
			}
			_data.timer_master = timer_master.elapsed();
			print_log(stream, _data, _options.LOG_LEVEL);
			_data.stop = stopping_criterion(_data,_options);
		}

		broadcast(world, _data.stop, 0);
		world.barrier();
	}

	if (world.rank() == 0) {
		print_solution(stream, _data.bestx, true);
		if (_options.TRACE) {
			print_csv(_trace,_problem_to_id,_data,_options);
		}
		if (_options.ACTIVECUTS) {
			print_active_cut(_active_cuts, _options);
		}
	}
}
