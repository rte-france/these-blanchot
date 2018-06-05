#include "common_mpi.h"

void warning(mpi::environment & env, mpi::communicator & world, int const nslaves) {
	if (world.rank() == 0) {
		if (world.size() > nslaves + 1) {
			std::cout << " Warning : number of thread cannot exceed number of problems" << std::endl;
			std::exit(0);
		}
		{
			unsigned int nthread = std::thread::hardware_concurrency();
			if (world.size() > nthread) {
				std::cout << nthread << " concurrent threads are supported.\n" << std::endl;
				std::exit(0);
			}
		}
	}
}

