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
	void get_value(double & lb) {
		XPRSgetdblattrib(_xprs, XPRS_LPOBJVAL, &lb);
	}
public:
	std::string _path_to_mps;
	std::string _path_to_mapping;

	std::map< std::string, int> _name_to_id;
	std::map< int, std::string> _id_to_name;
public:
	void errormsg(const char *sSubName, int nLineNo, int nErrCode);
	std::list<std::ostream *> & stream();

	void solve() {
		XPRSlpoptimize(_xprs, "");
	}
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
	int _id_alpha;
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
			XPRSgetintattrib(_xprs, XPRS_COLS, &_id_alpha);
			XPRSaddcols(_xprs, 1, 0, &obj, start.data(), NULL, NULL, &lb, &ub);
			XPRSaddnames(_xprs, 2, alpha.c_str(), _id_alpha, _id_alpha);
		}
		else {
			std::cout << "ERROR a variable named alpha is in input" << std::endl;
		}
	}
	virtual ~WorkerMaster() {

	}
	void get(Point & x0, double & alpha) {
		x0.clear();
		std::vector<double> ptr(_name_to_id.size() + 1, 0);
		int status = XPRSgetsol(_xprs, ptr.data(), NULL, NULL, NULL);
		for (auto const & kvp : _id_to_name) {
			x0[kvp.second] = ptr[kvp.first];
		}
		alpha = ptr[_id_alpha];
	}
	void write(int it) {
		std::stringstream name;
		name << "master_" << it << ".lp";
		XPRSwriteprob(_xprs, name.str().c_str(), "l");
	}
	void add_cut(Point const & s, Point const & x0, double rhs) {
		int ncols((int)_name_to_id.size());
		// cut is -rhs >= alpha  + s^(x-x0)
		int nrows(1);
		int ncoeffs(1+(int)_name_to_id.size());
		std::vector<char> rowtype(1, 'L');
		std::vector<double> rowrhs(1, 0);
		std::vector<double> matval(ncoeffs, 1);
		std::vector<int> mstart(nrows + 1, 0);
		std::vector<int> mclind(ncoeffs);

		rowrhs.front() -= rhs;

		for (auto const & kvp : _name_to_id) {
			rowrhs.front() += s.find(kvp.first)->second * x0.find(kvp.first)->second;
			mclind[kvp.second] = kvp.second;
			matval[kvp.second] = s.find(kvp.first)->second;
		}
		mclind.back() = _id_alpha;
		matval.back() = -1;
		mstart.back() = (int)matval.size();

		XPRSaddrows(_xprs, nrows, ncoeffs, rowtype.data(), rowrhs.data(), NULL, mstart.data(), mclind.data(), matval.data());
	}

};

class WorkerSlave : public Worker {
public:
	WorkerSlave(std::string const & mps, std::string const & mapping) :Worker(mps, mapping) {

	}
	virtual ~WorkerSlave() {

	}

public:

	void write(int it) {
		std::stringstream name;
		name << "slave_" << it << ".lp";
		XPRSwriteprob(_xprs, name.str().c_str(), "l");
	}
	void fix_to(Point const & x0) {
		int nbnds((int)_name_to_id.size());
		std::vector<int> indexes(nbnds);
		std::vector<char> bndtypes(nbnds, 'B');
		std::vector<double> values(nbnds);

		int i(0);
		for (auto const & kvp : _id_to_name) {
			indexes[i] = kvp.first;
			values[i] = x0.find(kvp.second)->second;
			++i;
		}

		int status = XPRSchgbounds(_xprs, nbnds, indexes.data(), bndtypes.data(), values.data());
	}
	void get_subgradient(Point & s) {
		s.clear();
		int ncols;
		XPRSgetintattrib(_xprs, XPRS_COLS, &ncols);
		std::vector<double> ptr(ncols, 0);
		int status = XPRSgetlpsol(_xprs, NULL, NULL, NULL, ptr.data());
		for (auto const & kvp : _id_to_name) {
			s[kvp.second] = +ptr[kvp.first];
		}
	}
};

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