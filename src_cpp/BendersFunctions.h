#pragma once

#include "SlaveCut.h"
#include "Worker.h"
#include "WorkerSlave.h"
#include "WorkerMaster.h"
#include "BendersOptions.h"


void init(BendersData & data);
void init_log(std::ostream&stream, int const log_level);


void print_log(std::ostream&stream, BendersData const & data, int const log_level);
void print_cut_csv(std::ostream&stream, SlaveCutDataHandler const & handler, std::string const & name, int const islaves);
void print_solution(std::ostream&stream, Point const & point, bool const filter_non_zero,int status);
void print_active_cut(ActiveCutStorage const & active_cuts, BendersOptions const & options);


void update_best_ub(double & best_ub, double const & ub, Point & bestx, Point const & x0);
void bound_simplex_iter(int simplexiter, BendersData & data);
bool stopping_criterion(BendersData & data, BendersOptions const & options);
void check_status(AllCutPackage const & all_package, BendersData & data);


void get_master_value(WorkerMasterPtr & master, BendersData & data, BendersOptions const & options);
int get_slave_cut(SlaveCutPackage & slave_cut_package, SlavesMapPtr & map_slaves, BendersOptions const & options, BendersData const & data);
int get_random_slave_cut(SlaveCutPackage & slave_cut_package, SlavesMapPtr & map_slaves, StrVector const & random_slaves, BendersOptions const & options, BendersData const & data);


void sort_cut_slave(AllCutPackage const & all_package, WorkerMasterPtr & master, Str2Int & problem_to_id, AllCutStorage & all_cuts_storage, BendersData & data, BendersOptions const & options, SlaveCutId & slave_cut_id);
void sort_cut_slave_aggregate(AllCutPackage const & all_package, WorkerMasterPtr & master, Str2Int & problem_to_id, AllCutStorage & all_cuts_storage, BendersData & data, BendersOptions const & options);
void add_random_cuts(WorkerMasterPtr & master, AllCutPackage const & all_package, Str2Int & problem_to_id, BendersOptions & options, BendersData & data);
void build_cut_full(WorkerMasterPtr & master, AllCutPackage const & all_package, Str2Int & problem_to_id, SlaveCutId & slave_cut_id, AllCutStorage & all_cuts_storage, DynamicAggregateCuts & dynamic_aggregate_cuts, BendersData & data, BendersOptions & options);

void get_slave_basis(SimplexBasisPackage & simplex_basis_package, SlavesMapPtr & map_slaves);
void update_active_cuts(WorkerMasterPtr & master, ActiveCutStorage & active_cuts, SlaveCutId & cut_id, int const it);
