#pragma once

#include "Worker.h"
#include "xprs.h"

class WorkerSlave;
typedef std::shared_ptr<WorkerSlave> WorkerSlavePtr;


class WorkerSlave : public Worker {
public:
	WorkerSlave();
	WorkerSlave(std::string const & mps, std::string const & mapping);
	virtual ~WorkerSlave();

public:

	void write(int it);

	void fix_to(Point const & x0);

	void get_subgradient(Point & s);
};


class WorkerSlaveTrace {
public:
	// cut build alpha >= rhs+s*(x-x0)
	std::vector<PointPtr> _s;
	std::vector<double> _rhs;
};

typedef std::shared_ptr<WorkerSlaveTrace> WorkerSlaveTracePtr;