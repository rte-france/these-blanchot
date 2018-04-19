#pragma once

#include "Worker.h"
#include "WorkerSlave.h"

class WorkerMasterTrace {
public:
	std::vector<PointPtr> _x0;
	std::vector<WorkerSlaveTracePtr> _slave_trace;
};

/*! \class WorkerMaster
* \brief Sub-class WorkerMaster
*
*  This class opens and sets a master problem from a mps and a mapping files
*/
class WorkerMaster : public Worker {
public:
	int _id_alpha;

	/*!
	*  \brief Constructor
	*
	*  Constructor of class WorkerMaster
	*
	*  \param mps : path to the mps master file
	*  \param mapping : path to the relevant mapping file
	*/
	WorkerMaster(std::string const & mps, std::string const & mapping) :Worker(mps, mapping) {
		// add the variable alpha
		std::string const alpha("alpha");
		auto const it(_name_to_id.find(alpha));
		if (it == _name_to_id.end()) {
			double lb(-1e10); /*!< Lower Bound */
			double ub(+1e20); /*!< Upper Bound*/
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

	void get(Point & x0, double & alpha);
	
	void write(int it);

	void add_cut(Point const & s, Point const & x0, double rhs);

};