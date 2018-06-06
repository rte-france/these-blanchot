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
	WorkerSlaves _slaves;

	std::stringstream _line_trace;
	WorkerMasterTrace _trace;
	AllCutStorage _all_cuts_storage;
	std::map< int, std::string> _id_to_problem;
	std::map< std::string, int > _problem_to_id;
	DblVector _slave_weight_coeff;

	BendersOptions _options;

	BendersData _data;

	void free();

	void init();
	
	void sort_cut(SlaveCutDataHandlerPtr & handler, int i_slave, std::string const & name_slave);
	void sort_cut_aggregate(SlaveCutDataHandlerPtr & handler, int i_slave, std::string const & name_slave, Point & s, double & rhs);
	void update_trace();
	void build_cut();
	void build_cut_aggregate();
	void run(std::ostream & stream);
	void print_csv();
};