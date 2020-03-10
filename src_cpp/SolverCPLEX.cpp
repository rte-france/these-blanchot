#include "SolverCPLEX.h"

// Definition of solver optimization status
StrVector CPLEX_LP_STATUS = {
	"XPRS_LP_UNSTARTED",
	"XPRS_LP_OPTIMAL",
	"XPRS_LP_INFEAS",
	"XPRS_LP_CUTOFF",
	"XPRS_LP_UNFINISHED",
	"XPRS_LP_UNBOUNDED",
	"XPRS_LP_CUTOFF_IN_DUAL",
	"XPRS_LP_UNSOLVED",
	"XPRS_LP_NONCONVEX"
};

StrVector XPRS_MIP_STATUS = {
	"XPRS_MIP_NOT_LOADED",
	"XPRS_MIP_LP_NOT_OPTIMAL",
	"XPRS_MIP_LP_OPTIMAL",
	"XPRS_MIP_NO_SOL_FOUND",
	"XPRS_MIP_SOLUTION",
	"XPRS_MIP_INFEAS",
	"XPRS_MIP_OPTIMAL",
	"XPRS_MIP_UNBOUNDED",
};


//int SolverCPLEX::_NumberOfProblems = 0;
SolverCPLEX::SolverCPLEX(std::string const& name) {
	int status(0);
	_env = CPXopenCPLEX(&status);
	_prb = CPXcreateprob(_env, &status, name.c_str());
}

SolverCPLEX::~SolverCPLEX() {
	CPXfreeprob(_env, &_prb);
	int status = CPXcloseCPLEX(&_env);
	std::cout << "CLOSE STATUS = " << status << std::endl;
}

void SolverCPLEX::init(std::string const& path_to_mps) {
	CPXsetintparam(_env, CPXPARAM_ScreenOutput, CPX_ON);
	CPXsetintparam(_env, CPXPARAM_Threads, 1);
	// Not coded in CPLEX
	//XPRSsetcbmessage(_xprs, optimizermsg, &get_stream());
}

void SolverCPLEX::load_lp(const char* probname, int ncol, int nrow, const char* qrtype, const double* rhs, const double* range, const double* obj, const int* mstart, const int* mnel, const int* mrwind, const double* dmatval, const double* dlb, const double* dub)
{
	CPXsetintparam(_env, CPXPARAM_Threads, 1);
	CPXsetintparam(_env, CPXPARAM_ScreenOutput, CPX_ON);
	//XPRSsetcbmessage(_xprs, optimizermsg, &get_stream());
	//XPRSloadlp(_xprs, probname, ncol, nrow, qrtype, rhs, range, obj, mstart, mnel, mrwind, dmatval, dlb, dub);
}

void SolverCPLEX::write_prob(const char* name, const char* flags) const {
	CPXwriteprob(_env, _prb, name, flags);
}

void SolverCPLEX::write_errored_prob(int status, BendersOptions const& options, std::string const& path_to_mps) const {
	
	int xprs_status(0);
	XPRSgetintattrib(_xprs, XPRS_LPSTATUS, &xprs_status);

	if (status != OPTIMAL) {
		std::cout << "status is : " << status << std::endl;
		std::stringstream buffer;

		std::string optim_status = "";
		if (status == OPTIMAL) {
			optim_status = SOLVER_STRING_STATUS[OPTIMAL];
		}else if(status == INFEASIBLE) {
			optim_status = SOLVER_STRING_STATUS[INFEASIBLE];
		}
		else if (status == UNBOUNDED) {
			optim_status = SOLVER_STRING_STATUS[UNBOUNDED];
		}
		else if (status == UNKNOWN) {
			optim_status = SOLVER_STRING_STATUS[UNKNOWN];
		}
		else {
			optim_status = SOLVER_STRING_STATUS[UNKNOWN];
		}

		if (options.WRITE_ERRORED_PROB) {
			buffer << path_to_mps << "status_";
			buffer << optim_status;
			buffer << ".mps";
			std::cout << "status is : " << optim_status << std::endl;
			std::cout << "written in " << buffer.str() << std::endl;
			XPRSwriteprob(_xprs, buffer.str().c_str(), "x");
		}
	}
	/*else if (status) {
		std::cout << "Worker::solve() status " << status << ", " << path_to_mps << std::endl;
	}*/
}

