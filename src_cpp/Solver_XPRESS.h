# pragma once
#include "Solver.h"
#include "common.h"

#ifdef XPRESS
#include "xprs.h"

class XPRESS_Solver : public AbstractSolver{
public :
	XPRESS_Solver();
	~XPRESS_Solver();
	void get_env(CPXENVptr &_env);
};
#endif