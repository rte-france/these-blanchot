#pragma once

#include "BendersOptions.h"
#include "common_mpi.h"
#include "SlaveCut.h"
#include "Worker.h"
#include "WorkerSlave.h"
#include "WorkerMaster.h"
#include "WorkerTrace.h"
#include "BendersFunctions.h"

class BendersMpi {

public:

	virtual ~BendersMpi();
	BendersMpi(mpi::environment & env, mpi::communicator & world, BendersOptions const & options);

	void load(CouplingMap const & problem_list, mpi::environment & env, mpi::communicator & world);
	
	WorkerMasterPtr _master;
	SlavesMapPtr _map_slaves;
	std::set<std::string> _random_slaves;
	std::set<SimplexBasisHandler> _basis;
	SlaveCutId _slave_cut_id;
	std::vector<ActiveCut> _active_cuts;
	IterAggregateCuts _dynamic_aggregate_cuts;

	std::map< std::string, int> _problem_to_id;
	BendersData _data;
	BendersOptions _options;

	AllCutStorage _all_cuts_storage;
	std::vector<WorkerMasterDataPtr> _trace;

	void run(mpi::environment & env, mpi::communicator & world, std::ostream & stream);
	void free(mpi::environment & env, mpi::communicator & world);
	void step_1(mpi::environment & env, mpi::communicator & world);
	void step_2(mpi::environment & env, mpi::communicator & world);
	void step_3(mpi::environment & env, mpi::communicator & world);
};
