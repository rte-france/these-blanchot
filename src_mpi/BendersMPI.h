#pragma once

#include "BendersOptions.h"
#include "common_mpi.h"
#include "SlaveCut.h"
#include "Worker.h"
#include "WorkerSlave.h"
#include "WorkerMaster.h"
#include "BendersFunctions.h"

/*!
* \class BendersMpi
* \brief Class use run the benders algorithm in parallel 
*/
class BendersMpi {

public:

	virtual ~BendersMpi();
	BendersMpi(mpi::environment & env, mpi::communicator & world, BendersOptions const & options);

	void load(CouplingMap const & problem_list, mpi::environment & env, mpi::communicator & world);
	
	WorkerMasterPtr _master;
	SlavesMapPtr _map_slaves;
	
	Str2Int _problem_to_id;
	BendersData _data;
	BendersOptions _options;
	StrVector _slaves;

	AllCutStorage _all_cuts_storage;
	DynamicAggregateCuts _dynamic_aggregate_cuts;

	SlaveCutId _slave_cut_id;

	void free(mpi::environment & env, mpi::communicator & world);
	void step_1(mpi::environment & env, mpi::communicator & world);
	void step_2(mpi::environment & env, mpi::communicator & world);
	void update_random_option(mpi::environment & env, mpi::communicator & world, BendersOptions const & options, BendersData & data);
	void run(mpi::environment & env, mpi::communicator & world, std::ostream & stream);

};
