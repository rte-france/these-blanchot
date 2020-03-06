#pragma once

#include <string>
#include <iostream>
#include "common.h"
#include "BendersOptions.h"


// Definition of optimality codes
enum SOLVER_STATUS {
	OPTIMAL,
	INFEASIBLE,
	UNBOUNDED,
	UNKNOWN,
};


/*!
* \class class SolverAbstract
* \brief Virtual class to implement solvers methods
*/
class SolverAbstract {

public :
	StrVector SOLVER_STRING_STATUS = {
	"OPTIMAL",
	"INFEASIBLE",
	"UNBOUNDED",
	"UNKNOWN"
	};

public:
	std::string _name;
	typedef std::shared_ptr<SolverAbstract> Ptr;
	std::list<std::ostream*> _stream;
public:
	SolverAbstract() {};
	virtual ~SolverAbstract() {};

public:
	std::list<std::ostream*>& get_stream() { return _stream; };
	void add_stream(std::ostream& stream) { get_stream().push_back(&stream); };

// General methods
public:
	virtual void init(std::string const& path_to_mps) = 0;
	virtual void load_lp(const char* probname, int ncol, int nrow, 
		const char* qrtype, const double* rhs, const double* range,
		const double* obj, const int* mstart, const int* mnel, const int* mrwind,
		const double* dmatval, const double* dlb, const double* dub) = 0;
	virtual void write_prob(const char* name, const char* flags) const = 0;
	virtual void write_errored_prob(int status, BendersOptions const& options, std::string const& path_to_mps) const = 0;
	virtual void read_prob(const char* prob_name, const char* flags) = 0;
	virtual void solve(int& lp_status, std::string const& path_to_mps) = 0;
	virtual void solve_integer(int& lp_status, std::string const& path_to_mps) = 0;
	virtual void get_obj(double* obj, int first, int last) const = 0;
	virtual int  get_ncols() const = 0;
	virtual int  get_nrows() const = 0;
	virtual int  get_nelems() const = 0;

	virtual void get_rows(int* mstart, int* mclind, double* dmatval, int size, int* nels, int first, int last) const = 0;
	virtual void get_row_type(char* qrtype, int first, int last) const = 0;
	virtual void get_rhs(double* rhs, int first, int last) const = 0;
	virtual void get_rhs_range(double* range, int first, int last) const = 0;
	virtual void get_col_type(char* coltype, int first, int last) const = 0;
	virtual void get_lb(double* lb, int fisrt, int last) const = 0;
	virtual void get_ub(double* ub, int fisrt, int last) const = 0;

	virtual void free() = 0;

// Methods to tranform a problem
public:
	virtual void fix_first_stage(Point const& x0) = 0;
	virtual void add_cut(Point const& s, Point const& x0, double rhs) = 0;
	virtual void del_rows(int nrows, const int* mindex) = 0;
	virtual void add_rows(int newrows, int newnz, const char* qrtype, const double* rhs, 
		const double* range, const int* mstart, const int* mclind, const double* dmatval) = 0;
	virtual void add_cols(int newcol, int newnz, const double* objx, const int* mstart, const int* mrwind,
		const double* dmatval, const double* bdl, const double* bdu) = 0;
	virtual void add_names(int type, const char* cnames, int first, int last) = 0;
	virtual void chg_obj(int nels, const int* mindex, const double* obj) = 0;
	virtual void chg_bounds(int nbds, const int* mindex, const char* qbtype, const double* bnd) = 0;
	virtual void chg_col_type(int nels, const int* mindex, const char* qctype) const = 0;
	
// Methods to get a solution
public:	
	virtual void get_basis(int* rstatus, int* cstatus) const = 0;
	virtual void get_value(double& lb) const = 0;
	virtual void get_mip_value(double& lb) const = 0;
	virtual void get_lp_value(double& lb) const = 0;
	virtual void get_simplex_ite(int& result) const = 0;

	// A conserver ?
	virtual void get(Point& x0, double& alpha, DblVector& alpha_i) = 0;
	
	virtual void get_LP_sol(double* primals, double* slacks, double* duals, double* reduced_costs) = 0;
	virtual void get_MIP_sol(double* primals, double* duals) = 0;

// Methods to set algorithm or logs levels
	virtual void set_output_log_level(int loglevel) = 0;
	virtual void set_algorithm(std::string const& algo) = 0;
	virtual void set_threads(int n_threads) = 0;
};