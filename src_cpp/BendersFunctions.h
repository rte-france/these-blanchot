#pragma once

#include "SlaveCut.h"
#include "Worker.h"
#include "WorkerSlave.h"
#include "WorkerMaster.h"
#include "WorkerTrace.h"
#include "BendersOptions.h"

void init(BendersData & data);
void init_log(std::ostream&stream, int const log_level);
void init_slave_weight(BendersData const & data, BendersOptions const & options, DblVector & slave_weight_coeff, std::map< std::string, int> & _problem_to_id);

void print_solution(std::ostream&stream, Point const & point, bool filter_non_zero);
void update_best_ub(double & best_ub, double const & ub, Point & bestx, Point const & x0);
void print_log(std::ostream&stream, BendersData const & data, int const log_level);
void bound_simplex_iter(int simplexiter, BendersData & data);
bool stopping_criterion(BendersData & data, BendersOptions const & options);
void get_master_value(WorkerMasterPtr & master, BendersData & data, BendersOptions const & options);
void get_slave_cut(SlaveCutPackage & slave_cut_package, SlavesMapPtr & map_slaves, BendersOptions const & options, BendersData const & data);
void get_random_slave_cut(SlaveCutPackage & slave_cut_package, SlavesMapPtr & map_slaves, std::set<std::string> const & random_slaves, BendersOptions const & options, BendersData const & data);
void update_trace(std::vector<WorkerMasterDataPtr> trace, BendersData const & data);
void sort_cut_slave(std::vector<SlaveCutPackage> const & all_package, WorkerMasterPtr & master, std::map<std::string, int> & problem_to_id, std::vector<WorkerMasterDataPtr> & trace, AllCutStorage & all_cuts_storage, BendersData & data, BendersOptions const & options, SlaveCutId & slave_cut_id);
void sort_cut_slave_aggregate(std::vector<SlaveCutPackage> const & all_package, WorkerMasterPtr & master, std::map<std::string, int> & problem_to_id, std::vector<WorkerMasterDataPtr> & trace, AllCutStorage & all_cuts_storage, BendersData & data, BendersOptions const & options);
void print_csv(std::vector<WorkerMasterDataPtr> & trace, std::map<std::string, int> & problem_to_id, BendersData const & data, BendersOptions const & options);
void print_cut_csv(std::ostream&stream, SlaveCutDataHandler const & handler, std::string const & name, int const islaves);
void print_master_csv(std::ostream&stream, WorkerMasterDataPtr & trace, Point const & xopt, std::string const & name, int const nslaves);
void check_status(std::vector<SlaveCutPackage> const & all_package, BendersData const & data);
void dump_cut(AllCutStorage const & cut_storage, BendersData const & data, BendersOptions const & options);
void get_slave_basis(SimplexBasisPackage & simplex_basis_package, SlavesMapPtr & map_slaves);
void sort_basis(std::vector<SimplexBasisPackage> const & all_basis_package, std::map<std::string, int> & problem_to_id, std::set<SimplexBasisHandler> & basis_storage, BendersData & data);
void update_active_cuts(WorkerMasterPtr & master, std::vector<ActiveCut> & active_cuts, SlaveCutId & cut_id, int const it);
void print_active_cut(std::vector<ActiveCut> const & active_cuts, BendersOptions const & options);
void store_current_aggregate_cut(DynamicAggregateCuts & dynamic_cuts, std::vector<SlaveCutPackage> const & all_package, std::map<std::string, int> problem_to_id, Point const & x0);
void gather_cut(DynamicAggregateCuts & dynamic_cuts, WorkerMasterPtr & master, int const it, int const nconstraints);
void select_random_slaves(std::map<std::string, int> & problem_to_id, BendersOptions const & options, std::set<std::string> & random_slaves);
void add_random_cuts(WorkerMasterPtr & master, std::vector<SlaveCutPackage> const & all_package, std::map<std::string, int> & problem_to_id, std::set<std::string> & random_slaves, std::vector<WorkerMasterDataPtr> & trace, BendersOptions & options, BendersData & data);
void add_random_aggregate_cuts(WorkerMasterPtr & master, std::vector<SlaveCutPackage> const & all_package, std::map<std::string, int> & problem_to_id, std::set<std::string> & random_slaves, std::vector<WorkerMasterDataPtr> & trace, BendersOptions & options, BendersData & data);
void build_cut_full(WorkerMasterPtr & master, std::vector<SlaveCutPackage> const & all_package, Str2Int & problem_to_id, std::set<std::string> & random_slaves, std::vector<WorkerMasterDataPtr> & trace, SlaveCutId & slave_cut_id, AllCutStorage & all_cuts_storage, DynamicAggregateCuts & dynamic_aggregate_cuts, BendersData & data, BendersOptions & options);