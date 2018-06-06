#pragma once

#include "BendersOptions.h"
#include "common_mpi.h"
#include "SlaveCut.h"
#include "Worker.h"
#include "WorkerSlave.h"
#include "WorkerMaster.h"
#include "BendersOptions.h"
#include "WorkerTrace.h"
#include "BendersFunctions.h"

typedef std::map<std::string, WorkerSlavePtr> Slaves_Ptr_map;

class BendersMpi {

public:

	virtual ~BendersMpi();
	BendersMpi();

	void load(CouplingMap const & problem_list, mpi::environment & env, mpi::communicator & world, BendersOptions const & options);
	
	DblVector _slave_weight_coeff;
	BendersData _data;
	BendersOptions _options;

	//AllCutStorage _all_cuts_storage;
	//WorkerMasterTrace _trace;


	Slaves_Ptr_map _map_slaves;
	WorkerMasterPtr _master;
	std::map< std::string, int> _problem_to_id;

	void run(mpi::environment & env, mpi::communicator & world, std::ostream & stream);
	void free(mpi::environment & env, mpi::communicator & world);
	void sort_cut_slave(std::vector<SlaveCutPackage> const & all_package);
	void sort_cut_slave_aggregate(std::vector<SlaveCutPackage> const & all_package);
	void step_1(mpi::environment & env, mpi::communicator & world);
	void step_2(mpi::environment & env, mpi::communicator & world);
	void update_trace();
	void init(mpi::environment & env, mpi::communicator & world, std::ostream & stream);
	void print_csv();
};
