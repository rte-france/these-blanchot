#pragma once

#include "BendersOptions.h"
#include "common_mpi.h"
#include "SlaveCut.h"
#include "Worker.h"
#include "WorkerSlave.h"
#include "WorkerMaster.h"

typedef std::map<std::string, WorkerSlavePtr> Slaves_Ptr_map;

class BendersMpi {

public:

	virtual ~BendersMpi();
	BendersMpi();

	void load(problem_names const & problem_list, mpi::environment & env, mpi::communicator & world, BendersOptions const & options);
	
	double _lb;
	double _ub;
	double _best_ub;
	bool _stop;
	Point _bestx;
	Point _x0;
	int _nslaves;
	int _iter;
	int _deleted_cut;
	int _maxsimplexiter;
	int _minsimplexiter;
	BendersOptions _options;

	AllCutStorage _all_cuts_storage;

	Slaves_Ptr_map _map_slaves;
	WorkerMasterPtr _master;
	std::map< std::string, int> _problem_to_id;

	void run(mpi::environment & env, mpi::communicator & world);
	void free(mpi::environment & env, mpi::communicator & world);

	void step_1(mpi::environment & env, mpi::communicator & world);
	void step_2(mpi::environment & env, mpi::communicator & world);
	void step_3(mpi::environment & env, mpi::communicator & world);
	void step_3_aggregated(mpi::environment & env, mpi::communicator & world);

};
