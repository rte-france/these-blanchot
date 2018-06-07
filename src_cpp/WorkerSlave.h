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
	WorkerSlave(std::map<std::string, int> const & variable_map, std::string const & problem_name);
	virtual ~WorkerSlave();
	SlaveCutStorage _slave_storage;
	std::vector<SimplexBasis> _basis;
	std::vector<IntVector> _gap_row_basis;
	std::vector<IntVector> _gap_col_basis;


public:

	void write(int it);

	void fix_to(Point const & x0);

	void get_subgradient(Point & s);

	void get_basis();

};


