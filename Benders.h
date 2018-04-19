#pragma once

#include "WorkerMaster.h"

typedef std::pair<std::string, std::string> mps_coupling;
typedef std::list<mps_coupling> mps_coupling_list;
typedef std::vector<WorkerSlavePtr> WorkerSlaves;

class Benders {
public:
	Benders(mps_coupling_list const & mps_coupling_list);
	virtual ~Benders();

	WorkerMasterPtr _master;
	WorkerSlaves _slaves;

	std::stringstream _line_trace;

	CutsPtr _cuts;

	WorkerMasterTrace _trace;

	double _lb;
	double _ub;
	double _best_ub;

	void run();
};