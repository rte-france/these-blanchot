#include "BendersMPI.h"



BendersMpi::~BendersMpi() {

}

BendersMpi::BendersMpi() {

}

/*!
*  \brief Method to load each problem in a thread
*
*  The initialization of each problem is done sequentially 
*
*  \param problem_list : list of string containing problems' names
*/

void BendersMpi::load(problem_names const & problem_list, mpi::environment & env, mpi::communicator & world) {

	if (!problem_list.empty()) {

		std::vector<mps_order> send_orders;

		_nslaves = static_cast<int>(problem_list.size()) - 1;
		problem_names::const_iterator it(problem_list.begin());
		problem_names::const_iterator end(problem_list.end());

		int rank(1);
		if (world.rank() == 0) {

			send_orders.reserve(problem_list.size() - 1);

			_master.reset(new WorkerMaster(*it, _nslaves));

			++it;
			int i(0);
			while (it != end) {
				if (rank >= world.size()) {
					rank = 1;
				}
				send_orders.push_back({ rank, *it });
				_problem_to_id[*it] = i;
				++rank;
				++it;
				i++;
			}
		}

		bool stop_communication(false);
		std::string mps;
		std::vector<mps_order>::const_iterator it_order(send_orders.begin());
		std::vector<mps_order>::const_iterator end_order(send_orders.end());

		while (!stop_communication) {
			if (world.rank() == 0) {
				if (it_order == end_order) {
					stop_communication = true;
				}
				else {
					mps = it_order->second;
					rank = it_order->first;
					++it_order;
				}
			}
			mpi::broadcast(world, stop_communication, 0);
			world.barrier();
			if (!stop_communication) {

				mpi::broadcast(world, mps, 0);
				mpi::broadcast(world, rank, 0);

				if (world.rank() != 0) {
					if (world.rank() == rank) {
						_map_slaves[mps] = WorkerSlavePtr(new WorkerSlave(mps));
					}
				}
				world.barrier();
			}
		}
	}
}


/*!
*  \brief Method to solve, get, and send solution of a problem to every thread
*
*/

void BendersMpi::step_1(mpi::environment & env, mpi::communicator & world) {

	if (world.rank() == 0)
	{
		double alpha(0);
		double invest_cost;

		_master->solve();
		_master->get(_x0, alpha);
		_master->get_value(_lb);

		invest_cost = _lb - alpha;
		_ub = invest_cost;

	}


	broadcast(world, _x0, 0);

	world.barrier();

}

/*!
*  \brief Method to solve and get solution of every problem on each thread
*
*/
void BendersMpi::step_2(mpi::environment & env, mpi::communicator & world) {
	if (world.rank() != 0)
	{

		SlaveCutPackage slave_cut_package;

		for (auto & kvp : _map_slaves) {
			IntVector intParam(SlaveCutInt::MAXINT);
			DblVector dblParam(SlaveCutDbl::MAXDBL);
			SlaveCutDataPtr slave_cut_data(new SlaveCutData);
			SlaveCutDataHandlerPtr handler(new SlaveCutDataHandler(slave_cut_data));
			WorkerSlavePtr & ptr(kvp.second);

			ptr->fix_to(_x0);
			ptr->solve();

			ptr->get_value(handler->get_dbl(SLAVE_COST));
			ptr->get_subgradient(handler->get_subgradient());
			ptr->get_simplex_ite(handler->get_int(SIMPLEXITER));

			SlaveCutTrimmer trimmercut(handler, _x0);
			slave_cut_package[kvp.first] = *slave_cut_data;
	
		}
		gather(world, slave_cut_package, 0);
	}
	world.barrier();
}

/*!
*  \brief Method to gather all information and add cut to Master problem
*
*/
void BendersMpi::step_3(mpi::environment & env, mpi::communicator & world) {
	if (world.rank() == 0) {

		_maxsimplexiter = 0;
		_minsimplexiter = 1000000;
		_deleted_cut = 0;

		SlaveCutPackage slave_cut_package;
		std::vector<SlaveCutPackage> all_package;

		gather(world, slave_cut_package, all_package, 0);

		for (int i(1); i < world.size(); i++) {
			for (auto & itmap : all_package[i]) {
				
				SlaveCutDataPtr slave_cut_data(new SlaveCutData(itmap.second));
				SlaveCutDataHandlerPtr handler(new SlaveCutDataHandler(slave_cut_data));
				handler->get_subgradient() = itmap.second.first.first.first;
				_ub += handler->get_dbl(SLAVE_COST);

				SlaveCutTrimmer cut(handler, _x0);

				if (!(_all_cuts_storage[itmap.first].find(cut) == _all_cuts_storage[itmap.first].end())) {
					_deleted_cut++;
				}
				else {
					_master->add_cut_slave(_problem_to_id[itmap.first], handler->get_subgradient(), _x0, handler->get_dbl(SLAVE_COST));
					_all_cuts_storage[itmap.first].insert(cut);
				}

				if (_maxsimplexiter < handler->get_int(SIMPLEXITER)) {
					_maxsimplexiter = handler->get_int(SIMPLEXITER);
				}
				else if (_minsimplexiter > handler->get_int(SIMPLEXITER)) {
					_minsimplexiter = handler->get_int(SIMPLEXITER);
				}
			}	
		}

		if (_best_ub > _ub) {
			_best_ub = _ub;
			_bestx = _x0;
		}

		std::cout << std::setw(10) << _iter;
		if (_lb == -1e20)
			std::cout << std::setw(20) << "-INF";
		else
			std::cout << std::setw(20) << std::scientific << std::setprecision(10) << _lb;
		if (_ub == +1e20)
			std::cout << std::setw(20) << "+INF";
		else
			std::cout << std::setw(20) << std::scientific << std::setprecision(10) << _ub;
		if (_best_ub == +1e20)
			std::cout << std::setw(20) << "+INF";
		else
			std::cout << std::setw(20) << std::scientific << std::setprecision(10) << _best_ub;
		std::cout << std::setw(15) << _minsimplexiter;
		std::cout << std::setw(15) << _maxsimplexiter;
		std::cout << std::setw(15) << _deleted_cut;
		std::cout << std::endl;

		if (_lb + 1e-6 >= _best_ub) {
			_stop = true;
		}
	}

	broadcast(world, _stop, 0);
	world.barrier();
}

