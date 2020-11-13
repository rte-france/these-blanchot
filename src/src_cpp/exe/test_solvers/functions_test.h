#pragma once

#include "SolverAbstract.h"
#include <iostream>

#ifdef CPLEX
	#include "SolverCplex.h"
#endif
#ifdef XPRESS
	#include "SolverXpress.h"
#endif

#include <typeinfo>

void declaration_solver(SolverAbstract::Ptr& solver, const std::string solver_name);

void print_big_message(const std::string& message);

void print_rows(int n_elems, const std::vector<double>& matval, std::vector<int>& mstart,
	const std::vector<int>& mind, const std::vector<double>& rhs, const std::vector<char>& rtypes);

void print_obj(const std::vector<double> &obj);

void get_and_print_rows(SolverAbstract::Ptr solver);

void get_and_print_obj(SolverAbstract::Ptr solver);

void get_and_print_variables_bounds(SolverAbstract::Ptr solver);

void test_read_prob(const std::string solver_name, const std::string prob_name);

void test_modify_prob(const std::string solver_name, const std::string prob_name);

void solve_problem(const std::string solver_name, const std::string prob_name);

void solve_with_and_without_solver_output(const std::string solver_name, 
	const std::string prob_name);