#include "SolverAbstract.h"
#include <iostream>

#ifdef CPLEX
	#include "SolverCplex.h"
#endif

void declaration_solver(SolverAbstract::Ptr &solver, const std::string solver_name){

	if (solver_name == "") {
		std::cout << "SOLVER NON RECONU" << std::endl;
		std::exit(0);
	}
	#ifdef CPLEX
	if (solver_name == "CPLEX") {
		solver = std::make_shared< SolverCplex>("");
        solver = std::make_shared< SolverCplex>();
	}
	#endif
	else {
		std::cout << "SOLVER NON RECONU" << std::endl;
		std::exit(0);
	}

	std::cout << "Declaration solver: OK" << std::endl;
}

void print_rows(int n_elems, const std::vector<double>& matval, std::vector<int>& mstart,
	const std::vector<int>& mind, const std::vector<double>& rhs, const std::vector<char>& rtypes) {

	int c_size(0);
	int offset(0);
	mstart.push_back(n_elems);
	for (int j(1); j < mstart.size(); j++) {
		c_size = mstart[j] - mstart[j - 1];
		for (int i(offset); i < offset + c_size; i++) {
			std::cout << "  " << matval[i] << "x[" << mind[i] << "] + ";
		}

		if (rtypes[j - 1] == 'L') { std::cout << " <= "; }
		else if (rtypes[j - 1] == 'G') { std::cout << " >= "; }
		else if (rtypes[j - 1] == 'E') { std::cout << " = "; }
		else { std::cout << " " << rtypes[j - 1] << " "; }

		std::cout << rhs[j - 1] << std::endl;
		offset += c_size;
	}
}

void print_obj(const std::vector<double> &obj) {
	for (int i(0); i < obj.size(); i++) {
		if (i > 0) { std::cout << " + "; }
		std::cout << "  " << obj[i] << "x[" << i << "]";
	}
	std::cout << std::endl;
}

void get_and_print_rows(SolverAbstract::Ptr solver) {

	int n_elems = solver->get_nelems();
	int n_vars = solver->get_ncols();
	int n_cstr = solver->get_nrows();

	std::cout << std::endl << "Rows elements:" << std::endl;
	std::vector<double> matval(n_elems);
	std::vector<int>	mstart(n_cstr);
	std::vector<int>	mind(n_elems);
	solver->get_rows(mstart.data(), mind.data(), matval.data(), n_elems, &n_elems, 0, n_cstr - 1);

	std::vector<double> rhs(n_cstr);
	solver->get_rhs(rhs.data(), 0, n_cstr - 1);

	std::vector<char> rtypes(n_cstr);
	solver->get_row_type(rtypes.data(), 0, n_cstr - 1);

	// Print result
	std::cout << "N ELEMS: " << n_elems << std::endl;
	std::cout << "Size of matval: " << matval.size() << std::endl;
	std::cout << "Size of matind: " << mind.size() << std::endl;
	std::cout << "Size of mstart: " << mstart.size() << std::endl;
	print_rows(n_elems, matval, mstart, mind, rhs, rtypes);
}

void get_and_print_obj(SolverAbstract::Ptr solver) {
	int n_vars = solver->get_ncols();

	std::vector<double> obj(n_vars);
	solver->get_obj(obj.data(), 0, n_vars - 1);
	std::cout << "Objectif Value:" << std::endl;
	print_obj(obj);
}

void get_and_print_variables_bounds(SolverAbstract::Ptr solver) {
	int n_vars = solver->get_ncols();

	std::vector<double> lb(n_vars);
	std::vector<double> ub(n_vars);
	solver->get_lb(lb.data(), 0, n_vars - 1);
	solver->get_ub(ub.data(), 0, n_vars - 1);

	for (int i(0); i < n_vars; i++) {
		std::cout << "  " << lb[i] << " <= x[" << i << "] <= " << ub[i] << std::endl;
	}
}

void test_read_prob(const std::string solver_name, const std::string prob_name){

    SolverAbstract::Ptr solver;
	declaration_solver(solver, solver_name);

    const std::string flags = "MPS";
    solver->read_prob(prob_name.c_str(), flags.c_str());

    // 1. Number of variables
	int n_vars = solver->get_ncols();
	std::cout << "N VARS: " << n_vars << std::endl;

    // 2. Number of constraints
	int n_cstr = solver->get_nrows();
	std::cout << "N CONSTR: " << n_cstr << std::endl;

    // 3. Number of integer variables
	int n_Intvars = solver->get_n_integer_vars();
	std::cout << "N INTEGER VARS: " << n_Intvars << std::endl;

    // 4. Number of non zeros elements in the matrix
	int n_elems = solver->get_nelems();
	std::cout << "N ELEMS: " << n_elems << std::endl;

    // 5. Objectif function
	get_and_print_obj(solver);

	// 6. Rows elements and RHS
	get_and_print_rows(solver);

    // 8. Column types
	std::vector<char> coltype(n_vars);
	solver->get_col_type(coltype.data(), 0, n_vars-1);

    // 9. LB's & UB's on variables
	get_and_print_variables_bounds(solver);
}

