#pragma once
#include "catch2.hpp"

#include "SolverAbstract.h"
#include <iostream>
#include <fstream>
#include "functions_test.h"
#include "define_datas.hpp"

#ifdef CPLEX
#include "SolverCplex.h"
#endif
#ifdef XPRESS
#include "SolverXpress.h"
#endif

TEST_CASE("3. A problem is solved and we can get the optimal solution") {

    AllDatas datas;
    fill_datas(datas);

    auto inst = GENERATE(MIP_TOY, MULTIKP, UNBD_PRB, INFEAS_PRB);
    SECTION("Reanding instance") {

        std::string instance = datas[inst]._path;
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
        SECTION("Solving the problem with solver's output solves it and the log is sent to a file out.txt") {
            std::ofstream out("out.txt");
            std::streambuf* coutbuf = std::cout.rdbuf(); //save old buf
            std::cout.rdbuf(out.rdbuf()); //redirect std::cout to out.txt!

            solver->set_output_log_level(3);
            int slv_status(0);
            solver->solve_mip(slv_status);

            bool success = false;
            for (auto stat : datas[inst]._status_int) {
                if (stat == slv_status) {
                    SUCCEED();
                    success = true;
                    break;
                }
            }
            if (!success) {
                FAIL();
            }

            success = false;
            for (auto stat : datas[inst]._status) {
                if (stat == solver->SOLVER_STRING_STATUS[slv_status]) {
                    SUCCEED();
                    success = true;
                    break;
                }
            }
            if (!success) {
                FAIL();
            }
            if (solver->SOLVER_STRING_STATUS[slv_status] == "OPTIMAL")
            {
                double mip_val(0);
                solver->get_mip_value(mip_val);
                REQUIRE(mip_val == datas[inst]._optval);
            }

            solver->free();
            REQUIRE(solver->get_number_of_instances() == 1);

            // Reset standard cout
            std::cout.rdbuf(coutbuf);

            // Check if a file has been written
            std::ifstream std_solv_output("out.txt");
            std::string line;
            int cnt = 0;
            while (getline(std_solv_output, line)) //Tant qu'on n'est pas � la fin, on lit
            {
                cnt += 1;
                break;
            }
            REQUIRE(cnt > 0);
        }

        //========================================================================================
        // 4. Solving without output on screen
        SECTION("Solving the problem without solver's output solves it and nothing is written in the output file out.txt") {
            solver->init();
            solver->add_stream(std::cout);
            REQUIRE(solver->get_number_of_instances() == 1);

            solver->read_prob(instance.c_str(), flags.c_str());

            solver->set_output_log_level(0);
            int slv_status = 0;
            solver->solve_mip(slv_status);

            bool success = false;
            for (auto stat : datas[inst]._status_int) {
                if (stat == slv_status) {
                    SUCCEED();
                    success = true;
                    break;
                }
            }
            if (!success) {
                FAIL();
            }

            success = false;
            for (auto stat : datas[inst]._status) {
                if (stat == solver->SOLVER_STRING_STATUS[slv_status]) {
                    SUCCEED();
                    success = true;
                    break;
                }
            }
            if (!success) {
                FAIL();
            }

            if (solver->SOLVER_STRING_STATUS[slv_status] == "OPTIMAL")
            {
                double mip_val(0);
                solver->get_mip_value(mip_val);
                REQUIRE(mip_val == datas[inst]._optval);
            }

            solver->free();
            REQUIRE(solver->get_number_of_instances() == 1);
        }
    }
}