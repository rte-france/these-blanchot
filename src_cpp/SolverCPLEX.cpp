#include "SolverCPLEX.h"

#ifdef CPLEX
// Definition of solver optimization status
StrVector CPLEX_LP_STATUS = {
		"0",
		"CPX_STAT_OPTIMAL",
		"CPX_STAT_UNBOUNDED",
		"CPX_STAT_INFEASIBLE",
		"CPX_STAT_INForUNBD",
		"CPX_STAT_OPTIMAL_INFEAS",
		"CPX_STAT_NUM_BEST"
};

int SolverCPLEX::_NumberOfProblems = 0;
SolverCPLEX::SolverCPLEX(std::string const& name) {
	int status(0);
	
	_env = CPXopenCPLEX(&status);
	_prb = CPXcreateprob(_env, &status, name.c_str());
	_NumberOfProblems += 1;
}

SolverCPLEX::~SolverCPLEX() {
	free();
	_NumberOfProblems -= 1;
	if (_NumberOfProblems == 0) {
		std::cout << "CLOSING CPLEX" << std::endl;
		int status = CPXcloseCPLEX(&_env);
	}
}

void SolverCPLEX::init(std::string const& path_to_mps) {
	CPXsetintparam(_env, CPXPARAM_ScreenOutput, CPX_OFF);
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
		else if (status == INForUNBOUND) {
			optim_status = SOLVER_STRING_STATUS[INForUNBOUND];
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
			write_prob(buffer.str().c_str(), "");
		}
	}
}

void SolverCPLEX::read_prob(const char* prob_name, const char* flags)
{
	std::string name = prob_name;
	std::string point = ".";
	std::string cpxflags;
	if (std::string(flags) == "LP") {
		cpxflags = "lp";
	}
	else if (std::string(flags) == "MPS") {
		cpxflags = "mps";
	}
	else {
		std::cout << "ERROR : Unknown file type " << flags << std::endl;
		std::exit(0);
	}
	int status = CPXreadcopyprob(_env, _prb, (name+point+ cpxflags).c_str(), flags);
}

void SolverCPLEX::solve(int& lp_status, std::string const& path_to_mps) {

	int status = CPXlpopt(_env, _prb);

	int cpx_status(0);
	cpx_status = CPXgetstat(_env, _prb);

	if (cpx_status == CPX_STAT_OPTIMAL) {
		lp_status = OPTIMAL;
	}
	else if (cpx_status == CPX_STAT_INFEASIBLE) {
		lp_status = INFEASIBLE;
	}
	else if (cpx_status == CPX_STAT_UNBOUNDED) {
		lp_status = UNBOUNDED;
	}
	else if (cpx_status == CPX_STAT_INForUNBD) {
		lp_status = INForUNBOUND;
	}
	else if(cpx_status == CPX_STAT_OPTIMAL_INFEAS) {
		DblVector solu;
		int n = get_ncols();
		solu.resize(n);
		get_LP_sol(solu.data(), NULL, NULL, NULL);

		for (int i(0); i < n; i++) {
			//std::cout << i << " " << solu[i] << std::endl;
		}
		lp_status = OPTIMAL;
	}
	else {
		lp_status = UNKNOWN;
		std::cout << "CPLEX STATUS IS : " << cpx_status << std::endl;
	}
}

void SolverCPLEX::solve_integer(int& lp_status, std::string const& path_to_mps) {

	if (get_n_integer_vars() == 0) {
		solve(lp_status, path_to_mps);
	}
	else {
		int status = CPXmipopt(_env, _prb);
		int cpx_status(0);
		cpx_status = CPXgetstat(_env, _prb);

		if (cpx_status == CPXMIP_OPTIMAL) {
			lp_status = OPTIMAL;
		}
		else if (cpx_status == CPXMIP_INFEASIBLE) {
			lp_status = INFEASIBLE;
		}
		else if (cpx_status == CPXMIP_UNBOUNDED) {
			lp_status = UNBOUNDED;
		}
		else if (cpx_status == CPXMIP_INForUNBD) {
			lp_status = INForUNBOUND;
		}
		else if (cpx_status == 102) {
			lp_status = OPTIMAL;
		}
		else {
			lp_status = UNKNOWN;
			std::cout << "CPLEX STATUS IS : " << cpx_status << std::endl;
		}
	}
}

void SolverCPLEX::get_obj(double* obj, int first, int last) const {
	CPXgetobj(_env, _prb, obj, first, last);
}

int SolverCPLEX::get_ncols() const {
	int cols(0);
	cols = CPXgetnumcols(_env, _prb);
	return cols;
}

int SolverCPLEX::get_nrows() const {
	int rows(0);
	rows = CPXgetnumrows(_env, _prb);
	return rows;
}

int SolverCPLEX::get_nelems() const {
	int elems = CPXgetnumnz(_env, _prb);
	return elems;
}

void SolverCPLEX::get_rows(int* mstart, int* mclind, double* dmatval, int size, int* nels, int first, int last) const {
	IntVector surplus;
	surplus.resize(1);
	CPXgetrows(_env, _prb, nels, mstart, mclind, dmatval, size, surplus.data(), first, last);
}

void SolverCPLEX::get_row_type(char* qrtype, int first, int last) const{
	CPXgetsense(_env, _prb, qrtype, first, last);
}

void SolverCPLEX::get_rhs(double* rhs, int first, int last) const {
	CPXgetrhs(_env, _prb, rhs, first, last);
}

