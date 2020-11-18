#pragma once
#include "catch2.hpp"
#include <iostream>
#include "SolverAbstract.h"
#include "define_datas.hpp"

#ifdef CPLEX
#include "SolverCplex.h"
#endif
#ifdef XPRESS
#include "SolverXpress.h"
#endif

TEST_CASE("2. Every part of a problem can be modified") {


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

        const std::string flags = "MPS";
        solver->read_prob(instance.c_str(), flags.c_str());

        //========================================================================================
        // 3. Delete Rows
        SECTION("Deleting the first row actually delete it") {
            solver->del_rows(0, 0);

            int n_elems = solver->get_nelems();
            int n_cstr = solver->get_nrows();
            std::vector<double> matval(n_elems);
            std::vector<int>	mstart(n_cstr + 1);
            std::vector<int>	mind(n_elems);
            int n_returned(0);
            solver->get_rows(mstart.data(), mind.data(), matval.data(), 
                n_elems, &n_returned, 0, n_cstr - 1);

            REQUIRE(solver->get_nrows() == datas[inst]._nrows - 1);
            REQUIRE(n_returned          == datas[inst]._matval_delete.size() );
            REQUIRE(matval              == datas[inst]._matval_delete);
            REQUIRE(mind                == datas[inst]._mind_delete);
            REQUIRE(mstart              == datas[inst]._mstart_delete);
        }

        //========================================================================================
        // 4. Add rows
        SECTION("Solving the problem without solver's output solves it") {
            int n_elems = solver->get_nelems();
            int n_cstr = solver->get_nrows();
            std::vector<double> matval(n_elems);
            std::vector<int>	mstart(n_cstr + 1);
            std::vector<int>	mind(n_elems);

            matval = { 5.0, -3.2 };
            mind = { 0, 1 };
            mstart = { 0, 2 };
            std::vector<char> ntype = { 'L' };
            std::vector<double> rhs = { 5.0 };
            solver->add_rows(1, 2, ntype.data(), rhs.data(), NULL, mstart.data(),
                mind.data(), matval.data());

            n_elems = solver->get_nelems();
            n_cstr = solver->get_nrows();

            matval.resize(n_elems);
            mstart.resize(n_cstr + 1);
            mind.resize(n_elems);
            int n_returned(0);
            solver->get_rows(mstart.data(), mind.data(), matval.data(), n_elems, &n_returned, 0, n_cstr - 1);
            

            std::vector<double> neededMatval = datas[inst]._matval;
            neededMatval.push_back(5.0);
            neededMatval.push_back(-3.2);

            std::vector<int> neededMatind = datas[inst]._mind;
            neededMatind.push_back(0);
            neededMatind.push_back(1);

            std::vector<int> neededMstart = datas[inst]._mstart;
            neededMstart.push_back( neededMstart.back() + 2 );

            REQUIRE(solver->get_nrows() == datas[inst]._nrows + 1);
            REQUIRE(n_elems == datas[inst]._nelems + 2);
            REQUIRE(n_returned == datas[inst]._nelems + 2);
            REQUIRE(matval == neededMatval);
            REQUIRE(mind == neededMatind);
            REQUIRE(mstart == neededMstart);
        }

        //========================================================================================
        // 5. Change obj
        SECTION("Substracting one to every value in the objective") {
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

            std::vector<double> neededObj = datas[inst]._obj;
            for (auto& o : neededObj) {
                o -= 1;
            }
            REQUIRE(obj == neededObj);
        }

        //========================================================================================
        // 6. Change RHS
        SECTION("Adding one to every value in the RHS") {
            int n_cstr = solver->get_nrows();
            std::vector<double> rhs;
            rhs.resize(n_cstr);
            solver->get_rhs(rhs.data(), 0, n_cstr - 1);
            for (int i(0); i < n_cstr; i++) {
                solver->chg_rhs(i, rhs[i] + 1);
            }
            solver->get_rhs(rhs.data(), 0, n_cstr - 1);

            std::vector<double> neededRhs = datas[inst]._rhs;
            for (auto& r : neededRhs) {
                r += 1;
            }
            REQUIRE(rhs == neededRhs);
        }

        //========================================================================================
        // 7. Change matrix coefficient
        SECTION("Dividing the last matrix coefficient by one") {
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

            std::vector<double> neededMatval = datas[inst]._matval;
            neededMatval[neededMatval.size() - 1] /= 10;

            REQUIRE(n_returned == datas[inst]._nelems);
            REQUIRE(matval == neededMatval);
            REQUIRE(mind == datas[inst]._mind);
            REQUIRE(mstart == datas[inst]._mstart);
        }

        //========================================================================================
        // 8. Add columns
        SECTION("Adding a column to the problem and setting its coefficients") {
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
            
            REQUIRE(solver->get_ncols() == datas[inst]._ncols + 1);

            //test obj
            int n_vars = solver->get_ncols();
            std::vector<double> obj(n_vars);
            obj.resize(n_vars);
            solver->get_obj(obj.data(), 0, n_vars - 1);

            std::vector<double> neededObj = datas[inst]._obj;
            neededObj.push_back(3.0);
            REQUIRE(obj == neededObj);

            //test matrix
            int n_elems = solver->get_nelems();
            int n_cstr = solver->get_nrows();
            REQUIRE(n_elems == datas[inst]._nelems + 2);
            REQUIRE(n_cstr == datas[inst]._nrows);

            std::vector<double> matval;
            std::vector<int> mind;
            std::vector<int> mstart;

            matval.resize(n_elems);
            mstart.resize(n_cstr + 1);
            mind.resize(n_elems);

            int n_returned = 0;
            solver->get_rows(mstart.data(), mind.data(), matval.data(), n_elems, &n_returned, 0, n_cstr - 1);
            

            std::vector<double> neededMatval = datas[inst]._matval;
            neededMatval.insert(neededMatval.begin() + datas[inst]._mstart[1], nmatval[0]);
            neededMatval.insert(neededMatval.begin() + datas[inst]._mstart[2] + 1, nmatval[1]);

            std::vector<int> neededMatind = datas[inst]._mind;
            neededMatind.insert(neededMatind.begin() + datas[inst]._mstart[1], datas[inst]._ncols);
            neededMatind.insert(neededMatind.begin() + datas[inst]._mstart[2] + 1, datas[inst]._ncols);

            std::vector<int> neededMstart = datas[inst]._mstart;
            neededMstart[1] += 1;
            for (int i(2); i < neededMstart.size(); i++) {
                neededMstart[i] += 2;
            }

            REQUIRE(n_returned == datas[inst]._nelems + 2);
            REQUIRE(matval == neededMatval);
            REQUIRE(mind == neededMatind);
            REQUIRE(mstart == neededMstart);

            // 9. test variables bounds
            n_vars = solver->get_ncols();
            lbs.resize(n_vars);
            ubs.resize(n_vars);

            solver->get_lb(lbs.data(), 0, n_vars - 1);
            solver->get_ub(ubs.data(), 0, n_vars - 1);

            std::vector<double> neededLb = datas[inst]._lb;
            neededLb.push_back(0.0);
            std::vector<double> neededUb = datas[inst]._ub;
            neededUb.push_back(6.0);

            REQUIRE(lbs == neededLb);
            REQUIRE(ubs == neededUb);
        }
    }
}