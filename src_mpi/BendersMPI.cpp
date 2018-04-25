#include "BendersMPI.h"


typedef std::pair<int, std::string> mps_order;


BendersMpi::~BendersMpi() {

}

BendersMpi::BendersMpi() {

}
void BendersMpi::load(problem_names const & problem_list, mpi::environment & env, mpi::communicator & world) {

	if (!problem_list.empty()) {

		std::vector<mps_order> send_orders;

		int nslaves = static_cast<int>(problem_list.size()) - 1;
		problem_names::const_iterator it(problem_list.begin());
		problem_names::const_iterator end(problem_list.end());

		int rank(1);
		if (world.rank() == 0) {
			send_orders.reserve(problem_list.size() - 1);
			std::cout << "building master " << *it << std::endl;

			_master.reset(new WorkerMaster(*it, nslaves));

			++it;

			while (it != end) {
				if (rank >= world.size()) {
					rank = 1;
				}
				send_orders.push_back({ rank, *it });
				//std::cout << world.rank() << " : " << rank << " | " << *it << std::endl;
				++rank;
				++it;
			}

			std::cout << "send_orders : " << send_orders.size() << std::endl;
		}



		bool stop_communication(false);
		std::string mps;
		std::vector<mps_order>::const_iterator it_order(send_orders.begin());
		std::vector<mps_order>::const_iterator end_order(send_orders.end());

		//for (auto const & order : send_orders) {
		//	if (world.rank() == 0) {
		//		mps = order.second;
		//		rank = order.first;
		//		std::cout << "master proc " << world.rank() << " send : " << mps << " | " << rank << std::endl;
		//	}
		//	mpi::broadcast(world, mps, 0);

		//	if (world.rank() != 0) {
		//		if (world.rank() == rank) {
		//			std::cout << "slave proc " << world.rank() << " received : " << mps << std::endl;
		//		}

		//	}
		//	world.barrier();
		//}

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

				if (world.rank() != 0) {
					if (world.rank() == rank) {
						std::cout << "slave proc " << world.rank() << " received : " << mps << std::endl;
					}

				}
				world.barrier();
			}
			//if (world.rank() == 0) {
			//	if (it_order == end_order) {
			//		stop_communication = true;
			//	}
			//	else {
			//		mps = it_order->second;
			//		rank = it_order->first;
			//		world.send(rank, 0, mps);
			//		world.send(rank, 1, rank);
			//		std::cout << "master proc " << world.rank() << " send : " << mps <<" | "<< rank << std::endl;
			//		++it_order;
			//	}
			//}
			//else {
			//	world.recv(0, 0, mps);
			//	world.recv(0, 1, rank);
			//	if (world.rank() == rank) {
			//		std::cout << "slave proc " << world.rank() << " received : " << mps << std::endl;
			//	}
			//}
		}
		//for (auto const & order : send_orders) {
		//	if (world.rank() == 0) {
		//		world.send(order.first, 0, order.second);
		//		std::cout << "master proc " << world.rank() << " send : " << *it << std::endl;
		//	}
		//	else if (world.rank() == order.first) {
		//		std::string problem_name;
		//		world.recv(0, 0, problem_name);
		//		//_map_slaves.insert(std::pair<std::string, WorkerSlavePtr>(*it, WorkerSlavePtr(new WorkerSlave(*it))));
		//		_map_slaves[problem_name] = WorkerSlavePtr(new WorkerSlave(problem_name));

		//		std::cout << "slave proc " << world.rank() << " received : " << problem_name << std::endl;
		//	}
		//	else {

		//	}
		//}
	}
}


	//if (!problem_list.empty()) {
	//	int nslaves = static_cast<int>(problem_list.size()) - 1;

	//	problem_names::const_iterator it(problem_list.begin());
	//	problem_names::const_iterator end(problem_list.end());
	//	_master.reset(new WorkerMaster(*it, nslaves));
	//	++it;
	//	int rank(1);

	//	while (it != end) {
	//		if (rank >= world.size()) {
	//			rank = 1;
	//		}
	//		if (world.rank() == 0) {
	//			world.send(rank, 0, *it);
	//			std::cout << "master proc " << world.rank() << " send : " << *it << std::endl;
	//		}
	//		else if (world.rank() == rank) {
	//			std::string problem_name;
	//			world.recv(0, 0, problem_name);
	//			//_map_slaves.insert(std::pair<std::string, WorkerSlavePtr>(*it, WorkerSlavePtr(new WorkerSlave(*it))));
	//			_map_slaves[*it] = WorkerSlavePtr(new WorkerSlave(*it));

	//			std::cout << "slave proc " << world.rank() << " received : " << problem_name << std::endl;
	//		}
	//		++rank;
	//		it++;
	//	}
//}
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
	else {
	}
	//broadcast(world, _x0, 0);
}
void BendersMpi::step_2(mpi::environment & env, mpi::communicator & world) {
	if (world.rank() != 0)
	{
		for (auto & kvp : _map_slaves) {
			WorkerSlavePtr & ptr(kvp.second);
			ptr->fix_to(_x0);
			ptr->solve();

			int simplexit(0);
			ptr->get_simplex_ite(simplexit);

			std::cout << "thread " << world.rank() << " solved " << kvp.first << " in " << simplexit << " simplex iterations" << std::endl;
		}
	}
}
void BendersMpi::run(mpi::environment & env, mpi::communicator & world) {

	WorkerMaster & master(*_master);
	_lb = -1e20;
	_ub = +1e20;
	_x0.clear();
	_best_ub = +1e20;

	bool stop = false;
	int it(0);
	int status(0);

	mpi_problem_init(master, env, world);

	//step_1(env, world);

	//step_2(env, world);


	//while (!stop) {
	//	++it;

	//	Point bestx;
	//	Point s;

	//	step_1(env, world);

	//	//step_2(env, world);

	//	std::exit(0);
	//	//send_x0_master(master, x0, _lb, _ub, env, world);

	//	//solve_slaves(env, world);

	//	//get_cut_master(env, world);

	//}
}