#pragma once

#include "launcher.h"
#include "Worker.h"
#include "BendersOptions.h"
#include "BendersFunctions.h"

//void declare_solver(SolverAbstract::Ptr& solv, BendersOptions& options);

class WorkerMerge;
typedef std::shared_ptr<WorkerMerge> WorkerMergePtr;

class WorkerMerge : public Worker {
public:
	Str2Int _decalage;
	int _ncols;
	int _nslaves;
	CouplingMap _x_mps_id;

public:
	WorkerMerge(BendersOptions const& options);
	WorkerMerge(BendersOptions const & options, CouplingMap const& input, std::string const& name);
	~WorkerMerge();

public:
	void read(std::string const& path_to_mps);

public:
	void get_obj(DblVector& obj, int first, int last);
	void chg_obj(BendersOptions const& options, double weight);
	void set_decalage(std::string const& prb);
	int get_ncols();
};