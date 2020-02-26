#pragma once

#include <string>
#include <iostream>
#include "common.h"
#include "BendersOptions.h"
#include "SlaveCut.h"
#include "SimplexBasis.h"

/*!
* \class class SolverAbstract
* \brief Virtual class to implement solvers methods
*/
class SolverAbstract {
public:
	std::string _name;

public:
	SolverAbstract();
	virtual ~SolverAbstract();

// General methods
public:
	virtual void init(std::string const& path_to_mps);
	virtual void writeprob(const char* name, const char* flags);
	virtual void solve(int& lp_status, std::string path_to_mps);
	virtual void solve_integer(int& lp_status, std::string path_to_mps);
	virtual void get_obj(double* obj, int first, int last);
	virtual void get_ncols(int& cols);
	virtual void get_nrows(int& rows);
	virtual void free();

// Methods to tranform a problem
public:
	virtual void fix_first_stage(Point const& x0);
	virtual void add_cut(Point const& s, Point const& x0, double const& rhs);
	virtual int del_rows(int nrows, const int* mindex);
	virtual void add_rows(int newrows, int newnz, const char* qrtype, const double* rhs, 
		const double* range, const int* mstart, const int* mclind, const double* dmatval);
	virtual void add_cols(int newcol, int newnz, const double* objx, const int* mstart, const int* mrwind,
		const double* dmatval, const double* bdl, const double* bdu);
	virtual void add_names(int type, const char* cnames, int first, int last);
	virtual void chgobj(int nels, const int* mindex, const double* obj);
	virtual void chgbounds(int nbds, const int* mindex, const char* qbtype, const double* bnd);
	
// Methods to get a solution
public:	
	virtual void get_basis(int* rstatus, int* cstatus);
	virtual void get_value(double& lb);
	virtual void getmipvalue(double& lb);
	virtual void getlpvalue(double& lb);
	virtual void get_simplex_ite(int& result);

	// A conserver ?
	virtual void get(Point& x0, double& alpha, DblVector& alpha_i);
	
	virtual void get_LPsol(double* primals, double* slacks, double* duals, double* reduced_costs);
	virtual void get_MIPsol(double* primals, double* duals);

// Methods to set algorithm or logs levels
	virtual void set_output_loglevel(int loglevel);
	virtual void set_algorithm(std::string algo);
};