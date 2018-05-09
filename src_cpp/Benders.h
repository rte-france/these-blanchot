#pragma once

#include "WorkerMaster.h"


class Benders {
public:
	Benders(problem_names const & problem_list);
	virtual ~Benders();

	WorkerMasterPtr _master;
	WorkerSlaves _slaves;

	std::stringstream _line_trace;

	WorkerMasterTrace _trace;
	AllCutStorage _all_cuts_storage;
	std::map< int, std::string> _id_to_problem;

	double _lb;
	double _ub;
	double _best_ub;

	void run();

	void free();
};