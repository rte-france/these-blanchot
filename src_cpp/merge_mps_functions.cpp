#include "merge_mps_functions.h"

void declare_solver(SolverAbstract::Ptr& solv, BendersOptions& options) {
	if (options.SOLVER == "") {
		std::cout << "SOLVER NON RECONU" << std::endl;
		std::exit(0);
	}
	#ifdef CPLEX
	else if (options.SOLVER == "CPLEX") {
		solv = std::make_shared< SolverCPLEX>();
	}
	#endif
	#ifdef XPRESS
	else if (options.SOLVER == "XPRESS") {
		solv = std::make_shared< SolverXPRESS>();
	}
	#endif
	else {
		std::cout << "SOLVER NON RECONU" << std::endl;
		std::exit(0);
	}
}