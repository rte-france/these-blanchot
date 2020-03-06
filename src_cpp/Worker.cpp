#include "Worker.h"


std::list<std::ostream *> & Worker::get_stream() {
	return _solver->get_stream();
}



Worker::Worker() {
	_is_master = false;
}

void Worker::declare_solver(std::string const & solver_name)
{
	// Declaration du solver
	if (solver_name == "") {
		std::cout << "SOLVER NON RECONU" << std::endl;
		std::exit(0);
	}
#ifdef CPLEX
	else if (solver_name == "CPLEX") {
		_solver = std::make_shared< SolverCPLEX>();
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
	declare_solver(solver_name);

	_path_to_mps = path_to_mps;
	add_stream(std::cout);
	_solver->init(path_to_mps);
	_solver->read_prob(path_to_mps.c_str(), "");

	//std::ifstream file(_path_to_mapping.c_str());
	_name_to_id = variable_map;
	for(auto const & kvp : variable_map) {
		_id_to_name[kvp.second] = kvp.first;
	}
	_is_master = false;
}

void Worker::add_stream(std::ostream& stream)
{
	get_stream().push_back(&stream);
}



/*!
*  \brief Method to solve a problem as a LP
*
*  \param lp_status : problem status after optimization
*/
void Worker::solve(int & lp_status, BendersOptions const& options, std::string const& path_to_mps) {
	_solver->solve(lp_status, _path_to_mps);
	write_errored_prob(lp_status, options, path_to_mps);
}

/*!
*  \brief Method to solve a problem as a MILP
*
*  \param lp_status : problem status after optimization
*/
void Worker::solve_integer(int& lp_status, BendersOptions const& options, std::string const& path_to_mps) {
	_solver->solve_integer(lp_status, _path_to_mps);
	write_errored_prob(lp_status, options, path_to_mps);
}

void Worker::write_errored_prob(int status, BendersOptions const& options, std::string const& path_to_mps) const {
	_solver->write_errored_prob(status, options, path_to_mps);
}

void Worker::get_MIP_sol(double* x, double* duals)
{
	_solver->get_MIP_sol(x, duals);
}

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