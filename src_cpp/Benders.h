#pragma once

#include "WorkerMaster.h"

#include "BendersOptions.h"

class Benders {
public:
	Benders(problem_names const & problem_list, BendersOptions const & options);
	virtual ~Benders();

	WorkerMasterPtr _master;
	WorkerSlaves _slaves;

	std::stringstream _line_trace;

	WorkerMasterTrace _trace;
	AllCutStorage _all_cuts_storage;
	std::map< int, std::string> _id_to_problem;

	BendersOptions _options;

	double _lb;
	double _ub;
	double _best_ub;
	int _maxsimplexiter;
	int _minsimplexiter;
	int _deletedcut;
	int _it;
	bool _stop;
	double _alpha;
	double _slave_cost;
	double _invest_cost;
	Point _bestx; 
	Point _x0;
	int _nslaves;
	double _dnslaves;



	void free();

	void init_log(std::ostream&)const;
	void print_log(std::ostream&)const;
	void print_solution(std::ostream & stream) const;

	bool stopping_criterion();
	void bound_simplex_iter(int simplexiter);
	void init();
	void step_1();
	void step_2();
	void step_2_aggregate();
	void step_3();
	void run(std::ostream & stream);
};