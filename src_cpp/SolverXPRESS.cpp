

#include "SolverXPRESS.h"
#ifdef XPRESS
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


/************************************************************************************\
* Name:         optimizermsg                                                         *
* Purpose:      Display Optimizer error messages and warnings.                       *
* Arguments:    const char *sMsg    Message string                                   *
*               int nLen            Message length                                   *
*               int nMsgLvl         Message type                                     *
* Return Value: None.                                                                *
\************************************************************************************/

void XPRS_CC optimizermsg(XPRSprob prob, void* strPtr, const char* sMsg, int nLen,
	int nMsglvl) {
	std::list<std::ostream* >* ptr = NULL;
	if (strPtr != NULL)ptr = (std::list<std::ostream* >*)strPtr;
	switch (nMsglvl) {

		/* Print Optimizer error messages and warnings */
	case 4: /* error */
	case 3: /* warning */
	case 2: /* dialogue */
	case 1: /* information */
		if (ptr != NULL) {
			for (auto const& stream : *ptr)
				* stream << sMsg << std::endl;
		}
		else {
			std::cout << sMsg << std::endl;
		}
		break;
		/* Exit and flush buffers */
	default:
		fflush(NULL);
		break;
	}
}

/************************************************************************************\
* Name:         errormsg                                                             *
* Purpose:      Display error information about failed subroutines.                  *
* Arguments:    const char *sSubName    Subroutine name                              *
*               int nLineNo             Line number                                  *
*               int nErrCode            Error code                                   *
* Return Value: None                                                                 *
\************************************************************************************/

void errormsg(XPRSprob & _xprs, const char* sSubName, int nLineNo, int nErrCode) {
	int nErrNo; /* Optimizer error number */
				/* Print error message */
	printf("The subroutine %s has failed on line %d\n", sSubName, nLineNo);

	/* Append the error code if it exists */
	if (nErrCode != -1)
		printf("with error code %d.\n\n", nErrCode);

	/* Append Optimizer error number, if available */
	if (nErrCode == 32) {
		XPRSgetintattrib(_xprs, XPRS_ERRORCODE, &nErrNo);
		printf("The Optimizer eror number is: %d\n\n", nErrNo);
	}

	/* Free memory close files and exit */
	XPRSdestroyprob(_xprs);
	XPRSfree();
	exit(nErrCode);
}

int SolverXPRESS::_NumberOfProblems = 0;
SolverXPRESS::SolverXPRESS() {
	if (_NumberOfProblems == 0) {
		XPRSinit("");
	}
	_NumberOfProblems += 1;
	_xprs = NULL;
	XPRScreateprob(&_xprs);
}

SolverXPRESS::~SolverXPRESS() {
	_NumberOfProblems -= 1;
	if (_NumberOfProblems == 0) {
		XPRSfree();
	}
}

void SolverXPRESS::init(std::string const& path_to_mps) {
	XPRSsetintcontrol(_xprs, XPRS_OUTPUTLOG, XPRS_OUTPUTLOG_NO_OUTPUT);
	XPRSsetintcontrol(_xprs, XPRS_THREADS, 1);
	XPRSsetcbmessage(_xprs, optimizermsg, &get_stream());
}

void SolverXPRESS::load_lp(const char* probname, int ncol, int nrow, const char* qrtype, const double* rhs, const double* range, const double* obj, const int* mstart, const int* mnel, const int* mrwind, const double* dmatval, const double* dlb, const double* dub)
{
	XPRSsetintcontrol(_xprs, XPRS_THREADS, 1);
	XPRSsetcbmessage(_xprs, optimizermsg, &get_stream());
	XPRSloadlp(_xprs, probname, ncol, nrow, qrtype, rhs, range, obj, mstart, mnel, mrwind, dmatval, dlb, dub);
}

void SolverXPRESS::write_prob(const char* name, const char* flags) const {
	XPRSwriteprob(_xprs, name, flags);
}

void SolverXPRESS::write_errored_prob(int status, BendersOptions const& options, std::string const& path_to_mps) const {
	
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
			XPRSwriteprob(_xprs, buffer.str().c_str(), "x");
		}
	}
	/*else if (status) {
		std::cout << "Worker::solve() status " << status << ", " << path_to_mps << std::endl;
	}*/
}

void SolverXPRESS::read_prob(const char* prob_name, const char* flags)
{
	std::string xprs_flags = "";
	if (std::string(flags) == "LP") {
		xprs_flags = "l";
	}

	XPRSreadprob(_xprs, prob_name, xprs_flags.c_str());
}

void SolverXPRESS::solve(int& lp_status, std::string const& path_to_mps) {
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
		std::cout << "XPRESS STATUS IS : " << xprs_status << std::endl;
	}
}

void SolverXPRESS::solve_integer(int& lp_status, std::string const& path_to_mps) {
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
		std::cout << "XPRESS STATUS IS : " << xprs_status << std::endl;
	}
}

void SolverXPRESS::get_obj(double* obj, int first, int last) const {
	XPRSgetobj(_xprs, obj, first, last);
}

int SolverXPRESS::get_ncols() const {
	int cols(0);
	XPRSgetintattrib(_xprs, XPRS_COLS, &cols);
	return cols;
}

int SolverXPRESS::get_nrows() const {
	int rows(0);
	XPRSgetintattrib(_xprs, XPRS_ROWS, &rows);
	return rows;
}

int SolverXPRESS::get_nelems() const {
	int elems(0);
	XPRSgetintattrib(_xprs, XPRS_ELEMS, &elems);
	std::cout << "XPRESS ELEMS = " << elems << std::endl;
	return elems;
}

