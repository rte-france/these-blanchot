#include "Worker.h"


std::list<std::ostream *> & Worker::stream() {
	return _stream;
}

Worker::Worker()
	: _is_master(false)
	, _solver(nullptr)
{
}

Worker::~Worker() {

}

/*!
*  \brief Free the problem
*/
void Worker::free() {
	_solver.reset();
	_solver = nullptr;
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
*
*  \param solver_name : name of the solve to use to create the problem
*/
void Worker::init(Str2Int const & variable_map, std::string const & path_to_mps, 
	std::string const& solver_name) {

	_path_to_mps = path_to_mps;
	_stream.push_back(&std::cout);

	SolverFactory factory;
	if (_is_master)
	{
		_solver = factory.create_solver(solver_name);
		_solver->init();
	}
	else
	{
		_solver = factory.create_solver(solver_name);
		_solver->init();
	}

	_solver->read_prob(_path_to_mps.c_str(), "MPS");
	_solver->set_threads(1);

	for(auto const & kvp : variable_map) {
		_id_to_name[kvp.second] = kvp.first;
		_name_to_id[kvp.first] = kvp.second;
	}
}

StrVector ORT_LP_STATUS = {
	"ORT_OPTIMAL",
    "ORT_FEASIBLE",
    "ORT_INFEASIBLE",
    "ORT_UNBOUNDED",
    "ORT_ABNORMAL",
    "ORT_MODEL_INVALID",
    "ORT_NOT_SOLVED"
};

/*!
*  \brief Method to solve a problem
*
*  \param lp_status : problem status after optimization
*/
void Worker::solve(int & lp_status) {
	if (_is_master) {
		_solver->solve_mip(lp_status);
	}
	else {
		_solver->solve_lp(lp_status);
	}

	if (lp_status != OPTIMAL) {
		std::cout << "lp_status is : " << lp_status << std::endl;
		std::exit(1);
	}
}

/*!
*  \brief Get the number of iteration needed to solve a problem
*
*  \param result : result
*/
void Worker::get_simplex_ite(int & result) {
	_solver->get_simplex_ite(result);
}
