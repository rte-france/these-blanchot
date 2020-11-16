#pragma once
#include "catch2.hpp"

#include "SolverAbstract.h"
#include <iostream>
#include <fstream>
#include "functions_test.h"

#ifdef CPLEX
#include "SolverCplex.h"
#endif
#ifdef XPRESS
#include "SolverXpress.h"
#endif

TEST_CASE("A problem is solved and we can get the optimal solution") {

    std::string instance = "../../data_test/mip_toy_prob.mps";
    std::string solver_name = "XPRESS";
    //========================================================================================
    // 1. declaration d'un objet solveur
    SolverAbstract::Ptr solver = declaration_solver(solver_name);

    //========================================================================================
    // 2. initialisation d'un probleme et lecture
    solver->init();
    solver->add_stream(std::cout);
    
    const std::string flags = "MPS";
    solver->read_prob(instance.c_str(), flags.c_str());

    //========================================================================================
    // 3. Solving with output on screen (redirected in a file)
    std::ofstream out("out.txt");
    std::streambuf* coutbuf = std::cout.rdbuf(); //save old buf
    std::cout.rdbuf(out.rdbuf()); //redirect std::cout to out.txt!

    solver->set_output_log_level(3);
    int slv_status(0);
    solver->solve_mip(slv_status);
    REQUIRE(slv_status == 0);
    REQUIRE(solver->SOLVER_STRING_STATUS[slv_status] == "OPTIMAL");

    double mip_val(0);
    solver->get_mip_value(mip_val);
    REQUIRE(mip_val == -23.0);

    solver->free();
    REQUIRE(solver->get_number_of_instances() == 1);

    // Reset standard cout
    std::cout.rdbuf(coutbuf);

    // Check if a file has been written
    std::ifstream std_solv_output("out.txt");
    std::string line; 
    int cnt = 0;
    while (getline(std_solv_output, line)) //Tant qu'on n'est pas à la fin, on lit
    {
        cnt += 1;
        break;
    }
    REQUIRE(cnt > 0);
    
    //========================================================================================
    // 4. Solving without output on screen

    solver->init();
    solver->add_stream(std::cout);
    REQUIRE(solver->get_number_of_instances() == 1);

    solver->read_prob(instance.c_str(), flags.c_str());

    solver->set_output_log_level(0);
    slv_status = 0;
    solver->solve_mip(slv_status);
    REQUIRE(slv_status == 0);
    REQUIRE(solver->SOLVER_STRING_STATUS[slv_status] == "OPTIMAL");

    solver->get_mip_value(mip_val);
    REQUIRE(mip_val == -23.0);

    solver->free();
    REQUIRE(solver->get_number_of_instances() == 1);
}