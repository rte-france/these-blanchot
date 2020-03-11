#pragma once

#include "SolverAbstract.h"
#ifdef CPLEX
#include "cplex.h"

/*!
* \class SolverCPLEX
* \brief Class for CPLEX problems and methods 
*/
class SolverCPLEX : public SolverAbstract {
	static int _NumberOfProblems;	/*!< Counter of the total number of XPRESS problems declared to set or end the environment */
public:
	CPXENVptr _env;				/*!< Ptr to the CPLEX environment */
	CPXLPptr _prb;				/*!< Ptr to the CPLEX problem */
	//StrVector CPLEX_LP_STATUS;	/*!< CPLEX lp status */

public:
	SolverCPLEX(std::string const& name);
	virtual ~SolverCPLEX();

// General methods
public:
	virtual void init(std::string const& path_to_mps);
	virtual void load_lp(const char* probname, int ncol, int nrow,
		const char* qrtype, const double* rhs, const double* range,
		const double* obj, const int* mstart, const int* mnel, const int* mrwind,
		const double* dmatval, const double* dlb, const double* dub);
	virtual void write_prob(const char* name, const char* flags) const;
	virtual void write_errored_prob(int status, BendersOptions const& options, std::string const& path_to_mps) const;
	virtual void read_prob(const char* prob_name, const char* flags);
	virtual void solve(int& lp_status, std::string const& path_to_mps);
	virtual void solve_integer(int& lp_status, std::string const& path_to_mps);
	virtual void get_obj(double* obj, int first, int last) const;
	virtual int get_ncols() const;
	virtual int get_nrows() const;
	virtual int get_nelems() const;
	
	virtual void get_rows(int* mstart, int* mclind, double* dmatval, int size, int* nels, int first, int last) const;
	virtual void get_row_type(char* qrtype, int first, int last) const;
	virtual void get_rhs(double* rhs, int first, int last) const;
	virtual void get_rhs_range(double* range, int first, int last) const;
	virtual void get_col_type(char* coltype, int first, int last) const;
	virtual void get_lb(double* lb, int fisrt, int last) const;
	virtual void get_ub(double* ub, int fisrt, int last) const;

	virtual int get_n_integer_vars() const;

	virtual void free();

// Methods to tranform a problem
public:
	virtual void fix_first_stage(Point const& x0);
	virtual void add_cut(Point const& s, Point const& x0, double rhs);
	virtual void del_rows(int first, int last);
	virtual void add_rows(int newrows, int newnz, const char* qrtype, const double* rhs,
		const double* range, const int* mstart, const int* mclind, const double* dmatval);
	virtual void add_cols(int newcol, int newnz, const double* objx, const int* mstart, const int* mrwind,
		const double* dmatval, const double* bdl, const double* bdu);
	virtual void add_name(int type, const char* cnames, int indice);
	virtual void chg_obj(int nels, const int* mindex, const double* obj);
	virtual void chg_bounds(int nbds, const int* mindex, const char* qbtype, const double* bnd);
	virtual void chg_col_type(int nels, const int* mindex, const char* qctype) const;

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
	virtual void set_threads(int n_threads);
};
#endif