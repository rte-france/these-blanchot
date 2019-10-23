# pragma once
#include "common.h"

#ifdef CPLEX
	#include "Solver.h"
	#include "ilcplex/cplex.h"

	class CPLEX_Solver : public AbstractSolver{
	public :
		CPLEX_Solver();
		~CPLEX_Solver();
		void get_env(CPXENVptr &_env);

	public :
		int _status; 
		CPXENVptr _env;
	};
#endif