void SolverCPLEX::read_prob(const char* prob_name, const char* flags)
{
	XPRSreadprob(_xprs, prob_name, flags);
}

void SolverCPLEX::solve(int& lp_status, std::string const& path_to_mps) {
	int status = XPRSlpoptimize(_xprs, "");

	int xprs_status(0);
	status = XPRSgetintattrib(_xprs, XPRS_LPSTATUS, &xprs_status);
	
	if (xprs_status == XPRS_LP_OPTIMAL) {
		lp_status = OPTIMAL;
	}
	else if (xprs_status == XPRS_LP_INFEAS) {
		lp_status = INFEASIBLE;
	}
	else if (xprs_status == XPRS_LP_UNBOUNDED) {
		lp_status = UNBOUNDED;
	}
	else {
		lp_status = UNKNOWN;
	}
}

void SolverCPLEX::solve_integer(int& lp_status, std::string const& path_to_mps) {
	int status(0);
	status = XPRSmipoptimize(_xprs, "");

	int xprs_status(0);
	XPRSgetintattrib(_xprs, XPRS_MIPSTATUS, &xprs_status);

	if (xprs_status == XPRS_MIP_OPTIMAL) {
		lp_status = OPTIMAL;
	}
	else if (xprs_status == XPRS_MIP_INFEAS) {
		lp_status = INFEASIBLE;
	}
	else if (xprs_status == XPRS_MIP_UNBOUNDED) {
		lp_status = UNBOUNDED;
	}
	else {
		lp_status = UNKNOWN;
	}
}

void SolverCPLEX::get_obj(double* obj, int first, int last) const {
	XPRSgetobj(_xprs, obj, first, last);
}

int SolverCPLEX::get_ncols() const {
	int cols(0);
	XPRSgetintattrib(_xprs, XPRS_COLS, &cols);
	return cols;
}

int SolverCPLEX::get_nrows() const {
	int rows(0);
	XPRSgetintattrib(_xprs, XPRS_ROWS, &rows);
	return rows;
}

int SolverCPLEX::get_nelems() const {
	int elems(0);
	XPRSgetintattrib(_xprs, XPRS_ELEMS, &elems);
	return elems;
}

void SolverCPLEX::get_rows(int* mstart, int* mclind, double* dmatval, int size, int* nels, int first, int last) const {
	XPRSgetrows(_xprs, mstart, mclind, dmatval, size, nels, first, last);
}

void SolverCPLEX::get_row_type(char* qrtype, int first, int last) const
{
	XPRSgetrowtype(_xprs, qrtype, first, last);
}

void SolverCPLEX::get_rhs(double* rhs, int first, int last) const {
	XPRSgetrhs(_xprs, rhs, first, last);
}

void SolverCPLEX::get_rhs_range(double* range, int first, int last) const {
	XPRSgetrhsrange(_xprs, range, first, last);
}

void SolverCPLEX::get_col_type(char* coltype, int first, int last) const {
	XPRSgetcoltype(_xprs, coltype, first, last);
}

void SolverCPLEX::get_lb(double* lb, int first, int last) const {
	XPRSgetlb(_xprs, lb, first, last);
}

void SolverCPLEX::get_ub(double* ub, int first, int last) const {
	XPRSgetub(_xprs, ub, first, last);
}

int SolverCPLEX::get_n_integer_vars() const
{
	int n_int_vars(0);
	XPRSgetintattrib(_xprs, XPRS_MIPENTS, &n_int_vars);
	return n_int_vars;
}

