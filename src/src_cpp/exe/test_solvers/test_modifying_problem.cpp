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

TEST_CASE("2. Every part of a problem can be modified") {

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
    SECTION( "Deleting the first row actually delete it" ) {
        solver->del_rows(0, 0);

        int n_elems = solver->get_nelems();
        int n_cstr = solver->get_nrows();
        std::vector<double> matval(n_elems);
        std::vector<int>	mstart(n_cstr + 1);
        std::vector<int>	mind(n_elems);
        int n_returned(0);
        solver->get_rows(mstart.data(), mind.data(), matval.data(), n_elems, &n_returned, 0, n_cstr - 1);     

        std::vector<double> neededMatval = { 10.0, 6.0 };
        std::vector<int> neededMatind = { 0, 1 };
        std::vector<int> neededMstart = { 0, 2 };

        REQUIRE(solver->get_nrows() == 1);
        REQUIRE(n_returned == 2);
        REQUIRE(matval == neededMatval);
        REQUIRE(mind == neededMatind);
        REQUIRE(mstart == neededMstart);
    }

    //========================================================================================
    // 4. Add rows
    SECTION( "Solving the problem without solver's output solves it" ) {
        int n_elems = solver->get_nelems();
        int n_cstr = solver->get_nrows();
        std::vector<double> matval(n_elems);
        std::vector<int>	mstart(n_cstr + 1);
        std::vector<int>	mind(n_elems);

        matval = { 5.0, -3.2 };
        mind = { 0, 1 };
        mstart = { 0, 2 };
        std::vector<char> ntype = {'L'};
        std::vector<double> rhs = { 5.0 };
        solver->add_rows(1, 2, ntype.data(), rhs.data(), NULL, mstart.data(),
            mind.data(), matval.data());
        REQUIRE(solver->get_nrows() == 3);

        n_elems = solver->get_nelems();
        n_cstr = solver->get_nrows();
        REQUIRE(n_elems == 6);

        matval.resize(n_elems);
        mstart.resize(n_cstr + 1);
        mind.resize(n_elems);
        int n_returned(0);
        solver->get_rows(mstart.data(), mind.data(), matval.data(), n_elems, &n_returned, 0, n_cstr - 1);
        REQUIRE(n_returned == 6);

        std::vector<double> neededMatval;
        neededMatval.resize(n_elems);
        neededMatval  = { 1.0, 1.0, 10.0, 6.0, 5.0, -3.2 };
        REQUIRE(matval == neededMatval);
        
        std::vector<int> neededMatind;
        neededMatind.resize(n_elems);
        neededMatind = { 0, 1, 0, 1, 0, 1 };
        REQUIRE(mind == neededMatind);

        std::vector<int> neededMstart;
        neededMstart.resize(n_cstr + 1);
        neededMstart = { 0, 2, 4, 6 };
        REQUIRE(mstart == neededMstart);
    }

    //========================================================================================
    // 5. Change obj
    SECTION( "Substracting one to every value in the objective" ) {
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
    }

    //========================================================================================
    // 6. Change RHS
    SECTION( "Adding one to every value in the RHS" ) {
        int n_cstr = solver->get_nrows();
        std::vector<double> rhs;
        rhs.resize(n_cstr);
        solver->get_rhs(rhs.data(), 0, n_cstr - 1);
        for (int i(0); i < n_cstr; i++) {
            solver->chg_rhs(i, rhs[i] + 1);
        }
        solver->get_rhs(rhs.data(), 0, n_cstr - 1);
        std::vector<double> neededRhs = { 6.0, 46.0 };
        REQUIRE(rhs == neededRhs);
    }

    //========================================================================================
    // 7. Change matrix coefficient
    SECTION( "Dividing the last matrix coefficient by one" ) {
        int n_elems = solver->get_nelems();
        int n_cstr = solver->get_nrows();
        
        std::vector<double> matval;
        std::vector<int> mind;
        std::vector<int> mstart;

        matval.resize(n_elems);
        mstart.resize(n_cstr + 1);
        mind.resize(n_elems);

        solver->get_rows(mstart.data(), mind.data(), matval.data(), n_elems, &n_elems, 0, n_cstr - 1);
        int idcol = mind[mind.size() - 1];
        int row = n_cstr - 1;
        double val = matval[matval.size() - 1];
        solver->chg_coef(row, idcol, val / 10.0);

        int n_returned(0);
        solver->get_rows(mstart.data(), mind.data(), matval.data(), n_elems, &n_returned, 0, n_cstr - 1);
        REQUIRE(n_returned == 4);

        std::vector<double> neededMatval;
        std::vector<int> neededMstart;
        std::vector<int> neededMatind;

        neededMatval.resize(n_elems);
        neededMatval = { 1.0, 1.0, 10.0, 0.6 };
        REQUIRE(matval == neededMatval);

        neededMatind.resize(n_elems);
        neededMatind = { 0, 1, 0, 1 };
        REQUIRE(mind == neededMatind);

        neededMstart.resize(n_cstr + 1);
        neededMstart = { 0, 2, 4 };
        REQUIRE(mstart == neededMstart);
    }

    //========================================================================================
    // 8. Add columns
    SECTION( "Adding a column to the problem and setting its coefficients" ) {
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
        int n_vars = solver->get_ncols();
        std::vector<double> obj(n_vars);
        obj.resize(n_vars);
        solver->get_obj(obj.data(), 0, n_vars - 1);

        std::vector<double> neededObj;
        neededObj.resize(n_vars);
        neededObj = { -5.0, -4.0, 3.0 };
        REQUIRE(obj == neededObj);

        //test matrix
        int n_elems = solver->get_nelems();
        int n_cstr = solver->get_nrows();
        REQUIRE(n_elems == 6);
        REQUIRE(n_cstr == 2);

        std::vector<double> matval;
        std::vector<int> mind;
        std::vector<int> mstart;

        matval.resize(n_elems);
        mstart.resize(n_cstr + 1);
        mind.resize(n_elems);

        int n_returned = 0;
        solver->get_rows(mstart.data(), mind.data(), matval.data(), n_elems, &n_returned, 0, n_cstr - 1);
        REQUIRE(n_returned == 6);

        std::vector<double> neededMatval;
        std::vector<int> neededMstart;
        std::vector<int> neededMatind;

        neededMatval.resize(n_elems);
        neededMatval = { 1.0, 1.0, 8.32, 10.0, 6.0, 21.78 };
        REQUIRE(matval == neededMatval);

        neededMatind.resize(n_elems);
        neededMatind = { 0, 1, 2,  0, 1, 2 };
        REQUIRE(mind == neededMatind);

        neededMstart.resize(n_cstr + 1);
        neededMstart = { 0, 3, 6 };
        REQUIRE(mstart == neededMstart);

        // 9. test variables bounds
        n_vars = solver->get_ncols();
        lbs.resize(n_vars);
        solver->get_lb(lbs.data(), 0, n_vars - 1);
        std::vector<double> neededLb = { 0.0 , 0.0, 0.0 };
        REQUIRE(lbs == neededLb);
    
        // 10. Recuperation des bornes sup sur les variables
        n_vars = solver->get_ncols();
        ubs.resize(n_vars);
        solver->get_ub(ubs.data(), 0, n_vars - 1);
        std::vector<double> neededUb = { 1e20, 1e20, 6.0 };
        REQUIRE(ubs == neededUb);
    }

}