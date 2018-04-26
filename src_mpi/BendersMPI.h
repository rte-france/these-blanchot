#pragma once

#include "common_mpi.h"
#include "SlaveCut.h"
#include "Worker.h"
#include "WorkerSlave.h"
#include "WorkerMaster.h"



typedef std::map<std::string, WorkerSlavePtr> Slaves_Ptr_map;
typedef std::map<std::string, SlaveCutData> SlaveCutPackage;

class BendersMpi {

public:

	virtual ~BendersMpi();
	BendersMpi();

	void load(problem_names const & problem_list, mpi::environment & env, mpi::communicator & world);
	
	double _lb;
	double _ub;
	double _best_ub;
	Point _x0;
	int _nslaves;

	Slaves_Ptr_map _map_slaves;
	WorkerMasterPtr _master;

	void run(mpi::environment & env, mpi::communicator & world);

	void step_1(mpi::environment & env, mpi::communicator & world);
	void step_2(mpi::environment & env, mpi::communicator & world);
	void step_3(mpi::environment & env, mpi::communicator & world);
};
