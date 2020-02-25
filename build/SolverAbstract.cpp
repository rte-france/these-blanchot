#include "SolverAbstract.h"

SolverAbstract::SolverAbstract() {}

SolverAbstract::~SolverAbstract() {}

void SolverAbstract::init(std::string const& path_to_mps) {}

void SolverAbstract::writeprob(const char* name, const char* flags) {}

void SolverAbstract::solve(int& lp_status, std::string path_to_mps) {}

void SolverAbstract::solve_integer(int& lp_status, std::string path_to_mps) {}

void SolverAbstract::get_ncols(int& cols) {}

void SolverAbstract::get_nrows(int& rows) {}

void SolverAbstract::free() {}

void SolverAbstract::fix_first_stage(Point const& x0) {}

void SolverAbstract::add_cut(Point const& s, Point const& x0, double const& rhs) {}

int SolverAbstract::del_rows(int nrows, const int* mindex) {}

void SolverAbstract::add_rows(int newrows, int newnz, const char* qrtype, const double* rhs, const double* range, const int* mstart, const int* mclind, const double* dmatval) {}

void SolverAbstract::add_cols(int newcol, int newnz, const double* objx, const int* mstart, const int* mrwind,
	const double* dmatval, const double* bdl, const double* bdu) {}

void SolverAbstract::add_names(int type, const char* cnames, int first, int last) {}

void SolverAbstract::chgbounds(int nbds, const int* mindex, const char* qbtype, const double* bnd) {}

SimplexBasis SolverAbstract::get_basis() {}

void SolverAbstract::get_value(double& lb) {}

void SolverAbstract::getmipvalue(double& lb) {}

void SolverAbstract::getlpvalue(double& lb) {}

void SolverAbstract::get_simplex_ite(int& result) {}

void SolverAbstract::get(Point& x0, double& alpha, DblVector& alpha_i) {}

void SolverAbstract::get_LPsol(double* primals, double* slacks, double* duals, double* reduced_costs) {}

void SolverAbstract::get_MIPsol(double* primals, double* duals) {}

void SolverAbstract::set_output_loglevel(int loglevel) {}

void SolverAbstract::set_algorithm(std::string algo) {}