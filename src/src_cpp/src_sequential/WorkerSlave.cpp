#include "WorkerSlave.h"

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
WorkerSlave::WorkerSlave(Str2Int const & variable_map, std::string const & path_to_mps, 
	double const & slave_weight, BendersOptions const & options, 
	std::string const& solver_name) {
	init(variable_map, path_to_mps, solver_name);
	std::cout << "TO DO CONSTRUCTEUR" << std::endl;
	//int mps_ncols(_solver->NumVariables());
	DblVector o_l;
	/*IntVector sequence(mps_ncols);
	for (int i(0); i < mps_ncols; ++i) {
		sequence[i] = i;
	}*/
	//ORTgetobj(*_solver, o_l, 0, mps_ncols - 1);
	//std::cout << "slave_weight : " << slave_weight << std::endl;
	for (auto & c : o_l) {
		c *= slave_weight;
	}
	//ORTchgobj(*_solver, sequence, o_l);	
}

WorkerSlave::~WorkerSlave() {

}

/*!
*  \brief Fix a set of variables to constant in a problem
*
*  Method to set variables in a problem by fixing their bounds
*
*  \param x0 : Set of variables to fix
*/
void WorkerSlave::fix_to(Point const & x0) {
	std::cout << "TO DO FIXTO" << std::endl;
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

	//ORTchgbounds(*_solver, indexes, bndtypes, values);
}

/*!
*  \brief Get LP solution value of a problem
*
*  \param s : Empty point which receives the solution
*/
void WorkerSlave::get_subgradient(Point & s) {
	std::cout << "TO DO GET SUBGRADIENT" << std::endl;
	s.clear();
	std::vector<double> ptr;
	//ORTgetlpreducedcost(*_solver, ptr);
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
	std::cout << "TO DO GETBASIS" << std::endl;
	IntVector cstatus;
	IntVector rstatus;
	//ORTgetbasis(*_solver, rstatus, cstatus);
	return std::make_pair(rstatus, cstatus);
}

