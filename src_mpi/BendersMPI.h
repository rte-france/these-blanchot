#pragma once

#include <boost/mpi.hpp>
#include "Worker.h"

namespace mpi = boost::mpi;

class BendersMpi {
	mpi::environment env;
	mpi::communicator world;

	//BendersMpi::BendersMpi(mps_coupling_list const & mps_coupling_list) {
		//if (!mps_coupling_list.empty()) {
		//	int rank(0);
		//	while (++it != end) {
		//			++rank;
		//			std::string msg;
		//			if (world.rank() == 0) {
		//				world.send(1, 0, msg);
		//			}

		//		}
		//	}
		//	int nslaves = static_cast<int>(mps_coupling_list.size()) - 1;
		//	_slaves.reserve(nslaves);
		//	auto it(mps_coupling_list.begin());
		//	auto end(mps_coupling_list.end());
		//	_master.reset(new WorkerMaster(it->first, it->second, nslaves));


		//	if (world.rank() != 0)
		//	while (++it != end) {
		//		_slaves.push_back(WorkerSlavePtr(new WorkerSlave(it->first, it->second)));
		//	}
		//}
	//}

	//void send_value_master(mpi::environment env, mpi::communicator world) {
	//		master.get(x0, alpha);
	//		master.get_value(lb);
	//		broadcast(world, 0, { x0, alpha, 'done' });
	//};

	//void receive_cut_master(mpi::environment env, mpi::communicator world) {
	//	gathers(world, cuts, all_cuts, 0);
	//	if ('done') {
	//	master.add_cut_slave(i_slave, s, x0, rhs);
	//	}
	//};

	//void receive_value_slave(mpi::environment env, mpi::communicator world) {
	//	broadcast(world, { x0, alpha, 'done' }, 0);
	//	slave.fix_to(x0);
	//};
	//	
	//void send_cut_slave(mpi::environment env, mpi::communicator world) {
	//	slave.solve();
	//	slave.get_subgradient(s);
	//	slave.get_value(slave_cost);
	//	slave.get_value(rhs);
	//	gathers(world, {i_slave, s, x0, rhs, 'done'}, 0);
	//};
};
