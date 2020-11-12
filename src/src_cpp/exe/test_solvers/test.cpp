#include "functions_test.h"

int main(int argc, char *argv[]){

    std::cout << "Arguments : " << std::endl;
    std::cout << "   1. Solver Name" << std::endl;
    std::cout << "   2. Path to MPS to read" << std::endl;
    std::cout << std::endl;

    std::string solver_name = argv[1];

    SolverAbstract::Ptr solver;
    test_read_prob(argv[1], argv[2]);

    test_modify_prob(argv[1], argv[2]);

    solve_problem(argv[1], argv[2]);
    
    solve_with_and_without_solver_output(argv[1], argv[2]);
    return 0;
}