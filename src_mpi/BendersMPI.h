#pragma once

#include <boost/mpi.hpp>
#include "Worker.h"
#include "WorkerSlave.h"
#include "WorkerMaster.h"

namespace mpi = boost::mpi;

typedef std::map<std::string, WorkerSlavePtr> Slaves_Ptr_map;

class BendersMpi {
public:
	BendersMpi::BendersMpi(problem_names const & problem_list, mpi::environment & env, mpi::communicator & world);
	
	Slaves_Ptr_map _map_slaves;
	WorkerSlaves _slaves;
	WorkerMasterPtr _master;


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
