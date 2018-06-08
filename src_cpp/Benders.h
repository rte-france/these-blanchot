#pragma once

#include "WorkerMaster.h"
#include "WorkerTrace.h"
#include "BendersOptions.h"
#include "BendersFunctions.h"

class Benders {
public:
	Benders(CouplingMap const & problem_list, BendersOptions const & options);
	virtual ~Benders();

	WorkerMasterPtr _master;
	SlavesMapPtr _slaves;

	std::map< std::string, int > _problem_to_id;
	DblVector _slave_weight_coeff;
	BendersData _data;
	BendersOptions _options;

	std::stringstream _line_trace;
	WorkerMasterTrace _trace;
	AllCutStorage _all_cuts_storage;

	void free();
	
	void build_cut();
	void run(std::ostream & stream);
	void print_csv();
};