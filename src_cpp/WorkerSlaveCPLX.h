#pragma once
#include "WorkerSlave.h"
#include "common.h"


#ifdef CPLEX
/*! 
* \class WorkerSlaveCPLX
* \brief Class daughter of WorkerSlave Class, build and manage a slave problem solved with CPLEX
*/
class WorkerSlaveCPLX : public WorkerSlave {
public :
	CPXENVptr _cplx;
	CPXLPptr _prb;
	
	StrVector CPLEX_LP_STATUS;

public:
	WorkerSlaveCPLX();
	WorkerSlaveCPLX(Str2Int const & variable_map, std::string const & path_to_mps, double const & slave_weight, BendersOptions const & options, AbstractSolver* solver);
	virtual ~WorkerSlaveCPLX();

public :
	void init(Str2Int const & variable_map, std::string const & path_to_mps);
	void get_value(double & lb);
	void get_simplex_ite(int & result) ;
	void free();
	void solve(int & lp_status);
	void get_ncols(int & ncols);
	void get_nrows(int & nrows);

public:
	void write(int it);
	void fix_to(Point const & x0);
	void get_subgradient(Point & s);
	SimplexBasis get_basis();


};

#endif
