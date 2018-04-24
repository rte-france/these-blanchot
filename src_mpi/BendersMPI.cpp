#include "BendersMPI.h"

#include <boost/mpi.hpp>
#include <iostream>
#include <string>
#include <list>
#include <boost/serialization/string.hpp>
#include <boost/serialization/utility.hpp>

namespace mpi = boost::mpi;

BendersMpi::BendersMpi(problem_names const & problem_list, mpi::environment & env, mpi::communicator & world){
	if (!problem_list.empty()) {
		int nslaves = static_cast<int>(problem_list.size()) -  1;

		problem_names::const_iterator it(problem_list.begin());
		problem_names::const_iterator end(problem_list.end());
		_master.reset(new WorkerMaster(*it, nslaves));
		++it;
		int rank(1);

		while (it != end) {
			if (rank >= world.size()) {
				rank = 1;
			}
			if (world.rank() == 0) {
				world.send(rank, 0, *it);
				std::cout << "proc " << world.rank() << " send : " << *it << std::endl;
			}
			else if (world.rank() == rank) {
				std::string problem_name;
				world.recv(0, 0, problem_name);
				_map_slaves.insert(std::pair<std::string, WorkerSlavePtr>(*it, WorkerSlavePtr(new WorkerSlave(*it))));
				//_map_slaves[*it] = WorkerSlavePtr(new WorkerSlave(*it));
				std::cout << "proc " << world.rank() << " received : " << problem_name << std::endl;
			}
			++rank;
			it++;
		}
	}
}