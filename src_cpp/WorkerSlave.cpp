#include "WorkerSlave.h"
#include "launcher.h"

WorkerSlave::WorkerSlave() {
	_proba = 1.0;
}


/*!
*  \brief Constructor of a Worker Slave
*
*  \param variable_map : Map of linking each variable of the problem to its id
*
*  \param problem_name : Name of the problem
*
*/
WorkerSlave::WorkerSlave(Str2Int const & variable_map, std::string const & path_to_mps, 
	double const & slave_weight, BendersOptions const & options) {
	init(variable_map, path_to_mps, options.SOLVER);
	_solver->set_output_log_level(options.XPRESS_TRACE);

	/*int mps_ncols;
	mps_ncols = _solver->get_ncols();

	DblVector o(mps_ncols, 0);
	IntVector sequence(mps_ncols);
	for (int i(0); i < mps_ncols; ++i) {
		sequence[i] = i;
	}
	_solver->get_obj(o.data(), 0, mps_ncols - 1);

	for (auto & c : o) {
		c *= slave_weight;
	}
	_solver->chg_obj(mps_ncols, sequence.data(), o.data());*/
	_proba = slave_weight;
	_solver->set_algorithm("DUAL");

}

/*!
*  \brief Constructor of a Worker Slave with SMPS data_format
*
*  \param variable_map : Map of linking each variable of the problem to its id
*
*  \param problem_name : Name of the problem
*
*  \param fictif : problem to copy
*
*/
WorkerSlave::WorkerSlave(Str2Int const& variable_map, std::string const& path_to_mps,
	double const& slave_weight, BendersOptions const& options, StrPairVector keys, 
	DblVector values, WorkerPtr fictif) {

	init(variable_map, path_to_mps, options.SOLVER, fictif);

	set_realisation_to_prob(keys, values);


	_solver->set_output_log_level(options.XPRESS_TRACE);

	/*int mps_ncols;
	mps_ncols = _solver->get_ncols();

	DblVector o(mps_ncols, 0);
	IntVector sequence(mps_ncols);
	for (int i(0); i < mps_ncols; ++i) {
		sequence[i] = i;
	}
	_solver->get_obj(o.data(), 0, mps_ncols - 1);

	for (auto& c : o) {
		c *= slave_weight;
	}
	_solver->chg_obj(mps_ncols, sequence.data(), o.data());*/
	_proba = slave_weight;
	_solver->set_algorithm("DUAL");
}

WorkerSlave::~WorkerSlave() {

}

void WorkerSlave::set_realisation_to_prob(StrPairVector keys, DblVector values)
{
	for (int k(0); k < keys.size(); k++) {
		int id_col, id_row;
		// 1. RHS
		if (keys[k].first == "RIGHT" ||
			keys[k].first == "RHS" ||
			keys[k].first == "RHS1") {
			id_row = _solver->get_row_index(keys[k].second);
			_solver->chg_rhs(id_row, values[k]);
		}
		// 2. MATRIX ELEMENT
		else {
			id_col = _solver->get_row_index(keys[k].first);
			id_row = _solver->get_row_index(keys[k].second);
			_solver->chg_coef(id_row, id_col, values[k]);
		}
	}
	/*for (auto const& kvp : realisation) {
		int id_col, id_row;
		 1. RHS
		if (kvp.first.first == "RIGHT" || 
			kvp.first.first == "RHS" || 
			kvp.first.first == "RHS1" ) {
			id_row = _solver->get_row_index(kvp.first.second);
			_solver->chg_rhs(id_row, kvp.second);
		}
		 2. MATRIX ELEMENT
		else {
			id_col = _solver->get_row_index(kvp.first.first);
			id_row = _solver->get_row_index(kvp.first.second);
			_solver->chg_coef(id_row, id_col, kvp.second);
		}
	}*/
}

/*!
*  \brief Write in a problem in an lp file 
*
* Method to write a problem in an lp file
*
*  \param it : id of the problem
*/
void WorkerSlave::write(int it) {
	std::stringstream name;
	name << "slave_" << it << ".lp";
	_solver->write_prob(name.str().c_str(), "l");
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

	_solver->chg_bounds(nbnds, indexes.data(), bndtypes.data(), values.data());
}

/*!
*  \brief Get LP solution value of a problem
*
*  \param s : Empty point which receives the solution
*/
void WorkerSlave::get_subgradient(Point & s) {
	s.clear();
	int ncols;
	ncols = _solver->get_ncols();
	std::vector<double> ptr(ncols, 0);
	_solver->get_LP_sol(NULL, NULL, NULL, ptr.data());
	for (auto const & kvp : _id_to_name) {
		s[kvp.second] = +ptr[kvp.first];
	}
}

void WorkerSlave::get_value(double& lb)
{
	//std::cout << "coucou" << std::endl;
	_solver->get_lp_value(lb);
	//lb *= _proba;
	//std::cout << lb << std::endl;
}

