#pragma once

#include "common_mpi.h"
#include "SlaveCut.h"
#include "Worker.h"
#include "WorkerSlave.h"
#include "WorkerMaster.h"
#include "WorkerTrace.h"
#include "BendersOptions.h"


void print_solution(std::ostream&stream, Point const & point, bool filter_non_zero);
void update_best_ub(double & best_ub, double const & ub, Point & bestx, Point const & x0);
void print_log(std::ostream&stream, BendersData const & data, int const log_level);
void init_log(std::ostream&stream, int const log_level);
void bound_simplex_iter(int simplexiter, BendersData & data);
bool stopping_criterion(BendersData & data, BendersOptions const & options);
void init_slave_weight(BendersData const & data, BendersOptions const & options, DblVector & slave_weight_coeff, std::map< std::string, int> & _problem_to_id);
void get_master_value(WorkerMasterPtr & master, BendersData & data);
void get_slave_cut(SlaveCutDataHandlerPtr & handler, WorkerSlavePtr & slave, BendersOptions const & options, BendersData const & data);