void SolverCPLEX::get_rhs_range(double* range, int first, int last) const {
	CPXgetrngval(_env, _prb, range, first, last);
}

void SolverCPLEX::get_col_type(char* coltype, int first, int last) const {

	// Declaration en MIP pour creer les types de variables dans la memoire (CPLEX)
	CPXchgprobtype(_env, _prb, CPXPROB_MILP);
	int status = CPXgetctype(_env, _prb, coltype, first, last);
}

void SolverCPLEX::get_lb(double* lb, int first, int last) const {
	CPXgetlb(_env, _prb, lb, first, last);
}

void SolverCPLEX::get_ub(double* ub, int first, int last) const {
	CPXgetub(_env, _prb, ub, first, last);
}

int SolverCPLEX::get_n_integer_vars() const
{
	int n_int_vars = -1;
	n_int_vars = CPXgetnumint(_env, _prb);
	int n_bin_vars = CPXgetnumbin(_env, _prb);
	return n_int_vars + n_bin_vars;
}

void SolverCPLEX::free() {
	CPXfreeprob(_env, &_prb);
}

void SolverCPLEX::fix_first_stage(Point const& x0) {

}

void SolverCPLEX::add_cut(Point const& s, Point const& x0, double rhs) {

}

void SolverCPLEX::del_rows(int first, int last) {
	CPXdelrows(_env, _prb, first, last);
}

void SolverCPLEX::add_rows(int newrows, int newnz, const char* qrtype, const double* rhs,
	const double* range, const int* mstart, const int* mclind, const double* dmatval) {
	int status = CPXaddrows(_env, _prb, 0, newrows, newnz, rhs, qrtype, mstart, mclind, dmatval,NULL, NULL);
}

void SolverCPLEX::add_cols(int newcol, int newnz, const double* objx, const int* mstart, const int* mrwind,
	const double* dmatval, const double* bdl, const double* bdu) {
	CPXaddcols(_env, _prb, newcol, newnz, objx, mstart, mrwind, dmatval, bdl, bdu, NULL);
}

void SolverCPLEX::add_name(int type, const char* cnames, int indice) {

	if (type == 1) {
		type = 'r';
	}
	else if (type == 2) {
		type = 'c';
	}
	else {
		std::cout << "ERROR : wrong type sent to add_name" << std::endl;
		std::exit(0);
	}
	CPXchgname(_env, _prb, type, indice, cnames);
}

void SolverCPLEX::chg_obj(int nels, const int* mindex, const double* obj) {
	CPXchgobj(_env, _prb, nels, mindex, obj);
}

void SolverCPLEX::chg_bounds(int nbds, const int* mindex, const char* qbtype, const double* bnd) {
	CPXchgbds(_env, _prb, nbds, mindex, qbtype, bnd);
}

void SolverCPLEX::chg_col_type(int nels, const int* mindex, const char* qctype) const {
	CPXchgctype(_env, _prb, nels, mindex, qctype);
	if (get_n_integer_vars() == 0) {
		CPXchgprobtype(_env, _prb, CPXPROB_LP);
	}
}

void SolverCPLEX::get_basis(int* rstatus, int* cstatus) const {
	CPXgetbase(_env, _prb, cstatus, rstatus);
}

void SolverCPLEX::get_value(double& lb)const  {

}

void SolverCPLEX::get_mip_value(double& lb) const {
	CPXsolution(_env, _prb, NULL, &lb, NULL, NULL, NULL, NULL);
}

void SolverCPLEX::get_lp_value(double& lb) const {
	CPXsolution(_env, _prb, NULL, &lb, NULL, NULL, NULL, NULL);
}

void SolverCPLEX::get_simplex_ite(int& result) const {
	result = CPXgetitcnt(_env, _prb);
}

void SolverCPLEX::get(Point& x0, double& alpha, DblVector& alpha_i) {

}

void SolverCPLEX::get_LP_sol(double* primals, double* slacks, double* duals, double* reduced_costs) {
	CPXsolution(_env, _prb, NULL, NULL, primals, duals, slacks, reduced_costs);
}

void SolverCPLEX::get_MIP_sol(double* primals, double* duals) {
	CPXsolution(_env, _prb, NULL, NULL, primals, NULL, NULL, NULL);
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
		CPXsetintparam(_env, CPXPARAM_LPMethod, CPX_ALG_BARRIER);
	}
	else if (algo == "BARRIER_WO_CROSSOVER") {
		CPXsetintparam(_env, CPXPARAM_LPMethod, CPX_ALG_BARRIER);
		CPXsetintparam(_env, CPXPARAM_Barrier_Crossover, CPX_ALG_NONE);
	}
	else {
		CPXsetintparam(_env, CPXPARAM_LPMethod, CPX_ALG_DUAL);
	}
}

void SolverCPLEX::set_threads(int n_threads)
{
	CPXsetintparam(_env, CPXPARAM_Threads, n_threads);
}

void SolverCPLEX::scaling(int scale) {
	CPXsetintparam(_env, CPXPARAM_Read_Scale, scale);
}

void SolverCPLEX::presolve(int presolve) {
	int status = CPXsetintparam(_env, CPXPARAM_Preprocessing_Presolve, presolve);
}

void SolverCPLEX::optimality_gap(double gap) {
	int status = CPXsetdblparam(_env, CPXPARAM_Simplex_Tolerances_Optimality, gap);
	CPXsetdblparam(_env, CPXPARAM_Barrier_ConvergeTol, gap);
}

#endif