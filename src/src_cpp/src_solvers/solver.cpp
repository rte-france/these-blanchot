#include "cplex.h"
#include <iostream>

void test_function(){

    int status(0);
	CPXENVptr _env = CPXopenCPLEX(&status);
    std::cout << "OPEN CPLEX status : " << status << std::endl;
}