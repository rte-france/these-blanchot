#include "worker_mpi.h"
#include "common_mpi.h"


void mpi_problem_init(WorkerMaster master, mpi::environment & env, mpi::communicator & world) {
	if (world.rank() == 0) {
		master.write(0);

#if __ACTIVE_CHECK__
		std::cout << "Master problem is built" << std::endl;
#endif 

		std::cout << std::setw(10) << "ITE";
		std::cout << std::setw(20) << "LB";
		std::cout << std::setw(20) << "UB";
		std::cout << std::setw(20) << "BESTUB";
		std::cout << std::setw(10) << "SIMPLEXIT";
		std::cout << std::endl;
	}
}

void send_x0_master(WorkerMaster & master,  double & _lb, double & _ub, mpi::environment & env, mpi::communicator & world) {
	Point x0;
	if (world.rank() == 0)
	{
		double alpha(0);
		double invest_cost;

		master.solve();
		master.get(x0, alpha);
		master.get_value(_lb);

		invest_cost = _lb - alpha;
		_ub = invest_cost;
	}
	broadcast(world, x0, 0);
	if (world.rank() != 0) {
		
	}
}

void solve_slaves(mpi::environment & env, mpi::communicator & world) {
	if (world.rank() != 0) {
		//WorkerSlave & slave()
	}
}

 void get_cut_master(mpi::environment & env, mpi::communicator & world) {

}
