#pragma once
#include "catch2.hpp"

#include "SolverAbstract.h"
#include <iostream>
#include "functions_test.h"

#ifdef CPLEX
#include "SolverCplex.h"
#endif
#ifdef XPRESS
#include "SolverXpress.h"
#endif

TEST_CASE("1. MPS file can be read and we can get every information about the problem") {

    std::string instance = "../../data_test/mip_toy_prob.mps";
    std::string solver_name = "XPRESS";
    //========================================================================================
    // 1. declaration d'un objet solveur
    SolverAbstract::Ptr solver = declaration_solver(solver_name);
    REQUIRE( solver->get_number_of_instances() == 1 );

    //========================================================================================
    // 2. initialisation d'un probleme et lecture
    solver->init();
    
    const std::string flags = "MPS";
    solver->read_prob(instance.c_str(), flags.c_str());

    //========================================================================================
    // 3. Recuperation des donnees de base du probleme
    REQUIRE(solver->get_ncols() == 2);
    REQUIRE(solver->get_nrows() == 2);
    REQUIRE(solver->get_n_integer_vars() == 2);
    REQUIRE(solver->get_nelems() == 4);

    //========================================================================================
    // 4. Recuperation de la fonction objectif
    int n_vars = solver->get_ncols();
    std::vector<double> obj(n_vars);
    solver->get_obj(obj.data(), 0, n_vars - 1);

    std::vector<double> neededObj = { -5.0, -4.0 };
    REQUIRE(obj == neededObj);


    //========================================================================================
    // 5. Recuperation des contraintes
    int n_elems = solver->get_nelems();
    int n_cstr  = solver->get_nrows();

    std::vector<double> matval(n_elems);
    std::vector<int>	mstart(n_cstr + 1);
    std::vector<int>	mind(n_elems);

    int n_returned(0);
    solver->get_rows(mstart.data(), mind.data(), matval.data(), n_elems, &n_returned, 0, n_cstr - 1);
    REQUIRE(n_returned == 4);

    std::vector<double> neededMatval = { 1.0, 1.0, 10.0, 6.0 };
    REQUIRE(matval == neededMatval);

    std::vector<int> neededMatind = { 0, 1, 0, 1 };
    REQUIRE(mind == neededMatind);

    std::vector<int> neededMstart = { 0, 2, 4 };
    REQUIRE(mstart == neededMstart);


    //========================================================================================
    // 6. Recuperation des seconds membres
    n_cstr = solver->get_nrows();
    std::vector<double> rhs(n_cstr);
    if (n_cstr > 0) {
        solver->get_rhs(rhs.data(), 0, n_cstr - 1);
    }
    
    std::vector<double> neededRhs = { 5.0, 45.0 };
    REQUIRE(rhs == neededRhs);

    //========================================================================================
    // 7. Recuperation des types de contraintes
    n_cstr = solver->get_nrows();
    std::vector<char> rtypes(n_cstr);
    if (n_cstr > 0) {
        solver->get_row_type(rtypes.data(), 0, n_cstr - 1);
    }

    std::vector<char> neededRtypes = { 'L', 'L' };
    REQUIRE(rtypes == neededRtypes);


    //========================================================================================
    // 8. Recuperation des types de variables
    n_vars = solver->get_ncols();
    std::vector<char> coltype(n_vars);
    solver->get_col_type(coltype.data(), 0, n_vars - 1);
    
    std::vector<char> needCtypes = { 'I', 'I' };
    REQUIRE(coltype == needCtypes);


    //========================================================================================
    // 9. Recuperation des bornes inf sur les variables
    n_vars = solver->get_ncols();
    std::vector<double> lb(n_vars);
    solver->get_lb(lb.data(), 0, n_vars - 1);
    std::vector<double> neededLb = { 0.0 , 0.0 };
    REQUIRE(lb == neededLb);


    //========================================================================================
    // 10. Recuperation des bornes sup sur les variables
    n_vars = solver->get_ncols();
    std::vector<double> ub(n_vars);
    solver->get_ub(ub.data(), 0, n_vars - 1);
    std::vector<double> neededUb = { 1e20, 1e20 };
    REQUIRE(ub == neededUb);
}