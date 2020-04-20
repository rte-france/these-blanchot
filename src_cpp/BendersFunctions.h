#pragma once

#include "SlaveCut.h"
#include "Worker.h"
#include "WorkerSlave.h"
#include "WorkerMaster.h"
#include "BendersOptions.h"


void init(BendersData & data,  BendersOptions const& options);
void init_log(std::ostream&stream, int const log_level, BendersOptions const& options);
void init_log_base(std::ostream& stream, int const log_level);
void init_log_inout(std::ostream& stream, int const log_level);
void init_log_enhanced_multicut(std::ostream& stream, int const log_level);

void reset_iteration_data(BendersData & data, BendersOptions const& options);

void print_log(std::ostream&stream, BendersData const & data, int const log_level, BendersOptions const& options);
void print_log_base(std::ostream& stream, BendersData const& data, int const log_level);
void print_log_inout(std::ostream& stream, BendersData const& data, int const log_level);
void print_log_enhanced_multicut(std::ostream& stream, BendersData const& data, int const log_level);

void print_cut_csv(std::ostream&stream, SlaveCutDataHandler const & handler, 
	std::string const & name, int const islaves);
void print_solution(std::ostream&stream, Point const & point, bool const filter_non_zero,int status, bool printsol);


void update_best_ub(double & best_ub, double const & ub, Point & bestx, Point const & x0);
void bound_simplex_iter(int simplexiter, BendersData & data);
bool stopping_criterion(BendersData & data, BendersOptions const & options);
void check_status(AllCutPackage const & all_package, BendersData & data);


void get_master_value(WorkerMasterPtr & master, BendersData & data, BendersOptions const & options);
int get_slave_cut(SlaveCutPackage & slave_cut_package, SlavesMapPtr & map_slaves, 
	BendersOptions const & options, BendersData & data);
int get_random_slave_cut(SlaveCutPackage & slave_cut_package, SlavesMapPtr & map_slaves, 
	StrVector const & random_slaves, BendersOptions const & options, BendersData const& data, 
	Str2Int& problem_to_id);


void sort_cut_slave(AllCutPackage const & all_package, WorkerMasterPtr & master, 
	Str2Int & problem_to_id, AllCutStorage & all_cuts_storage, BendersData & data, 
	BendersOptions const & options, SlaveCutId & slave_cut_id);
void sort_cut_slave_aggregate(AllCutPackage const & all_package, WorkerMasterPtr & master, 
	Str2Int & problem_to_id, AllCutStorage & all_cuts_storage, BendersData & data, 
	BendersOptions const & options);
void add_random_cuts(WorkerMasterPtr & master, AllCutPackage const & all_package, 
	Str2Int & problem_to_id, BendersOptions & options, BendersData & data);
void build_cut_full(WorkerMasterPtr & master, AllCutPackage const & all_package, 
	Str2Int & problem_to_id, SlaveCutId & slave_cut_id, AllCutStorage & all_cuts_storage, 
	BendersData & data, BendersOptions & options);

// in-out stabilisation
void compute_x_cut(BendersOptions const& options, BendersData& data);
void update_in_out_stabilisation(WorkerMasterPtr & _master, BendersData& data);
void compute_ub(WorkerMasterPtr& master, BendersData& data);

void set_slaves_order(BendersData& data, BendersOptions const& options);

void compute_separation_point_cost(WorkerMasterPtr& master, BendersData& data, BendersOptions const& options);
bool has_cut_master(WorkerMasterPtr& master, BendersData& data, BendersOptions const& options, int id, double val, Point subgrad);