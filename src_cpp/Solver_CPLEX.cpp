#ifdef CPLEX
#include "Solver_CPLEX.h"
/*
Solver CPLEX
*/
CPLEX_Solver::CPLEX_Solver()
	:AbstractSolver()
{
	_status = -1;
	_env = CPXopenCPLEX (&_status);
	_name = "CPLEX";
}

CPLEX_Solver::~CPLEX_Solver(){
	CPXcloseCPLEX (&_env);
}

void CPLEX_Solver::get_env(CPXENVptr &env){
	env = _env;
}

#endif