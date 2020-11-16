#include "functions_test.h"

int main(int argc, char *argv[]){

    std::cout << "Arguments : " << std::endl;
    std::cout << "   1. Solver Name" << std::endl;
    std::cout << "   2. Path to MPS to read" << std::endl;
    std::cout << std::endl;

    std::string solver_name     = argv[1];
    std::string instance_path   = argv[2];
    
    /*SolverAbstract::Ptr solver = declaration_solver(solver_name);
    std::string flag = "MPS";
    solver->read_prob(instance_path.c_str(), flag.c_str());
    std::cout << "ok" << std::endl;
    print_full_problem_from_solver(solver);

    print_problem_caracteristics(solver);

    test_read_prob(solver_name, instance_path);
    test_modify_prob(solver_name, instance_path);
    solve_problem(solver_name, instance_path);*/
    solve_with_and_without_solver_output(solver_name, instance_path);

    return 0;
}