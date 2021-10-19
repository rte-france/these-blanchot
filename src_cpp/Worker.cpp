#include "Worker.h"


std::list<std::ostream *> & Worker::get_stream() {
	return _solver->get_stream();
}



Worker::Worker() {
	_is_master = false;
}

/*!
*  \brief Create a pointer to the right solver
*
*  \param solver_name : name of the solver to use
*/
void Worker::declare_solver(std::string const & solver_name, WorkerPtr fictif)
{
	// Declaration du solver
	if (solver_name == "") {
		std::cout << "SOLVER NON RECONU" << std::endl;
		std::exit(0);
	}
#ifdef CPLEX
	else if (solver_name == "CPLEX") {
		if (fictif != NULL) {
			_solver = std::make_shared< SolverCPLEX>(_path_to_mps, fictif->_solver);
		}
		else {
			_solver = std::make_shared< SolverCPLEX>(_path_to_mps);
		}
	}
#endif
#ifdef XPRESS
	else if (solver_name == "XPRESS") {
		_solver = std::make_shared< SolverXPRESS>();
	}
#endif
	else {
		std::cout << "SOLVER NON RECONU" << std::endl;
		std::exit(0);
	}
}

Worker::~Worker() {
}

/*!
*  \brief Free the problem
*/
void Worker::free() {
	_solver->free();
}

/*!
*  \brief Return the optimal value of a problem
*
*  \param lb : double which receives the optimal value
*/
void Worker::get_value(double & lb) {
	if (_is_master) {
		_solver->get_mip_value(lb);
	}
	else {
		_solver->get_lp_value(lb);

	}
}	

/*!
*  \brief Initialization of a problem 
*
*  \param variable_map : map linking each problem name to its variables and their ids
*
*  \param problem_name : name of the problem
*/
void Worker::init(Str2Int const & variable_map, std::string const & path_to_mps, std::string const& solver_name) {
	
	// Creation du solver adapte
	declare_solver(solver_name, NULL);

	_path_to_mps = path_to_mps;
	add_stream(std::cout);
	_solver->init(path_to_mps);
	_solver->read_prob(path_to_mps.c_str(), "MPS");

	//std::ifstream file(_path_to_mapping.c_str());
	_name_to_id = variable_map;
	for(auto const & kvp : variable_map) {
		_id_to_name[kvp.second] = kvp.first;
	}
	_is_master = false;
}

/*!
*  \brief Initialization of a problem
*
*  \param variable_map : map linking each problem name to its variables and their ids
*
*  \param problem_name : name of the problem
*
*  \param fictif : slave problem to copy
*/
void Worker::init(Str2Int const& variable_map, std::string const& path_to_mps, 
	std::string const& solver_name, WorkerPtr fictif) {

	// Creation du solver adapte
	declare_solver(solver_name, fictif);

	add_stream(std::cout);
	_solver->init("");
	//_solver->read_prob(path_to_mps.c_str(), "MPS");

	//std::ifstream file(_path_to_mapping.c_str());
	_name_to_id = variable_map;
	for (auto const& kvp : variable_map) {
		_id_to_name[kvp.second] = kvp.first;
	}
	_is_master = false;
}


/*!
*  \brief Add a stream to the list of output streams of the Worker
*
*  \param stream : stream to add
*/
void Worker::add_stream(std::ostream& stream)
{
	get_stream().push_back(&stream);
}

/*!
*  \brief Method to solve a problem as a LP
*
*  \param lp_status : problem status after optimization
*/
void Worker::solve(int & lp_status, BendersOptions const& options, int n_prob, std::string const& path_to_mps) {

	if (_is_master) {
		_solver->presolve(options.MASTER_PRESOLVE);
		_solver->scaling(options.MASTER_SCALING);
		_solver->optimality_gap(options.GAP / n_prob);
	}
	else {
		_solver->presolve(options.SLAVE_PRESOLVE);
		_solver->scaling(options.SLAVE_SCALING);
		_solver->optimality_gap(options.GAP / n_prob);
	}

	_solver->solve(lp_status, _path_to_mps);
	write_errored_prob(lp_status, options, path_to_mps);
}

/*!
*  \brief Method to solve a problem as a MILP
*
*  \param lp_status : problem status after optimization
*/
void Worker::solve_integer(int& lp_status, BendersOptions const& options, int n_prob, std::string const& path_to_mps) {

	if (_is_master) {
		_solver->presolve(options.MASTER_PRESOLVE);
		_solver->scaling(options.MASTER_SCALING);
		_solver->optimality_gap(options.GAP / n_prob);
	}
	else {
		_solver->presolve(options.SLAVE_PRESOLVE);
		_solver->scaling(options.SLAVE_SCALING);
		_solver->optimality_gap(options.GAP / n_prob);
	}

	_solver->solve_integer(lp_status, path_to_mps);
	write_errored_prob(lp_status, options, path_to_mps);
}

void Worker::solve_quadratic(int& lp_status)
{
	_solver->solve_qp(lp_status);
}

/*!
*  \brief Write a problem which was not solved to optimality
*
*  \param status : status of the solver
*
*  \param options : bool to say if need to write
*
*  \param path_to_mps : path to the file to write
*/
void Worker::write_errored_prob(int status, BendersOptions const& options, std::string const& path_to_mps) const {
	_solver->write_errored_prob(status, options, path_to_mps);
}

/*!
*  \brief get MIP solution of a problem (available after solve_integer)
*
*  \param x : primal values
*
*  \param duals : dual values
*/
void Worker::get_MIP_sol(double* x, double* duals)
{
	_solver->get_MIP_sol(x, duals);
}

/*!
*  \brief get MIP value of a problem (available after solve_integer)
*/
double Worker::get_mip_value()
{
	double val(0);
	_solver->get_mip_value(val);
	return val;
}

/*!
*  \brief Get the number of iteration needed to solve a problem
*
*  \param result : result
*/
void Worker::get_simplex_ite(int & result) {
	_solver->get_simplex_ite(result);
}

void Worker::set_simplex_iter(int iter)
{
	return _solver->set_simplex_iter(iter);
}

/*!
*  \brief Get the number of columns of the problem
*/
int Worker::get_ncols()
{
	return _solver->get_ncols();
}

/*!
*  \brief Get the number of rows of the problem
*/
int Worker::get_nrows()
{
	return _solver->get_nrows();
}

/*!
*  \brief Get the objective vector of the problem between column first and last
*
*  \param obj : vector to fill with the objective
*
*  \param first : first column
*
*  \param last : last column
*/
void Worker::get_obj(DblVector & obj, int first, int last)
{
	_solver->get_obj(obj.data(), first, last);
}

int Worker::get_n_integer_vars()
{
	return _solver->get_n_integer_vars();
}
