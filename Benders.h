#pragma once

#include "WorkerMaster.h"

class Benders {
public:
	Benders(std::string const & mps_master, std::string const & mapping_master, std::string const & mps_slave, std::string const & mapping_slave) :
		_master(mps_master, mapping_master), _slave(mps_slave, mapping_slave) {

	}
	WorkerMaster _master;
	WorkerSlave _slave;

	std::stringstream _line_trace;

	CutsPtr _cuts;

	WorkerMasterTrace _trace;

	double _lb;
	double _ub;
	double _best_ub;

	void run();
};