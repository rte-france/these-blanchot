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
			std::cout << "building master " << *it << std::endl;

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
					std::cout << "master proc " << world.rank() << " send : " << mps << " | " << rank << std::endl;
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
						std::cout << "slave proc " << world.rank() << " received : " << mps << std::endl;
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

		/*std::cout << "Upper bound : " << _ub << ", Lower bound : " << _lb << ", alpha : " << alpha << ", invest cost : " << invest_cost << std::endl;*/
		std::cout << _map_slaves.size() << std::endl;

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
			SlaveCutData slave_cut_data;
			SlaveCutDataHandler handler(slave_cut_data);
			handler.init();
			WorkerSlavePtr & ptr(kvp.second);

			
			ptr->fix_to(_x0);
			ptr->solve();

			ptr->get_value(handler.get_dbl(SLAVE_COST));
			ptr->get_subgradient(handler.get_point());
			ptr->get_simplex_ite(handler.get_int(SIMPLEXITER));

			slave_cut_package[kvp.first] = slave_cut_data;
			/*std::cout << "thread " << world.rank() << " solved " << kvp.first << " in " << handler.get_dbl(SLAVE_COST) << " simplex iterations" << std::endl;*/
			/*std::cout << "thread " << world.rank() << " stocked " << slave_cut_package.size() << " package " << std::endl;*/
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
		SlaveCutPackage slave_cut_package;
		std::vector<SlaveCutPackage> all_package;
		gather(world, slave_cut_package, all_package, 0);

		for (int i(1); i < world.size(); i++) {
			/*std::cout << "thread " << i << " sent " << all_package[i].size() << " package " << std::endl; */
			for (auto & itmap : all_package[i]) {
				SlaveCutDataHandler handler((all_package[i])[itmap.first]);
				_ub += handler.get_dbl(SLAVE_COST);
				_master->add_cut_slave(_problem_to_id[itmap.first], handler.get_point(), _x0, handler.get_dbl(SLAVE_COST));
				/*std::cout << "thread " << world.rank() << " received and add " << handler.get_dbl(SLAVE_COST) << " from " << itmap.first << std::endl;*/
				_simplexiter = handler.get_int(SIMPLEXITER);
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
		std::cout << std::setw(10) << _simplexiter;
		std::cout << std::endl;

		if (_lb + 1e-6 >= _best_ub) {
			_stop = true;
		}
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

	if (world.rank() == 0) {
		std::cout << std::setw(10) << "ITE";
		std::cout << std::setw(20) << "LB";
		std::cout << std::setw(20) << "UB";
		std::cout << std::setw(20) << "BESTUB";
		std::cout << std::setw(10) << "SIMPLEXIT";
		std::cout << std::endl;
	}

	world.barrier();

	while (!_stop) {
		++_iter;

		step_1(env, world);

		step_2(env, world);

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