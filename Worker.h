#pragma once

#include "common.h"
#include "xprs.h"



class Worker
{
public:
	static bool IsInit;

	Worker(std::string const & mps, std::string const & mapping);
	virtual ~Worker() {

	}
public:
	std::string _path_to_mps;
	std::string _path_to_mapping;

	std::map< std::string, int> _name_to_id;
	std::map< int, std::string> _id_to_name;

public:
	void fix_to(Point const & x0);
	void get(Point & x0);
	void add_cut(Point const & s, Point const & x0, double rhs);
	void get_subgradient(Point & s);
public:
	void errormsg(const char *sSubName, int nLineNo, int nErrCode);
	std::list<std::ostream *> & stream();
public:
	XPRSprob _xprs;
	std::list<std::ostream * >_stream;
};

class WorkerSlaveTrace {
public:
	// cut build alpha >= rhs+s*(x-x0)
	std::vector<PointPtr> _s;
	std::vector<double> _rhs;
};

typedef std::shared_ptr<WorkerSlaveTrace> WorkerSlaveTracePtr;

class WorkerMasterTrace {
public:
	std::vector<PointPtr> _x0;
	std::vector<WorkerSlaveTracePtr> _slave_trace;
};

class WorkerMaster : public Worker {
public:
	WorkerMaster(std::string const & mps, std::string const & mapping) :Worker(mps, mapping) {
		// add the variable alpha
		std::string const alpha("alpha");
		auto const it(_name_to_id.find(alpha));
		if (it == _name_to_id.end()) {
			double lb(-1e10);
			double ub(+1e20);
			double obj(+1);
			int zero(0);
			std::vector<int> start(2, 0);
			XPRSaddcols(_xprs, 1, 0, &obj, start.data(), NULL, NULL, &lb, &ub);

			XPRSwriteprob(_xprs, "master_alpha.lp", "l");
		}
		else {
			std::cout << "ERROR a variable named alpha is in input" << std::endl;
		}
	}
	virtual ~WorkerMaster() {

	}
};

class WorkerSlave : public Worker {
public:
	WorkerSlave(std::string const & mps, std::string const & mapping) :Worker(mps, mapping) {

	}
	virtual ~WorkerSlave() {

	}
};

class Benders {
public:
	WorkerMaster _master;
	std::map<std::string, WorkerSlave> _slaves;

	std::stringstream _line_trace;

	CutsPtr _cuts;

	WorkerMasterTrace _trace;

	double _lb;
	double _ub;
	


	void run();
};