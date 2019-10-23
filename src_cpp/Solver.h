# pragma once

#include <string>
#include <iostream>


// Classe abstraite dont vont heriter les classes solveurs
class AbstractSolver{
public :
	std::string _name;

public :
	AbstractSolver();
	virtual ~AbstractSolver();
	virtual void get_env(CPXENVptr &_env) = 0;
};



class CPLEX_Solver : public AbstractSolver{
public :
	CPLEX_Solver();
	~CPLEX_Solver();
	void get_env(CPXENVptr &_env);

public :
	int _status; 
	CPXENVptr _env;
};