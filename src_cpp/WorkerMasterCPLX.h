#pragma once
#ifdef CPLEX
#include "WorkerMaster.h"

/*!
* \class WorkerSlave
* \brief Class daughter of Worker Class, build and manage a master problem
*/
class WorkerMasterCPLX : public WorkerMaster {
public:
	CPXENVptr _cplx;
	CPXLPptr _prb;
	
	StrVector CPLEX_LP_STATUS;

public :
	WorkerMasterCPLX();
	WorkerMasterCPLX(Str2Int const & variable_map, std::string const & problem_name, BendersOptions const & options, AbstractSolver* solver, int nslaves = 1 );
	virtual ~WorkerMasterCPLX();

public :
	void init(Str2Int const & variable_map, std::string const & path_to_mps);
	void get_value(double & lb);
	void get_simplex_ite(int & result);
	void free();
	void solve(int & lp_status);

public :
	void get(Point & x0, double & alpha, DblVector & alpha_i);
	void get_dual_values(std::vector<double> & dual);
	int get_number_constraint();

	void write(int it);

	void add_cut(Point const & s, Point const & x0, double const & rhs);
	void add_cut_by_iter(int const i, Point const & s, double const & sx0, double const & rhs);
	void add_dynamic_cut(Point const & s, double const & sx0, double const & rhs);
	void add_cut_slave(int i, Point const & s, Point const & x0, double const & rhs);
	void delete_constraint(int const nrows);
	void fix_alpha(double const & bestUB);

public :
	void get_ncols(int & ncols);
	void get_nrows(int & nrows);

	void getlb_variables(BendersData & data, BendersOptions const & options, int nvars);
	void getub_variables(BendersData & data, BendersOptions const & options, int nvars);

	void chgbounds(int nvars, std::vector<int> index_vars, std::vector<char> bnd_types, std::vector<double> values);
	void save_alpha_values(BendersData & data);
};


#endif