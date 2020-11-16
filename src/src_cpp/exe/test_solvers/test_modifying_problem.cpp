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

TEST_CASE("Every part of a problem can be modified") {

    std::string instance = "../../data_test/mip_toy_prob.mps";
    std::string solver_name = "XPRESS";
    //========================================================================================
    // 1. declaration d'un objet solveur
    SolverAbstract::Ptr solver = declaration_solver(solver_name);

    //========================================================================================
    // 2. initialisation d'un probleme et lecture
    solver->init();
    
    const std::string flags = "MPS";
    solver->read_prob(instance.c_str(), flags.c_str());


    //========================================================================================
    // 3. Delete Rows
    solver->del_rows(0, 0);
    REQUIRE(solver->get_nrows() == 1);

    int n_elems = solver->get_nelems();
    int n_cstr = solver->get_nrows();
    std::vector<double> matval(n_elems);
    std::vector<int>	mstart(n_cstr + 1);
    std::vector<int>	mind(n_elems);
    int n_returned(0);
    solver->get_rows(mstart.data(), mind.data(), matval.data(), n_elems, &n_returned, 0, n_cstr - 1);
    REQUIRE(n_returned == 2);

    std::vector<double> neededMatval = { 10.0, 6.0 };
    REQUIRE(matval == neededMatval);

    std::vector<int> neededMatind = { 0, 1 };
    REQUIRE(mind == neededMatind);

    std::vector<int> neededMstart = { 0, 2 };
    REQUIRE(mstart == neededMstart);


    //========================================================================================
    // 4. Add rows
    matval = { 1.0, 1.0 };
    mind = { 0, 1 };
    mstart = { 0, 2 };
    std::vector<char> ntype = {'L'};
    std::vector<double> rhs = { 5.0 };
    solver->add_rows(1, 2, ntype.data(), rhs.data(), NULL, mstart.data(),
        mind.data(), matval.data());
    REQUIRE(solver->get_nrows() == 2);

    n_elems = solver->get_nelems();
    n_cstr = solver->get_nrows();
    REQUIRE(n_elems == 4);

    matval.resize(n_elems);
    mstart.resize(n_cstr + 1);
    mind.resize(n_elems);
    solver->get_rows(mstart.data(), mind.data(), matval.data(), n_elems, &n_returned, 0, n_cstr - 1);
    REQUIRE(n_returned == 4);

    neededMatval.resize(n_elems);
    neededMatval  = { 10.0, 6.0, 1.0, 1.0 };
    REQUIRE(matval == neededMatval);
    
    neededMatind.resize(n_elems);
    neededMatind = { 0, 1, 0, 1 };
    REQUIRE(mind == neededMatind);

    neededMstart.resize(n_cstr + 1);
    neededMstart = { 0, 2, 4 };
    REQUIRE(mstart == neededMstart);


    //========================================================================================
    // 5. Change obj
    int n_vars = solver->get_ncols();
    std::vector<double> obj(n_vars);
    std::vector<int> ids(n_vars);
    solver->get_obj(obj.data(), 0, n_vars - 1);
    for (int i(0); i < n_vars; i++) {
        ids[i] = i;
        obj[i] -= 1;
    }
    solver->chg_obj(n_vars, ids.data(), obj.data());
    solver->get_obj(obj.data(), 0, n_vars - 1);

    std::vector<double> neededObj = { -6.0, -5.0 };
    REQUIRE(obj == neededObj);

    //========================================================================================
    // 6. Change RHS
    n_cstr = solver->get_nrows();
    rhs.resize(n_cstr);
    solver->get_rhs(rhs.data(), 0, n_cstr - 1);
    for (int i(0); i < n_cstr; i++) {
        solver->chg_rhs(i, rhs[i] + 1);
    }
    solver->get_rhs(rhs.data(), 0, n_cstr - 1);
    std::vector<double> neededRhs = { 46.0, 6.0 };
    REQUIRE(rhs == neededRhs);

    //========================================================================================
    // 7. Change matrix coefficient
    n_elems = solver->get_nelems();
    n_cstr = solver->get_nrows();
    matval.resize(n_elems);
    mstart.resize(n_cstr + 1);
    mind.resize(n_elems);
    solver->get_rows(mstart.data(), mind.data(), matval.data(), n_elems, &n_elems, 0, n_cstr - 1);
    int idcol = mind[mind.size() - 1];
    int row = n_cstr - 1;
    double val = matval[matval.size() - 1];
    solver->chg_coef(row, idcol, val / 10.0);

    solver->get_rows(mstart.data(), mind.data(), matval.data(), n_elems, &n_returned, 0, n_cstr - 1);
    REQUIRE(n_returned == 4);

    neededMatval.resize(n_elems);
    neededMatval = { 10.0, 6.0, 1.0, 0.1 };
    REQUIRE(matval == neededMatval);

    neededMatind.resize(n_elems);
    neededMatind = { 0, 1, 0, 1 };
    REQUIRE(mind == neededMatind);

    neededMstart.resize(n_cstr + 1);
    neededMstart = { 0, 2, 4 };
    REQUIRE(mstart == neededMstart);


    //========================================================================================
    // 8. Add columns
    int newcol = 1;
    int nnz = 2;
    std::vector<double> newobj(newcol, 3.0);
    // Offset of col i
    std::vector<int> nmstart(newcol);
    nmstart[0] = 0;
    // Row indices
    std::vector<int> nmind(nnz);
    nmind[0] = 0;
    nmind[1] = 1;

    // Values
    std::vector<double> nmatval(nnz);
    nmatval[0] = 8.32;
    nmatval[1] = 21.78;
    // LBs & UBs
    std::vector<double> lbs(newcol, 0.0);
    std::vector<double> ubs(newcol, 6.0);

    solver->add_cols(newcol, nnz, newobj.data(), nmstart.data(),
        nmind.data(), nmatval.data(), lbs.data(), ubs.data());
    //test ncols
    REQUIRE(solver->get_ncols() == 3);

    //test obj
    n_vars = solver->get_ncols();
    obj.resize(n_vars);
    solver->get_obj(obj.data(), 0, n_vars - 1);

    neededObj.resize(n_vars);
    neededObj = { -6.0, -5.0, 3.0 };
    REQUIRE(obj == neededObj);

    //test matrix
    n_elems = solver->get_nelems();
    n_cstr = solver->get_nrows();
    REQUIRE(n_elems == 6);
    REQUIRE(n_cstr == 2);

    matval.resize(n_elems);
    mstart.resize(n_cstr + 1);
    mind.resize(n_elems);
    solver->get_rows(mstart.data(), mind.data(), matval.data(), n_elems, &n_returned, 0, n_cstr - 1);
    REQUIRE(n_returned == 6);

    neededMatval.resize(n_elems);
    neededMatval = { 10.0, 6.0, 8.32, 1.0, 0.1, 21.78 };
    REQUIRE(matval == neededMatval);

    neededMatind.resize(n_elems);
    neededMatind = { 0, 1, 2,  0, 1, 2 };
    REQUIRE(mind == neededMatind);

    neededMstart.resize(n_cstr + 1);
    neededMstart = { 0, 3, 6 };
    REQUIRE(mstart == neededMstart);

    //test variables bounds
    n_vars = solver->get_ncols();
    lbs.resize(n_vars);
    solver->get_lb(lbs.data(), 0, n_vars - 1);
    std::vector<double> neededLb = { 0.0 , 0.0, 0.0 };
    REQUIRE(lbs == neededLb);

    //========================================================================================
    // 10. Recuperation des bornes sup sur les variables
    n_vars = solver->get_ncols();
    ubs.resize(n_vars);
    solver->get_ub(ubs.data(), 0, n_vars - 1);
    std::vector<double> neededUb = { 1e20, 1e20, 6.0 };
    REQUIRE(ubs == neededUb);

}