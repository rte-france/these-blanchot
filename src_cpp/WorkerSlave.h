#pragma once


#include "Worker.h"
#include "xprs.h"
#include "SlaveCut.h"
#include "SimplexBasis.h"

class WorkerSlave;
typedef std::shared_ptr<WorkerSlave> WorkerSlavePtr;
typedef std::vector<WorkerSlavePtr> WorkerSlaves;
typedef std::map<std::string, WorkerSlavePtr> SlavesMapPtr;


class WorkerSlave : public Worker {
public:

	WorkerSlave();
	WorkerSlave(std::map<std::string, int> const & variable_map, std::string const & path_to_mps, double const & slave_weight);
	virtual ~WorkerSlave();
	SlaveCutStorage _slave_storage;

public:

	void write(int it);

	void fix_to(Point const & x0);

	void get_subgradient(Point & s);

	SimplexBasis get_basis();

};


