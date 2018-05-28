#pragma once


#include "Worker.h"
#include "xprs.h"
#include "SlaveCut.h"

class WorkerSlave;
typedef std::shared_ptr<WorkerSlave> WorkerSlavePtr;
typedef std::vector<WorkerSlavePtr> WorkerSlaves;

class WorkerSlave : public Worker {
public:

	WorkerSlave();
	WorkerSlave(std::string const & problem_name);
	virtual ~WorkerSlave();
	SlaveCutStorage _slave_storage;

public:

	void write(int it);

	void fix_to(Point const & x0);

	void get_subgradient(Point & s);

	void get_basis(DblVector & basis);
};
