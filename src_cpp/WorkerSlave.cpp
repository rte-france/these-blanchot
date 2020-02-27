#include "WorkerSlave.h"
#include "launcher.h"

WorkerSlave::WorkerSlave() {

}


/*!
*  \brief Constructor of a Worker Slave
*
*  \param variable_map : Map of linking each variable of the problem to its id
*
*  \param problem_name : Name of the problem
*
*/
WorkerSlave::WorkerSlave(Str2Int const & variable_map, std::string const & path_to_mps, double const & slave_weight, BendersOptions const & options) {
	
	// Declaration du solver
	if (options.SOLVER == "") {
		std::cout << "SOLVER NON RECONU" << std::endl;
		std::exit(0);
	}
	#ifdef CPLEX
	else if (options.SOLVER == "CPLEX") {
		_solver = new SolverCPLEX();
	}
	#endif
	#ifdef XPRESS
	else if (options.SOLVER == "XPRESS") {
		_solver = std::make_unique< SolverXPRESS>();
	}
	#endif
	else {
		std::cout << "SOLVER NON RECONU" << std::endl;
		std::exit(0);
	}
	
	init(variable_map, path_to_mps);
	_solver->set_output_loglevel(options.XPRESS_TRACE);

	int mps_ncols;
	_solver->get_ncols(mps_ncols);

	DblVector o(mps_ncols, 0);
	IntVector sequence(mps_ncols);
	for (int i(0); i < mps_ncols; ++i) {
		sequence[i] = i;
	}
	_solver->get_obj(o.data(), 0, mps_ncols - 1);

	for (auto & c : o) {
		c *= slave_weight;
	}
	_solver->chgobj(mps_ncols, sequence.data(), o.data());
	_solver->set_algorithm("DUAL");

}
WorkerSlave::~WorkerSlave() {

}

/*!
*  \brief Write in a problem in an lp file 
*
* Method to write a problem in an lp file
*
*  \param it : id of the problem
*/
void WorkerSlave::write(int it) {
	std::stringstream name;
	name << "slave_" << it << ".lp";
	_solver->writeprob(name.str().c_str(), "l");
}

/*!
*  \brief Fix a set of variables to constant in a problem
*
*  Method to set variables in a problem by fixing their bounds
*
*  \param x0 : Set of variables to fix
*/
void WorkerSlave::fix_to(Point const & x0) {
	int nbnds((int)_name_to_id.size());
	std::vector<int> indexes(nbnds);
	std::vector<char> bndtypes(nbnds, 'B');
	std::vector<double> values(nbnds);

	int i(0);
	for (auto const & kvp : _id_to_name) {
		indexes[i] = kvp.first;
		values[i] = x0.find(kvp.second)->second;
		++i;
	}

	_solver->chgbounds(nbnds, indexes.data(), bndtypes.data(), values.data());
}

/*!
*  \brief Get LP solution value of a problem
*
*  \param s : Empty point which receives the solution
*/
void WorkerSlave::get_subgradient(Point & s) {
	s.clear();
	int ncols;
	_solver->get_ncols(ncols);
	std::vector<double> ptr(ncols, 0);
	_solver->get_LPsol(NULL, NULL, NULL, ptr.data());
	for (auto const & kvp : _id_to_name) {
		s[kvp.second] = +ptr[kvp.first];
	}

}

/*!
*  \brief Get simplex basis of a problem
*
*  Method to store simplex basis of a problem, and build the distance matrix
*/
SimplexBasis WorkerSlave::get_basis() {
	int ncols;
	int nrows;
	IntVector cstatus;
	IntVector rstatus;

	_solver->get_nrows(nrows);
	_solver->get_ncols(ncols);

	cstatus.resize(ncols);
	rstatus.resize(nrows);
	_solver->get_basis(rstatus.data(), cstatus.data());
	return std::make_pair(rstatus, cstatus);
}

