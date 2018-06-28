#pragma once

#include "WorkerMaster.h"
#include "WorkerTrace.h"
#include "BendersOptions.h"
#include "BendersFunctions.h"
#include "SimplexBasis.h"

class Benders {
public:
	Benders(CouplingMap const & problem_list, BendersOptions const & options);
	virtual ~Benders();

	WorkerMasterPtr _master;
	SlavesMapPtr _slaves;

	std::map< std::string, int > _problem_to_id;
	DblVector _slave_weight_coeff;
	BendersData _data;
	BendersOptions _options;

	std::set<SimplexBasisHandler> _basis;
	SlaveCutId _slave_cut_id;
	std::vector<ActiveCut> _active_cuts;
	std::stringstream _line_trace;
	std::vector<WorkerMasterDataPtr> _trace;
	AllCutStorage _all_cuts_storage;
	DynamicAggregateCuts _dynamic_aggregate_cuts;

	void free();
	
	void build_cut();
	void run(std::ostream & stream);
};