void BendersMpi::step_3_aggregated(mpi::environment & env, mpi::communicator & world) {
	if (world.rank() == 0) {

		Point s;
		double rhs;
		for (auto & var : _x0) {
			s[var.first] = 0;
		}

		_maxsimplexiter = 0;
		_minsimplexiter = 1000000;
		_deleted_cut = 0;

		SlaveCutPackage slave_cut_package;
		std::vector<SlaveCutPackage> all_package;

		gather(world, slave_cut_package, all_package, 0);

		for (int i(1); i < world.size(); i++) {
			for (auto & itmap : all_package[i]) {

				SlaveCutDataPtr slave_cut_data(new SlaveCutData(itmap.second));
				SlaveCutDataHandlerPtr handler(new SlaveCutDataHandler(slave_cut_data));
				handler->get_subgradient() = itmap.second.first.first.first;
				_ub += handler->get_dbl(SLAVE_COST);
				rhs += handler->get_dbl(SLAVE_COST);

				for (auto & var : s) {
					s[var.first] += handler->get_subgradient()[var.first];
				}

				SlaveCutTrimmer cut(handler, _x0);

				if (!(_all_cuts_storage[itmap.first].find(cut) == _all_cuts_storage[itmap.first].end())) {
					_deleted_cut++;
				}
				_master->add_cut(s, _x0, rhs);

				if (_maxsimplexiter < handler->get_int(SIMPLEXITER)) {
					_maxsimplexiter = handler->get_int(SIMPLEXITER);
				}
				else if (_minsimplexiter > handler->get_int(SIMPLEXITER)) {
					_minsimplexiter = handler->get_int(SIMPLEXITER);
				}
			}
		}

		if (_best_ub > _ub) {
			_best_ub = _ub;
			_bestx = _x0;
		}

		std::cout << std::setw(10) << _iter;
		if (_lb == -1e20)
			std::cout << std::setw(20) << "-INF";
		else
			std::cout << std::setw(20) << std::scientific << std::setprecision(10) << _lb;
		if (_ub == +1e20)
			std::cout << std::setw(20) << "+INF";
		else
			std::cout << std::setw(20) << std::scientific << std::setprecision(10) << _ub;
		if (_best_ub == +1e20)
			std::cout << std::setw(20) << "+INF";
		else
			std::cout << std::setw(20) << std::scientific << std::setprecision(10) << _best_ub;
		std::cout << std::setw(15) << _minsimplexiter;
		std::cout << std::setw(15) << _maxsimplexiter;
		std::cout << std::setw(15) << _deleted_cut;
		std::cout << std::endl;

		if (_lb + 1e-6 >= _best_ub) {
			_stop = true;
		}
	}

	broadcast(world, _stop, 0);
	world.barrier();
}


void BendersMpi::free(mpi::environment & env, mpi::communicator & world) {
	if(world.rank() == 0)
		_master->free();
	else {
		for (auto & ptr : _map_slaves)
			ptr.second->free();
	}
	world.barrier();
}
void BendersMpi::run(mpi::environment & env, mpi::communicator & world) {

	WorkerMaster & master(*_master);
	_lb = -1e20;
	_ub = +1e20;
	_x0.clear();
	_best_ub = +1e20;
	_stop = false;
	_iter = 0;
	for (auto const & kvp : _problem_to_id) {
		_all_cuts_storage[kvp.first] = SlaveCutStorage();
	}

	if (world.rank() == 0) {
		std::cout << std::setw(10) << "ITE";
		std::cout << std::setw(20) << "LB";
		std::cout << std::setw(20) << "UB";
		std::cout << std::setw(20) << "BESTUB";
		std::cout << std::setw(15) << "MINSIMPLEXIT";
		std::cout << std::setw(15) << "MAXSIMPLEXIT";
		std::cout << std::setw(15) << "DELETEDCUT";
		std::cout << std::endl;
	}

	world.barrier();

	while (!_stop) {
		++_iter;
		_deleted_cut = 0;

		/*Solve Master problem, get optimal value and cost and send it to Slaves*/
		step_1(env, world);

		/*Fix trial values in each slaves and send back data for Master to build cuts*/
		step_2(env, world);

		/*Receive datas from each slaves and add cuts to Master Problem*/
		step_3(env, world);
	}

	if (world.rank() == 0) {
		for (auto const & kvp : _bestx) {
			std::cout << std::setw(50) << std::left << kvp.first;
			std::cout << " = ";
			std::cout << std::setw(20) << std::scientific << std::setprecision(10) << kvp.second;
			std::cout << std::endl;
		}
	}

}