# pragma once

#include <string>
#include <iostream>
#include "ilcplex/cplex.h"


// Classe abstraite dont vont heriter les classes solveurs
class AbstractSolver{
public :
	std::string _name;

public :
	AbstractSolver();
	virtual ~AbstractSolver();
	virtual void get_env(CPXENVptr &_env) = 0;
};