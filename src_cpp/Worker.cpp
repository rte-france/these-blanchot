#include "Worker.h"


std::list<std::ostream *> & Worker::get_stream() {
	return _solver->get_stream();
}



Worker::Worker() {
	_is_master = false;
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
void Worker::init(Str2Int const & variable_map, std::string const & path_to_mps) {
	_path_to_mps = path_to_mps;
	add_stream(std::cout);
	_solver->init(path_to_mps);

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
void Worker::solve(int & lp_status) {
	_solver->solve(lp_status, _path_to_mps);
}

/*!
*  \brief Method to solve a problem as a MILP
*
*  \param lp_status : problem status after optimization
*/
void Worker::solve_integer(int& lp_status) {
	_solver->solve_integer(lp_status, _path_to_mps);
}

/*!
*  \brief Get the number of iteration needed to solve a problem
*
*  \param result : result
*/
void Worker::get_simplex_ite(int & result) {
	_solver->get_simplex_ite(result);
}