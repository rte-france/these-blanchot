#include "SolverFactory.h"

SolverFactory::SolverFactory(){
	_available_solvers.clear();
#ifdef CPLEX
	_available_solvers.push_back("CPLEX");
#endif
#ifdef XPRESS
	_available_solvers.push_back("XPRESS");
#endif
}

SolverAbstract::Ptr SolverFactory::create_solver(const std::string solver_name){

    SolverAbstract::Ptr solver;

	if (solver_name == "") {
		std::cout << "SOLVER NON RECONU" << std::endl;
		std::exit(0);
	}
#ifdef CPLEX
	else if (solver_name == "CPLEX") {
		solver = std::make_unique< SolverCplex>("");
		solver = std::make_unique< SolverCplex>();
	}
#endif
#ifdef XPRESS
	else if (solver_name == "XPRESS") {
		solver = std::make_unique< SolverXpress>();
	}
#endif
	else {
		std::cout << "SOLVER NON RECONU" << std::endl;
		std::exit(0);
	}

	return solver;
}