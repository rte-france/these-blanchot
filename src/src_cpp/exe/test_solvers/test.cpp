#include "functions_test.h"

int main(int argc, char *argv[]){

    std::cout << "Arguments : " << std::endl;
    std::cout << "   1. Solver Name" << std::endl;
    std::cout << "   2. Path to MPS to read" << std::endl;
    std::cout << std::endl;

    std::string solver_name     = argv[1];
    std::string instance_path   = argv[2];
    
    
    SolverAbstract::Ptr solver;
    test_read_prob(solver_name, instance_path);

    test_modify_prob(solver_name, instance_path);

    solve_problem(solver_name, instance_path);
    
    solve_with_and_without_solver_output(solver_name, instance_path);
    return 0;
}