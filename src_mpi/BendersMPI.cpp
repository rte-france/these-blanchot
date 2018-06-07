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
				std::cout << i << "  :  " << _options.get_slave_path(it->first)<< std::endl;
			}
			init_slave_weight(_data, _options, _slave_weight_coeff, _problem_to_id);
			_master.reset(new WorkerMaster(it_master->second, _options.get_master_path(), _slave_weight_coeff, _data.nslaves));
			std::cout << _options.get_master_path() << std::endl;
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
	//if (!problem_list.empty()) {
	//	std::vector<mps_order> send_orders;
	//	_data.nslaves = static_cast<int>(problem_list.size()) - 1;
	//	auto it(problem_list.begin());
	//	auto end(problem_list.end());
	//	_options = options;
	//	int rank(1);
	//	if (world.rank() == 0) {

	//		send_orders.reserve(problem_list.size() - 1);

	//		auto it_master = problem_list.find(_options.MASTER_NAME);
	//		std::string master_name(it_master->first);
	//		std::map<std::string, int> master_variable(problem_list.find(_options.MASTER_NAME)->second);
	//		++it;
	//		int i(0);
	//		while (it != end) {
	//			if (it == it_master) {
	//				++it;
	//			}
	//			else {
	//				if (rank >= world.size()) {
	//					rank = 1;
	//				}
	//				send_orders.push_back({ rank, it->first });
	//				_problem_to_id[it->first] = i;
	//				++rank;
	//				++it;
	//				i++;
	//			}
	//		}
	//		init_slave_weight();
	//		_master.reset(new WorkerMaster(master_variable, master_name, _slave_weight_coeff, _data.nslaves));

	//	}

	//	bool stop_communication(false);
	//	std::string mps;
	//	std::vector<mps_order>::const_iterator it_order(send_orders.begin());
	//	std::vector<mps_order>::const_iterator end_order(send_orders.end());

	//	while (!stop_communication) {
	//		if (world.rank() == 0) {
	//			if (it_order == end_order) {
	//				stop_communication = true;
	//			}
	//			else {
	//				mps = it_order->second;
	//				rank = it_order->first;
	//				++it_order;
	//			}
	//		}
	//		mpi::broadcast(world, stop_communication, 0);
	//		world.barrier();
	//		if (!stop_communication) {

	//			mpi::broadcast(world, mps, 0);
	//			mpi::broadcast(world, rank, 0);
	//			world.barrier();
	//			if (world.rank() != 0) {
	//				if (world.rank() == rank) {
	//					_map_slaves[mps] = WorkerSlavePtr(new WorkerSlave(problem_list.find(mps)->second, mps));
	//				}
	//			}
	//			world.barrier();
	//		}
	//	}
	//}
}


/*!
*  \brief Solve, get and send solution of the Master Problem to every thread
*/
void BendersMpi::step_1(mpi::environment & env, mpi::communicator & world) {

	if (world.rank() == 0)
	{
		get_master_value(_master, _data);
		if (_options.TRACE) {
			_trace._master_trace.push_back(WorkerMasterDataPtr(new WorkerMasterData));
		}
	}

	broadcast(world, _data.x0, 0);
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
		gather(world, slave_cut_package, all_package, 0);
		all_package.erase(all_package.begin());
		if (_options.AGGREGATION == 0) {
			sort_cut_slave(all_package, _slave_weight_coeff, _master, _problem_to_id, _trace, _all_cuts_storage, _data, _options);
		}
		else {
			sort_cut_slave_aggregate(all_package);
		}
	}
	else {
		get_slave_cut(slave_cut_package, _map_slaves, _options, _data);
		gather(world, slave_cut_package, 0);
	}
#if __DEBUG_BENDERS_MPI__
			std::cout << "fix_to done" << std::endl;
#endif
#if __DEBUG_BENDERS_MPI__
			std::cout << "solve done" << std::endl;
#endif
#if __DEBUG_BENDERS_MPI__
			std::cout << "get_basis done" << std::endl;
#endif
	
#if __DEBUG_BENDERS_MPI__
		std::cout << "gathering ..." << std::endl;
#endif
#if __DEBUG_BENDERS_MPI__
		std::cout << "... done" << std::endl;
#endif

	world.barrier();
}

