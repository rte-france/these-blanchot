#include "WorkerSlave.h"

WorkerSlave::WorkerSlave() {

}

WorkerSlave::WorkerSlave(std::map<std::string, int> const & variable_map, std::string const & problem_name) {
	init(variable_map, problem_name);
	
}
WorkerSlave::~WorkerSlave() {

}

void WorkerSlave::write(int it) {
	std::stringstream name;
	name << "slave_" << it << ".lp";
	XPRSwriteprob(_xprs, name.str().c_str(), "l");
}

/*!
*  \brief Fix a set of variables to constant in a problem
*
*  Method to set variables in a problem by fixing their bounds
*
*  \param x0 : Set of variables to fix
*/
void WorkerSlave::fix_to(Point const & x0) {
	int nbnds((int)_name_to_id.size());
	std::vector<int> indexes(nbnds);
	std::vector<char> bndtypes(nbnds, 'B');
	std::vector<double> values(nbnds);

	int i(0);
	for (auto const & kvp : _id_to_name) {
		indexes[i] = kvp.first;
		values[i] = x0.find(kvp.second)->second;
		++i;
	}

	int status = XPRSchgbounds(_xprs, nbnds, indexes.data(), bndtypes.data(), values.data());
}

/*!
*  \brief Get LP solution value of a problem
*
*  \param s : Empty point which receives the solution
*/
void WorkerSlave::get_subgradient(Point & s) {
	s.clear();
	int ncols;
	XPRSgetintattrib(_xprs, XPRS_COLS, &ncols);
	std::vector<double> ptr(ncols, 0);
	int status = XPRSgetlpsol(_xprs, NULL, NULL, NULL, ptr.data());
	for (auto const & kvp : _id_to_name) {
		s[kvp.second] = +ptr[kvp.first];
	}
}

void WorkerSlave::get_basis() {
	int ncols;
	int nrows;
	IntVector cstatus;
	IntVector rstatus;
	XPRSgetintattrib(_xprs, XPRS_COLS, &ncols);
	XPRSgetintattrib(_xprs, XPRS_ROWS, &nrows);
	cstatus.resize(ncols);
	rstatus.resize(nrows);
	int status = XPRSgetbasis(_xprs, rstatus.data(), cstatus.data());
	SimplexBasis basis(rstatus, cstatus);
	_basis.push_back(basis);
	IntVector distance_row(_basis.size(),0);
	IntVector distance_col(_basis.size(), 0);
	for (int i(0); i < _basis.size(); i++) {
		distance_row[i] = norm_int(_basis[i].first, basis.first);
		distance_col[i] = norm_int(_basis[i].second, basis.second);
	}
	_gap_col_basis.push_back(distance_col);
	_gap_row_basis.push_back(distance_row);
}