void SolverCPLEX::free() {
	XPRSdestroyprob(_xprs);
}

void SolverCPLEX::fix_first_stage(Point const& x0) {

}

void SolverCPLEX::add_cut(Point const& s, Point const& x0, double rhs) {

}

void SolverCPLEX::del_rows(int nrows, const int* mindex) {
	XPRSdelrows(_xprs, nrows, mindex);
}

void SolverCPLEX::add_rows(int newrows, int newnz, const char* qrtype, const double* rhs,
	const double* range, const int* mstart, const int* mclind, const double* dmatval) {
	int status = XPRSaddrows(_xprs, newrows, newnz, qrtype, rhs, range, mstart, mclind, dmatval);
}

void SolverCPLEX::add_cols(int newcol, int newnz, const double* objx, const int* mstart, const int* mrwind,
	const double* dmatval, const double* bdl, const double* bdu) {
	XPRSaddcols(_xprs, newcol, newnz, objx, mstart, mrwind, dmatval, bdl, bdu);
}

void SolverCPLEX::add_names(int type, const char* cnames, int first, int last) {
	XPRSaddnames(_xprs, type, cnames, first, last);
}

void SolverCPLEX::chg_obj(int nels, const int* mindex, const double* obj) {
	XPRSchgobj(_xprs, nels, mindex, obj);
}

void SolverCPLEX::chg_bounds(int nbds, const int* mindex, const char* qbtype, const double* bnd) {
	XPRSchgbounds(_xprs, nbds, mindex, qbtype, bnd);
}

void SolverCPLEX::chg_col_type(int nels, const int* mindex, const char* qctype) const {
	XPRSchgcoltype(_xprs, nels, mindex, qctype);
}

void SolverCPLEX::get_basis(int* rstatus, int* cstatus) const {
	XPRSgetbasis(_xprs, rstatus, cstatus);
}

void SolverCPLEX::get_value(double& lb)const  {

}

void SolverCPLEX::get_mip_value(double& lb) const {
	XPRSgetdblattrib(_xprs, XPRS_MIPOBJVAL, &lb);
}

void SolverCPLEX::get_lp_value(double& lb) const {
	XPRSgetdblattrib(_xprs, XPRS_LPOBJVAL, &lb);
}

void SolverCPLEX::get_simplex_ite(int& result) const {
	XPRSgetintattrib(_xprs, XPRS_SIMPLEXITER, &result);
}

void SolverCPLEX::get(Point& x0, double& alpha, DblVector& alpha_i) {

}

void SolverCPLEX::get_LP_sol(double* primals, double* slacks, double* duals, double* reduced_costs) {
	XPRSgetlpsol(_xprs, primals, slacks, duals, reduced_costs);
}

void SolverCPLEX::get_MIP_sol(double* primals, double* duals) {
	XPRSgetmipsol(_xprs, primals, duals);
}

void SolverCPLEX::set_output_log_level(int loglevel) {
	if (loglevel == 1 || loglevel == 3) {
		CPXsetintparam(_env, CPXPARAM_ScreenOutput, CPX_ON);
	}
	else {
		CPXsetintparam(_env, CPXPARAM_ScreenOutput, CPX_OFF);
	}
}

void SolverCPLEX::set_algorithm(std::string const& algo) {
	if (algo == "BARRIER") {
		XPRSsetintcontrol(_xprs, XPRS_DEFAULTALG, 4);
	}
	else if (algo == "BARRIER_WO_CROSSOVER") {
		XPRSsetintcontrol(_xprs, XPRS_DEFAULTALG, 4);
		XPRSsetintcontrol(_xprs, XPRS_CROSSOVER, 0);
	}
	else {
		XPRSsetintcontrol(_xprs, XPRS_DEFAULTALG, 2);
	}
}

void SolverCPLEX::set_threads(int n_threads)
{
	CPXsetintparam(_env, CPXPARAM_Threads, n_threads);
}
