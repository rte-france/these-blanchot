# pragma once

#include "xprs.h"
#include "Solver.h"


class XPRESS_Solver : public AbstractSolver{
public :
	XPRESS_Solver();
	~XPRESS_Solver();
	void get_env(CPXENVptr &_env);
};