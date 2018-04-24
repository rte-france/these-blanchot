#pragma once

#include "WorkerMaster.h"


class Benders {
public:
	Benders(problem_names const & problem_list);
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