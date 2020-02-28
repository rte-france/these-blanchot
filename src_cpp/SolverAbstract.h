#pragma once

#include <string>
#include <iostream>
#include "common.h"
#include "BendersOptions.h"
//#include "SlaveCut.h"
//#include "SimplexBasis.h"

/*!
* \class class SolverAbstract
* \brief Virtual class to implement solvers methods
*/

class SolverAbstract {
public:
	std::string _name;
	typedef std::shared_ptr<SolverAbstract> Ptr;
	std::list<std::ostream*> _stream;
public:
	SolverAbstract() {};
	virtual ~SolverAbstract() {};

public:
	std::list<std::ostream*>& get_stream() { return _stream; };

// General methods
public:
	virtual void init(std::string const& path_to_mps) = 0;
	virtual void write_prob(const char* name, const char* flags) const = 0;
	virtual void solve(int& lp_status, std::string const& path_to_mps) = 0;
	virtual void solve_integer(int& lp_status, std::string const& path_to_mps) = 0;
	virtual void get_obj(double* obj, int first, int last) const = 0;
	virtual int  get_ncols() const = 0;
	virtual int  get_nrows() const = 0;
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
};