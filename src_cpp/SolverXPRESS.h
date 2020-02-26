#pragma once

#include "SolverAbstract.h"
#ifdef XPRESS
#include "xprs.h"

void XPRS_CC optimizermsg(XPRSprob prob, void* stream, const char* sMsg, int nLen, int nMsglvl);

/*!
* \class SolverXPRESS
* \brief Class for XPRESS problems and methods 
*/
class SolverXPRESS : public SolverAbstract {
public:
	XPRSprob _xprs;

public:
	SolverXPRESS();
	~SolverXPRESS();

// General methods
public:
	void init(std::string const& path_to_mps);
	void writeprob(const char* name, const char* flags);
	void solve(int& lp_status, std::string path_to_mps);
	void solve_integer(int& lp_status, std::string path_to_mps);
	void get_obj(double* obj, int first, int last);
	void get_ncols(int& cols);
	void get_nrows(int& rows);
	void free();

// Methods to tranform a problem
public:
	void fix_first_stage(Point const& x0);
	void add_cut(Point const& s, Point const& x0, double const& rhs);
	void del_rows(int nrows, const int* mindex);
	void add_rows(int newrows, int newnz, const char* qrtype, const double* rhs,
		const double* range, const int* mstart, const int* mclind, const double* dmatval);
	void add_cols(int newcol, int newnz, const double* objx, const int* mstart, const int* mrwind,
		const double* dmatval, const double* bdl, const double* bdu);
	void add_names(int type, const char* cnames, int first, int last);
	void chgobj(int nels, const int* mindex, const double* obj);
	void chgbounds(int nbds, const int* mindex, const char* qbtype, const double* bnd);

// Methods to get a solution
public:
	void get_basis(int* rstatus, int* cstatus);
	void get_value(double& lb);
	void getmipvalue(double& lb);
	void getlpvalue(double& lb);
	void get_simplex_ite(int& result);
	void get(Point& x0, double& alpha, DblVector& alpha_i);
	
	void get_LPsol(double* primals, double* slacks, double* duals, double* reduced_costs);

	void get_MIPsol(double* primals, double* duals);

// Methods to set algorithms or loglevel
	void set_output_loglevel(int loglevel);
	void set_algorithm(std::string algo);
};

void errormsg(XPRSprob& xprs, const char* sSubName, int nLineNo, int nErrCode);
void optimizermsg(XPRSprob prob, void* stream, const char* sMsg, int nLen, int nMsglvl);
#endif