#include "solver.cpp"

int main(int argc, char *argv[]){

    std::cout << "Arguments : " << std::endl;
    std::cout << "   1. Solver Name" << std::endl;
    std::cout << "   2. Path to MPS to read" << std::endl;
    std::cout << std::endl;

    std::string solver_name = argv[1];

    SolverAbstract::Ptr solver;
    test_read_prob(argv[1], argv[2]);

    test_modify_prob(argv[1], argv[2]);
    
    return 0;
}