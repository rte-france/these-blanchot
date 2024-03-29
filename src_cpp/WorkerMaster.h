#pragma once

#include "Worker.h"
#include "WorkerSlave.h"

/*!
* \class WorkerSlave
* \brief Class daughter of Worker Class, build and manage a master problem
*/
class WorkerMaster;
typedef std::shared_ptr<WorkerMaster> WorkerMasterPtr;

class WorkerMaster : public Worker {
public:
	int _id_alpha;					/*!< Column id of the global epigraph variable alpha (weighted sum of the subproblems epigraph variables ) */
	std::vector<int> _id_alpha_i;	/*!< Column id of each subproblem epigraph variable */
	int _id_level_constraint;		/*!< Id of constraint "theta <= level" of level bundle algorithm */
	DblVector _initial_obj;			/*!< Initial master objective, removed when using LEVEL Bundle, saved to compute solution values */

public:
	WorkerMaster();
	WorkerMaster(Str2Int const & variable_map, std::string const & problem_name, BendersOptions const & options, int nslaves = 1);
	virtual ~WorkerMaster();

	void get(Point & x0, double & alpha, DblVector & alpha_i);
	void get_dual_values(std::vector<double> & dual);
	int get_number_constraint();

	void write(int it);

	void add_cut(Point const & s, Point const & x0, double const & rhs);
	void add_cut_by_iter(int const i, Point const & s, double const & sx0, double const & rhs);
	void add_dynamic_cut(Point const & s, double const & sx0, double const & rhs);
	void add_cut_slave(int i, Point const & s, Point const & x0, double const & rhs);
	void add_agregated_cut_slaves(IntVector const & ids, Point const& s, Point const& x0, double const& rhs);
	void delete_constraint(int const nrows);
	void fix_alpha(double const & bestUB);

	// Level bundle related methods
	void remove_obj();
	void set_problem_to_quadratic();
	void update_level_objective(Point const & center);
	void update_level_constraint(double const & lev);
	void set_matrix_identity(int n_first_vars);
	void save_obj();
};