void SolverXPRESS::get_rows(int* mstart, int* mclind, double* dmatval, int size, int* nels, int first, int last) const {
	XPRSgetrows(_xprs, mstart, mclind, dmatval, size, nels, first, last);
}

void SolverXPRESS::get_row_type(char* qrtype, int first, int last) const
{
	XPRSgetrowtype(_xprs, qrtype, first, last);
}

void SolverXPRESS::get_rhs(double* rhs, int first, int last) const {
	XPRSgetrhs(_xprs, rhs, first, last);
}

void SolverXPRESS::get_rhs_range(double* range, int first, int last) const {
	XPRSgetrhsrange(_xprs, range, first, last);
}

void SolverXPRESS::get_col_type(char* coltype, int first, int last) const {
	XPRSgetcoltype(_xprs, coltype, first, last);
}

void SolverXPRESS::get_lb(double* lb, int first, int last) const {
	XPRSgetlb(_xprs, lb, first, last);
}

void SolverXPRESS::get_ub(double* ub, int first, int last) const {
	XPRSgetub(_xprs, ub, first, last);
}

int SolverXPRESS::get_n_integer_vars() const
{
	int n_int_vars(0);
	XPRSgetintattrib(_xprs, XPRS_MIPENTS, &n_int_vars);
	return n_int_vars;
}

int SolverXPRESS::get_row_index(std::string const& name) const
{
	return 0;
}

int SolverXPRESS::get_col_index(std::string const& name) const
{
	return 0;
}

void SolverXPRESS::free() {
	XPRSdestroyprob(_xprs);
}

void SolverXPRESS::fix_first_stage(Point const& x0) {

}

void SolverXPRESS::add_cut(Point const& s, Point const& x0, double rhs) {

}

void SolverXPRESS::del_rows(int first, int last) {
	IntVector mindex(last - first + 1);
	for (int i = 0; i < last - first + 1; i++) {
		mindex[i] = first + i;
	}
	XPRSdelrows(_xprs, last - first + 1, mindex.data());
}

void SolverXPRESS::add_rows(int newrows, int newnz, const char* qrtype, const double* rhs, 
	const double* range, const int* mstart, const int* mclind, const double* dmatval) {
	int status = XPRSaddrows(_xprs, newrows, newnz, qrtype, rhs, range, mstart, mclind, dmatval);
}

void SolverXPRESS::add_cols(int newcol, int newnz, const double* objx, const int* mstart, const int* mrwind,
	const double* dmatval, const double* bdl, const double* bdu) {
	XPRSaddcols(_xprs, newcol, newnz, objx, mstart, mrwind, dmatval, bdl, bdu);
}

void SolverXPRESS::add_name(int type, const char* cnames, int indice) {
	XPRSaddnames(_xprs, type, cnames, indice, indice);
}

void SolverXPRESS::chg_obj(int nels, const int* mindex, const double* obj) {
	XPRSchgobj(_xprs, nels, mindex, obj);
}

void SolverXPRESS::chg_bounds(int nbds, const int* mindex, const char* qbtype, const double* bnd) {
	XPRSchgbounds(_xprs, nbds, mindex, qbtype, bnd);
}

void SolverXPRESS::chg_col_type(int nels, const int* mindex, const char* qctype) const {
	XPRSchgcoltype(_xprs, nels, mindex, qctype);
}

void SolverXPRESS::chg_rhs(int id_row, double val)
{
}

void SolverXPRESS::chg_coef(int id_row, int id_col, double val)
{
}

void SolverXPRESS::get_basis(int* rstatus, int* cstatus) const {
	XPRSgetbasis(_xprs, rstatus, cstatus);
}

void SolverXPRESS::get_value(double& lb)const  {

}

void SolverXPRESS::get_mip_value(double& lb) const {
	XPRSgetdblattrib(_xprs, XPRS_MIPOBJVAL, &lb);
}

void SolverXPRESS::get_lp_value(double& lb) const {
	XPRSgetdblattrib(_xprs, XPRS_LPOBJVAL, &lb);
}

void SolverXPRESS::get_simplex_ite(int& result) const {
	XPRSgetintattrib(_xprs, XPRS_SIMPLEXITER, &result);
}

void SolverXPRESS::get(Point& x0, double& alpha, DblVector& alpha_i) {

}

void SolverXPRESS::get_LP_sol(double* primals, double* slacks, double* duals, double* reduced_costs) {
	XPRSgetlpsol(_xprs, primals, slacks, duals, reduced_costs);
}

void SolverXPRESS::get_MIP_sol(double* primals, double* duals) {
	XPRSgetmipsol(_xprs, primals, duals);
}

void SolverXPRESS::set_output_log_level(int loglevel) {
	if (loglevel == 1 || loglevel == 3) {
		XPRSsetintcontrol(_xprs, XPRS_OUTPUTLOG, XPRS_OUTPUTLOG_FULL_OUTPUT);
	}
	else {
		XPRSsetintcontrol(_xprs, XPRS_OUTPUTLOG, XPRS_OUTPUTLOG_NO_OUTPUT);
	}
}

void SolverXPRESS::set_algorithm(std::string const& algo) {
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

void SolverXPRESS::set_threads(int n_threads)
{
	XPRSsetintcontrol(_xprs, XPRS_THREADS, n_threads);
}

void SolverXPRESS::scaling(int scale)
{
	//XPRSsetintcontrol(_xprs, XPRS_SCALING, scale);
}

void SolverXPRESS::presolve(int presolve)
{
	XPRSsetintcontrol(_xprs, XPRS_PRESOLVE, presolve);
}

void SolverXPRESS::optimality_gap(double gap)
{
	XPRSsetdblcontrol(_xprs, XPRS_OPTIMALITYTOL, gap);
}

void SolverXPRESS::set_simplex_iter(int iter)
{
}

#endif
