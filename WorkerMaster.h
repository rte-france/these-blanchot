#pragma once

#include "Worker.h"
#include "WorkerSlaves.h"

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
	WorkerMaster(std::string const & mps, std::string const & mapping) :Worker(mps, mapping);
	
	virtual ~WorkerMaster() {

	}

	void get(Point & x0, double & alpha);
	
	void write(int it);

	void add_cut(Point const & s, Point const & x0, double rhs);

};