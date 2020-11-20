#pragma once
#include "catch2.hpp"
#include <iostream>
#include "Solver.h"
#include "define_datas.hpp"

TEST_CASE("Un objet solveur peut etre cree et detruit", "[read][init]") {
    AllDatas datas;
    fill_datas(datas);

    SolverFactory factory;

    auto inst = GENERATE(MIP_TOY, MULTIKP, UNBD_PRB, INFEAS_PRB);
    SECTION("Construction and destruction") {

        for (auto const& solver_name : factory.get_solvers_list()) {

            std::string instance = datas[inst]._path;

            //========================================================================================
            // 1. declaration d'un objet solveur
            SolverAbstract::Ptr solver = factory.create_solver(solver_name);
            REQUIRE(solver->get_number_of_instances() == 1);

            //========================================================================================
            // 2. destruction de l'objet pointe
            solver.reset();
            REQUIRE(solver == nullptr);
        }
    }
}


TEST_CASE("1. MPS file can be read and we can get every information about the problem", "[read]") {

    AllDatas datas;
    fill_datas(datas);

    SolverFactory factory;

    auto inst = GENERATE(MIP_TOY, MULTIKP, UNBD_PRB, INFEAS_PRB);
    SECTION("Reading instance") {

        for (auto const& solver_name : factory.get_solvers_list()) {

            std::string instance = datas[inst]._path;
            //========================================================================================
            // 1. declaration d'un objet solveur
            SolverAbstract::Ptr solver = factory.create_solver(solver_name);
            REQUIRE(solver->get_number_of_instances() == 1);

            //========================================================================================
            // 2. initialisation d'un probleme et lecture
            solver->init();

            const std::string flags = "MPS";
            solver->read_prob(instance.c_str(), flags.c_str());

            //========================================================================================
            // 3. Recuperation des donnees de base du probleme
            REQUIRE(solver->get_ncols() == datas[inst]._ncols);
            REQUIRE(solver->get_nrows() == datas[inst]._nrows);
            REQUIRE(solver->get_n_integer_vars() == datas[inst]._nintegervars);
            REQUIRE(solver->get_nelems() == datas[inst]._nelems);

            //========================================================================================
            // 4. Recuperation de la fonction objectif
            int n_vars = solver->get_ncols();
            std::vector<double> obj(n_vars);
            solver->get_obj(obj.data(), 0, n_vars - 1);

            REQUIRE(obj == datas[inst]._obj);


            //========================================================================================
            // 5. Recuperation des contraintes
            int n_elems = solver->get_nelems();
            int n_cstr = solver->get_nrows();

            std::vector<double> matval(n_elems);
            std::vector<int>	mstart(n_cstr + 1);
            std::vector<int>	mind(n_elems);

            int n_returned(0);
            solver->get_rows(mstart.data(), mind.data(), matval.data(), n_elems, &n_returned, 0, n_cstr - 1);

            REQUIRE(n_returned == datas[inst]._nelems);
            REQUIRE(matval == datas[inst]._matval);
            REQUIRE(mind == datas[inst]._mind);
            REQUIRE(mstart == datas[inst]._mstart);


            //========================================================================================
            // 6. Recuperation des seconds membres
            n_cstr = solver->get_nrows();
            std::vector<double> rhs(n_cstr);
            if (n_cstr > 0) {
                solver->get_rhs(rhs.data(), 0, n_cstr - 1);
            }
            REQUIRE(rhs == datas[inst]._rhs);

            //========================================================================================
            // 7. Recuperation des types de contraintes
            n_cstr = solver->get_nrows();
            std::vector<char> rtypes(n_cstr);
            if (n_cstr > 0) {
                solver->get_row_type(rtypes.data(), 0, n_cstr - 1);
            }
            REQUIRE(rtypes == datas[inst]._rowtypes);


            //========================================================================================
            // 8. Recuperation des types de variables
            n_vars = solver->get_ncols();
            std::vector<char> coltype(n_vars);
            solver->get_col_type(coltype.data(), 0, n_vars - 1);

            REQUIRE(coltype == datas[inst]._coltypes);


            //========================================================================================
            // 9. Recuperation des bornes inf sur les variables
            n_vars = solver->get_ncols();
            std::vector<double> lb(n_vars);
            solver->get_lb(lb.data(), 0, n_vars - 1);

            REQUIRE(lb == datas[inst]._lb);

            //========================================================================================
            // 10. Recuperation des bornes sup sur les variables
            n_vars = solver->get_ncols();
            std::vector<double> ub(n_vars);
            solver->get_ub(ub.data(), 0, n_vars - 1);

            REQUIRE(ub == datas[inst]._ub);
        }
    }
}