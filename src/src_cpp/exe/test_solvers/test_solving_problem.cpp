#pragma once
#include "catch2.hpp"

#include "Solver.h"
#include <iostream>
#include <fstream>
#include "define_datas.hpp"


TEST_CASE("3. A problem is solved and we can get the optimal solution with screen output", "[solve][.][output]") {

    AllDatas datas;
    fill_datas(datas);

    SolverFactory factory;

    std::ofstream out("out.txt");

    auto inst = GENERATE(MIP_TOY, MULTIKP, UNBD_PRB, INFEAS_PRB);
    SECTION("Loop on the instances") {

        for (auto const& solver_name : factory.get_solvers_list()) {

            std::cout << "Solver: " << solver_name << std::endl;
            std::string instance = datas[inst]._path;
            //========================================================================================
            // 1. declaration d'un objet solveur
            SolverAbstract::Ptr solver = factory.create_solver(solver_name);

            //========================================================================================
            // 2. initialisation d'un probleme et lecture
            solver->init();
            

            const std::string flags = "MPS";
            solver->read_prob(instance.c_str(), flags.c_str());

            //========================================================================================
            // 3. Solving with output on screen (redirected in a file)

            
            solver->add_stream(std::cout);
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
        }
    }
}

//========================================================================================
// 4. Solving without output on screen
TEST_CASE("3. A problem is solved and we can get the optimal solution without output", "[solve][no-output]") {

    AllDatas datas;
    fill_datas(datas);

    SolverFactory factory;

    auto inst = GENERATE(MIP_TOY, MULTIKP, UNBD_PRB, INFEAS_PRB, NET_MASTER, NET_SP1, NET_SP2);
    SECTION("Loop on the instances") {

        for (auto const& solver_name : factory.get_solvers_list()) {

            std::string instance = datas[inst]._path;

            //================================================================================
            // Initialization
            SolverAbstract::Ptr solver = factory.create_solver(solver_name);
            solver->init();
            solver->add_stream(std::cout);

            const std::string flags = "MPS";
            solver->read_prob(instance.c_str(), flags.c_str());

            //================================================================================
            // Desactivate output and solve
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