/*!
*  \brief Add aggregated cut to Master Problem and store it in a set
*
*  Method to add aggregated cut from a slave to the Master Problem and store it in a map linking each slave to its set of cuts.
*
*  \param slave_cut_package : cut information
*/
void BendersMpi::sort_cut_slave_aggregate(std::vector<SlaveCutPackage> const & all_package) {
	Point s;
	double rhs(0);
	for (int i(0); i < all_package.size(); i++) {
		for (auto & itmap : all_package[i]) {
			SlaveCutDataPtr slave_cut_data(new SlaveCutData(itmap.second));
			SlaveCutDataHandlerPtr handler(new SlaveCutDataHandler(slave_cut_data));
			handler->get_subgradient() = itmap.second.first.first.first;

			_data.ub += handler->get_dbl(SLAVE_COST) * _slave_weight_coeff[_problem_to_id[itmap.first]];
			rhs += handler->get_dbl(SLAVE_COST) * _slave_weight_coeff[_problem_to_id[itmap.first]];

			for (auto & var : _data.x0) {
				s[var.first] += handler->get_subgradient()[var.first] * _slave_weight_coeff[_problem_to_id[itmap.first]];
			}

			//SlaveCutTrimmer cut(handler, _data.x0);


			//if (_options.DELETE_CUT && !(_all_cuts_storage[itmap.first].find(cut) == _all_cuts_storage[itmap.first].end())) {
			//	_data.deletedcut++;
			//}
			//_all_cuts_storage.find(itmap.first)->second.insert(cut);

			//if (_options.TRACE) {
			//	_trace._master_trace[_data.it - 1]->_cut_trace[itmap.first] = slave_cut_data;
			//}

			bound_simplex_iter(handler->get_int(SIMPLEXITER), _data);
		}
	}
	_master->add_cut(s, _data.x0, rhs);
}



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
*  \brief Print the trace of the Benders algorithm in a csv file
*
*  Method to print trace of the Benders algorithm in a csv file
*
* \param stream : stream to print the output
*/
//void BendersMpi::print_csv() {
//	 std::string output(_options.ROOTPATH + PATH_SEPARATOR + "bendersMpi_output.csv");
//	 if (_options.AGGREGATION) {
//		 output = (_options.ROOTPATH + PATH_SEPARATOR + "bendersMpi_output_aggregate.csv");
//	 }
//	 std::ofstream file(output, std::ios::out | std::ios::trunc);
//
//	 if (file)
//	 {
//		 file << "Ite;Worker;Problem;Id;UB;LB;bestUB;simplexiter;deletedcut" << std::endl;
//		 Point xopt;
//		 int nite;
//		 nite = _trace.get_ite();
//		 xopt = _trace._master_trace[nite - 1]->get_point();
//		 std::size_t found = _options.MASTER_NAME.find_last_of(PATH_SEPARATOR);
//		 for (int i(0); i < nite; i++) {
//			 file << i + 1 << ";";
//			 file << "Master" << ";";
//			 file << _options.MASTER_NAME.substr(found+1) << ";";
//			 file << _data.nslaves << ";";
//			 file << _trace._master_trace[i]->get_ub() << ";";
//			 file << _trace._master_trace[i]->get_lb() << ";";
//			 file << _trace._master_trace[i]->get_bestub() << ";";
//			 file << norm_point(xopt, _trace._master_trace[i]->get_point()) << ";";
//			 file << _trace._master_trace[i]->get_deletedcut() << std::endl;
//			 for (auto & kvp : _trace._master_trace[i]->_cut_trace) {
//				 SlaveCutDataHandler handler(kvp.second);
//				 file << i + 1 << ";";
//				 file << "Slave" << ";";
//				 file << kvp.first.substr(found + 1) << ";";
//				 file << _problem_to_id[kvp.first] << ";";
//				 file << handler.get_dbl(SLAVE_COST) << ";";
//				 file << handler.get_dbl(ALPHA_I) << ";";
//				 file << ";";
//				 file << handler.get_int(SIMPLEXITER) << ";";
//				 file << std::endl;
//			 }
//		 }
//		 file.close();
//	 }
//	 else {
//		 std::cout << "Impossible d'ouvrir le fichier .csv" << std::endl;
//	 }
//}
//my test !!!!

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
	}

	init(_data);

	if (world.rank() == 0) {
		init_log(stream, _options.LOG_LEVEL);
		for (auto const & kvp : _problem_to_id) {
			_all_cuts_storage[kvp.first] = SlaveCutStorage();
		}
	}
	world.barrier();

	while (!_data.stop) {
		++_data.it;
		_data.deletedcut = 0;
#if __DEBUG_BENDERS_MPI__ 
		std::cout << "new loop" << std::endl;
#endif
		/*Solve Master problem, get optimal value and cost and send it to Slaves*/
		step_1(env, world);
#if __DEBUG_BENDERS_MPI__ 
		std::cout << "step1 ended" << std::endl;
#endif

		/*Fix trial values in each slaves and send back data for Master to build cuts*/
		step_2(env, world);
#if __DEBUG_BENDERS_MPI__ 
		std::cout << "step2 ended" << std::endl;
#endif

		/*Receive datas from each slaves and add cuts to Master Problem*/
		if (world.rank() == 0) {
			update_best_ub(_data.best_ub, _data.ub, _data.bestx, _data.x0);
			print_log(stream, _data, _options.LOG_LEVEL);
			_data.stop = stopping_criterion(_data,_options);
		}

		broadcast(world, _data.stop, 0);
		world.barrier();

#if __DEBUG_BENDERS_MPI__ 
		std::cout << "step3 ended" << std::endl;
#endif

		if (world.rank() == 0) {
			/*if (_options.TRACE) {
				update_trace(_trace, _data);
			}*/
		}
		world.barrier();
	}

	if (world.rank() == 0) {
		print_solution(stream, _data.bestx, true);
		if (_options.TRACE) {
			//print_csv();
		}
	}

}
