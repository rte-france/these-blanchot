#pragma once

#include "common_mpi.h"
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
void get_master_value(WorkerMasterPtr & master, BendersData & data);
void get_slave_cut(SlaveCutPackage & slave_cut_package, SlavesMapPtr & map_slaves, BendersOptions const & options, BendersData const & data);
void update_trace(WorkerMasterTrace & trace, BendersData const & data);
void sort_cut_slave(std::vector<SlaveCutPackage> const & all_package, DblVector const & slave_weight_coeff, WorkerMasterPtr & master, std::map<std::string, int> & problem_to_id, WorkerMasterTrace & _trace, AllCutStorage & all_cuts_storage, BendersData & data, BendersOptions const & options);