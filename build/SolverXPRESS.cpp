#include "SolverXPRESS.h"

// Definition of solver optimization status
StrVector XPRS_LP_STATUS = {
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


SolverXPRESS::SolverXPRESS() {

}

SolverXPRESS::~SolverXPRESS() {
	
}

void SolverXPRESS::init(std::string const& path_to_mps) {
	XPRScreateprob(&_xprs);
	XPRSsetintcontrol(_xprs, XPRS_OUTPUTLOG, XPRS_OUTPUTLOG_FULL_OUTPUT);
	XPRSsetintcontrol(_xprs, XPRS_OUTPUTLOG, XPRS_OUTPUTLOG_NO_OUTPUT);
	XPRSsetintcontrol(_xprs, XPRS_THREADS, 1);
	XPRSsetcbmessage(_xprs, optimizermsg, this);
	XPRSreadprob(_xprs, path_to_mps.c_str(), "");
}

void SolverXPRESS::writeprob(const char* name, const char* flags) {
	XPRSwriteprob(_xprs, name, flags);
}

void SolverXPRESS::solve(int& lp_status, std::string path_to_mps) {
	int status = XPRSlpoptimize(_xprs, "");

	status = XPRSgetintattrib(_xprs, XPRS_LPSTATUS, &lp_status);
	if (lp_status != XPRS_LP_OPTIMAL) {
		std::cout << "lp_status is : " << lp_status << std::endl;
		std::stringstream buffer;

		buffer << path_to_mps << "_lp_status_";
		buffer << XPRS_LP_STATUS[lp_status];
		buffer << ".mps";
		std::cout << "lp_status is : " << XPRS_LP_STATUS[lp_status] << std::endl;
		std::cout << "written in " << buffer.str() << std::endl;
		XPRSwriteprob(_xprs, buffer.str().c_str(), "x");
		std::exit(0);
	}
	else if (status) {
		std::cout << "Worker::solve() status " << status << ", " << path_to_mps << std::endl;
	}
}

void SolverXPRESS::solve_integer(int& lp_status, std::string path_to_mps) {
	int status(0);
	status = XPRSmipoptimize(_xprs, "");

	XPRSgetintattrib(_xprs, XPRS_MIPSTATUS, &lp_status);
	if (lp_status != XPRS_MIP_OPTIMAL && lp_status != XPRS_LP_OPTIMAL) {
		std::cout << "lp_status is : " << lp_status << std::endl;
		std::stringstream buffer;

		buffer << path_to_mps << "_lp_status_";
		buffer << XPRS_LP_STATUS[lp_status];
		buffer << ".mps";
		std::cout << "lp_status is : " << XPRS_LP_STATUS[lp_status] << std::endl;
		std::cout << "written in " << buffer.str() << std::endl;
		XPRSwriteprob(_xprs, buffer.str().c_str(), "x");
		std::exit(0);
	}
	else if (status) {
		std::cout << "Worker::solve() status " << status << ", " << path_to_mps << std::endl;
	}
}

void SolverXPRESS::get_ncols(int& cols) {
	XPRSgetintattrib(_xprs, XPRS_COLS, &cols);
}

void SolverXPRESS::get_nrows(int& rows){
	XPRSgetintattrib(_xprs, XPRS_ROWS, &rows);
}

void SolverXPRESS::free() {
	XPRSdestroyprob(_xprs);
}

void SolverXPRESS::fix_first_stage(Point const& x0) {

}

void SolverXPRESS::add_cut(Point const& s, Point const& x0, double const& rhs) {

}

int SolverXPRESS::del_rows(int nrows, const int* mindex) {
	XPRSdelrows(_xprs, nrows, mindex);
}

void SolverXPRESS::add_rows(int newrows, int newnz, const char* qrtype, const double* rhs, 
	const double* range, const int* mstart, const int* mclind, const double* dmatval) {
	XPRSaddrows(_xprs, newrows, newnz, qrtype, rhs, range, mstart, mclind, dmatval);
}

void SolverXPRESS::add_cols(int newcol, int newnz, const double* objx, const int* mstart, const int* mrwind,
	const double* dmatval, const double* bdl, const double* bdu) {
	XPRSaddcols(_xprs, newcol, newnz, objx, mstart, mrwind, dmatval, bdl, bdu);
}

void SolverXPRESS::add_names(int type, const char* cnames, int first, int last) {
	XPRSaddnames(_xprs, type, cnames, first, last);
}

void SolverXPRESS::chgbounds(int nbds, const int* mindex, const char* qbtype, const double* bnd) {
	XPRSchgbounds(_xprs, nbds, mindex, qbtype, bnd);
}

SimplexBasis SolverXPRESS::get_basis() {

}

void SolverXPRESS::get_value(double& lb) {

}

void SolverXPRESS::getmipvalue(double& lb) {
	XPRSgetdblattrib(_xprs, XPRS_MIPOBJVAL, &lb);
}

void SolverXPRESS::getlpvalue(double& lb) {
	XPRSgetdblattrib(_xprs, XPRS_LPOBJVAL, &lb);
}

void SolverXPRESS::get_simplex_ite(int& result) {
	XPRSgetintattrib(_xprs, XPRS_SIMPLEXITER, &result);
}

void SolverXPRESS::get(Point& x0, double& alpha, DblVector& alpha_i) {

}

void SolverXPRESS::get_LPsol(double* primals, double* slacks, double* duals, double* reduced_costs) {
	XPRSgetlpsol(_xprs, primals, slacks, duals, reduced_costs);
}

void SolverXPRESS::get_MIPsol(double* primals, double* duals) {
	XPRSgetmipsol(_xprs, primals, duals);
}

void SolverXPRESS::set_output_loglevel(int loglevel) {
	if (loglevel == 1 || loglevel == 3) {
		XPRSsetintcontrol(_xprs, XPRS_OUTPUTLOG, XPRS_OUTPUTLOG_FULL_OUTPUT);
	}
	else {
		XPRSsetintcontrol(_xprs, XPRS_OUTPUTLOG, XPRS_OUTPUTLOG_NO_OUTPUT);
	}
}

void SolverXPRESS::set_algorithm(std::string algo) {
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