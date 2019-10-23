#ifdef XPRESS
#include "Solver_XPRESS.h"

/*
Solver XPRESS
*/
XPRESS_Solver::XPRESS_Solver()
	:AbstractSolver()
{

	XPRSinit("");
	_name = "XPRESS";
}

XPRESS_Solver::~XPRESS_Solver(){
	XPRSfree();
}


void XPRESS_Solver::get_env(CPXENVptr &env){
}

#endif