void test_modify_prob(const std::string solver_name, const std::string prob_name){

    SolverAbstract::Ptr solver;
	declaration_solver(solver, solver_name);
	
    const std::string flags = "MPS";
    solver->read_prob(prob_name.c_str(), flags.c_str());
    


    // 1. Deleting rows
	std::cout << "Rows before modif" << std::endl;
	int n_elems = solver->get_nelems();
	int n_vars = solver->get_ncols();
	int n_cstr = solver->get_nrows();
	std::cout << std::endl << "  Rows elements:" << std::endl;
	std::vector<double> matval(n_elems);
	std::vector<int>	mstart(n_cstr);
	std::vector<int>	mind(n_elems);
	solver->get_rows(mstart.data(), mind.data(), matval.data(), n_elems, &n_elems, 0, n_cstr - 1);

	std::vector<double> rhs(n_cstr);
	solver->get_rhs(rhs.data(), 0, n_cstr - 1);

	std::vector<char> rtypes(n_cstr);
	solver->get_row_type(rtypes.data(), 0, n_cstr - 1);

	print_rows(n_elems, matval, mstart, mind, rhs, rtypes);

	solver->del_rows(0, 1);

	std::cout << "Rows after modif" << std::endl;
	get_and_print_rows(solver);


	// 2. Adding rows to the end
	// We add the two rows deleted before to the problem
	int ind_row2 = mstart[2];
	std::cout << "Niumber of elts to add: " << ind_row2 << std::endl;
	std::vector<double> vals(0);
	std::vector<int>	ind(0);
	int new_nz = 0;
	for (int i(0); i < ind_row2; i++) {
		vals.push_back(matval[i]);
		ind.push_back(mind[i]);
		new_nz += 1;
	}
	std::vector<int> newstart(0);
	newstart.push_back(mstart[0]);
	newstart.push_back(mstart[1]);


	std::vector<char> newtypes(0);
	newtypes.push_back(rtypes[0]);
	newtypes.push_back(rtypes[1]);

	std::vector<double> newrhs(0);
	newrhs.push_back(rhs[0]);
	newrhs.push_back(rhs[1]);

	solver->add_rows(2, new_nz, newtypes.data(), newrhs.data(), NULL, newstart.data(),
		ind.data(), vals.data());
	std::cout << "Rows after adding the deleted rows" << std::endl;
	get_and_print_rows(solver);


    // 3. Changing obj
	std::vector<double> obj(n_vars);
	std::vector<int> ids(n_vars);
	solver->get_obj(obj.data(), 0, n_vars - 1);
	print_obj(obj);
	for (int i(0); i < n_vars; i++) {
		ids[i] = i;
		obj[i] -= 1;
    }
	solver->chg_obj(n_vars, ids.data(), obj.data());
	std::cout << "Obj after substracting 1 to every value:" << std::endl;
	print_obj(obj);

    // 4. Changing RHS
	// rhs already got before
	// Adding one to every RHS
    solver->get_rhs(rhs.data(), 0, n_cstr - 1);
	for (int i(0); i < n_cstr; i++) {
		solver->chg_rhs(i, rhs[i] + 1);
	}
	get_and_print_rows(solver);

	// 5. Change coef
	// dividing last coef by 10
	solver->get_rows(mstart.data(), mind.data(), matval.data(), n_elems, &n_elems, 0, n_cstr - 1);
	int idcol = mind[mind.size() - 1];
	int row = n_cstr - 1;
	double val = matval[matval.size() - 1];
	std::cout << idcol << " " << row << " " << val << std::endl;
	solver->chg_coef(row, idcol, val/10.0);
	get_and_print_rows(solver);
}

void solve_problem(const std::string solver_name, const std::string prob_name){
    SolverAbstract::Ptr solver;
	declaration_solver(solver, solver_name);
	
    const std::string flags = "MPS";
    solver->read_prob(prob_name.c_str(), flags.c_str());
    
}