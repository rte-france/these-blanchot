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
	static int _NumberOfProblems;
public:
	XPRSprob _xprs;

public:
	SolverXPRESS();
	virtual ~SolverXPRESS();

// General methods
public:
	virtual void init(std::string const& path_to_mps);
	virtual void write_prob(const char* name, const char* flags) const;
	virtual void solve(int& lp_status, std::string const& path_to_mps);
	virtual void solve_integer(int& lp_status, std::string const& path_to_mps);
	virtual void get_obj(double* obj, int first, int last) const;
	virtual int get_ncols() const;
	virtual int get_nrows() const;
	virtual void free();

// Methods to tranform a problem
public:
	virtual void fix_first_stage(Point const& x0);
	virtual void add_cut(Point const& s, Point const& x0, double rhs);
	virtual void del_rows(int nrows, const int* mindex);
	virtual void add_rows(int newrows, int newnz, const char* qrtype, const double* rhs,
		const double* range, const int* mstart, const int* mclind, const double* dmatval);
	virtual void add_cols(int newcol, int newnz, const double* objx, const int* mstart, const int* mrwind,
		const double* dmatval, const double* bdl, const double* bdu);
	virtual void add_names(int type, const char* cnames, int first, int last);
	virtual void chg_obj(int nels, const int* mindex, const double* obj);
	virtual void chg_bounds(int nbds, const int* mindex, const char* qbtype, const double* bnd);

// Methods to get a solution
public:
	virtual void get_basis(int* rstatus, int* cstatus) const;
	virtual void get_value(double& lb) const;
	virtual void get_mip_value(double& lb) const;
	virtual void get_lp_value(double& lb) const;
	virtual void get_simplex_ite(int& result) const;

	virtual void get(Point& x0, double& alpha, DblVector& alpha_i) ;
	
	virtual void get_LP_sol(double* primals, double* slacks, double* duals, double* reduced_costs);
	virtual void get_MIP_sol(double* primals, double* duals);

// Methods to set algorithms or loglevel
	virtual void set_output_log_level(int loglevel);
	virtual void set_algorithm(std::string const& algo);
};

void errormsg(XPRSprob& xprs, const char* sSubName, int nLineNo, int nErrCode);
void optimizermsg(XPRSprob prob, void* stream, const char* sMsg, int nLen, int nMsglvl);
#endif