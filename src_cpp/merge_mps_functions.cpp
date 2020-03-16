#include "merge_mps_functions.h"

/*!
*  \brief Creator of a Worker problem without any data
*
*  Creator of a Worker problem without any data
*
*  \param options : solver to set
*/
WorkerMerge::WorkerMerge(BendersOptions const& options)
{
	// Initialisation du solver adapte
	declare_solver(options.SOLVER);

	// Affichage du solver
	add_stream(std::cout);
	_solver->set_output_log_level(options.XPRESS_TRACE);

	_ncols		= 0;
	_nslaves	= 0;
}

/*!
*  \brief Creator of an empty Worker problem to fill it with all the .MPS problems 
*
*  Creator of an empty Worker problem to fill it with all the .MPS problems 
*
*  \param options : solver to set
*
*  \param input : total number of problems
*
*  \param name : Name of the Worker problem
*/
WorkerMerge::WorkerMerge(BendersOptions const& options, CouplingMap const& input, std::string const& name)
{
	// Initialisation du solver adapte
	declare_solver(options.SOLVER);

	// Affichage du solver
	add_stream(std::cout);
	_solver->set_output_log_level(1);

	// Charger le probleme vide
	_solver->load_lp(name.c_str(), 0, 0, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL);

	_ncols = 0;
	_nslaves = input.size() - 1;

	for (auto const& kvp : input) {
		_decalage[kvp.first] = 0;
	}
}

WorkerMerge::~WorkerMerge()
{
}

/*!
*  \brief Calling the free method of the solver
*/
void WorkerMerge::free()
{
	_solver->free();
}

/*!
*  \brief Reading a problem
*
*  Creator of an empty Worker problem to fill it with all the .MPS problems
*
*  \param problem_name : name of the file to read
*
*  \param flags : extension of the file to read
*/
void WorkerMerge::read(std::string const& problem_name, std::string const& flags)
{
	_solver->init(problem_name.c_str());
	_solver->read_prob(problem_name.c_str(), flags.c_str());
}

/*!
*  \brief Writing a problem
*
*  \param name : name of the file to write
*
*  \param flags : extension of the file to write
*/
void WorkerMerge::write_prob(std::string const& name, std::string const& flags)
{
	_solver->write_prob(name.c_str(), flags.c_str());
}


/*!
*  \brief Saving the id of the replication of a first-stage variable in the full problem to set up linking constraints
*
*  \param first_stage_var : pair<Problem_name, <var_name, Column ID> >
*/
void WorkerMerge::fill_mps_id(std::pair<std::string, Str2Int> first_stage_var)
{
	for (auto const& x : first_stage_var.second) {
		_x_mps_id[x.first][first_stage_var.first] = x.second;
	}
}

/*!
*  \brief Reading all the problems in input and merging them in the current problem
*
*  \param input : input of all the problems file to merge
*
*  \param options :path to the files
*/
void WorkerMerge::merge_problems(CouplingMap const& input, BendersOptions const& options) {

	for (auto const& kvp : input) {

		// Probleme name
		std::string problem_name(options.INPUTROOT + PATH_SEPARATOR + kvp.first);
		double const weight = options.slave_weight(input.size() - 1, problem_name);

		set_decalage(kvp.first);

		WorkerMerge prob(options);
		prob.read(problem_name, "MPS");

		if (kvp.first != options.MASTER_NAME) {
			prob.chg_obj(options, weight);
		}

		StandardLp lpData(prob);
		lpData.append_in(*this);

		prob.free();
		fill_mps_id(kvp);
	}

	add_coupling_constraints();
}

/*!
*  \brief Getter of the objective vector of the problem between columns index first and last
*
*  \param obj : the vector to fill
*
*  \param first : first index
*
*  \param last : last index
*/
void WorkerMerge::get_obj(DblVector& obj, int first, int last)
{
	_solver->get_obj(obj.data(), first, last);
}


/*!
*  \brief Getter of the solution vector and value
*
*  \param x0 : optimal point
*
*  \param val : optial value
*
*  \param input : first stage variables
*
*  \param options : options
*/
void WorkerMerge::get_optimal_point_and_value(Point& x0, double& val, CouplingMap & input, BendersOptions const& options) {
	DblVector ptr(get_ncols(), 0);
	get_MIP_sol(ptr.data(), NULL);

	for (auto const& kvp : input[options.MASTER_NAME]) {
		x0[kvp.first] = ptr[kvp.second];
	}

	val = get_mip_value();
}

/*!
*  \brief Multiply the objective vector of the problem by the weight value of the subproblem
*
*  \param options : options
*
*  \param weight : weight factor of the problem in the final objective
*/
void WorkerMerge::chg_obj(BendersOptions const& options, double weight)
{
	int mps_ncols = get_ncols();

	DblVector obj(mps_ncols, 0);
	IntVector sequence(mps_ncols);
	for (int i(0); i < mps_ncols; ++i) {
		sequence[i] = i;
	}

	get_obj(obj, 0, mps_ncols - 1);

	for (auto& c : obj) {
		c *= weight;
	}
	_solver->chg_obj(mps_ncols, sequence.data(), obj.data());
}

/*!
*  \brief Set the ID of the first column of the problem prb in the final problem
*
*  \param prb : problem name
*/
void WorkerMerge::set_decalage(std::string const& prb)
{
	_decalage[prb] = get_ncols();
}

/*!
*  \brief Add the couling constraints to link the first stage variables from all the problems in the final problem
*/
void WorkerMerge::add_coupling_constraints()
{
	IntVector mstart;
	IntVector cindex;
	DblVector values;
	int nrows(0);
	int neles(0);
	size_t neles_reserve(0);
	size_t nrows_reserve(0);
	for (auto const& kvp : _x_mps_id) {
		neles_reserve += kvp.second.size() * (kvp.second.size() - 1);
		nrows_reserve += kvp.second.size() * (kvp.second.size() - 1) / 2;
	}
	std::cout << "About to add " << nrows_reserve << " coupling constraints" << std::endl;
	values.reserve(neles_reserve);
	cindex.reserve(neles_reserve);
	mstart.reserve(nrows_reserve + 1);

	// adding coupling constraints
	for (auto const& kvp : _x_mps_id) {
		std::string const name(kvp.first);
		std::cout << name << std::endl;
		bool is_first(true);
		int id1(-1);
		std::string first_mps;
		for (auto const& mps : kvp.second) {
			if (is_first) {
				is_first = false;
				first_mps = mps.first;
				id1 = mps.second + _decalage.find(first_mps)->second;
			}
			else {
				int id2 = mps.second + _decalage.find(mps.first)->second;
				mstart.push_back(neles);
				cindex.push_back(id1);
				values.push_back(1);
				neles += 1;

				cindex.push_back(id2);
				values.push_back(-1);
				neles += 1;
				nrows += 1;
			}
		}
	}
	DblVector rhs(nrows, 0);
	CharVector sense(nrows, 'E');
	_solver->add_rows(nrows, neles, sense.data(), rhs.data(), NULL, mstart.data(), cindex.data(), values.data());
}

/*!
*  \brief return the number of columns of the problem
*/
int WorkerMerge::get_ncols()
{
	return _solver->get_ncols();
}

/*!
*  \brief Set the number of threads to solve the problem
*
*  \param n_threads : max number of threads
*/
void WorkerMerge::set_threads(int n_threads)
{
	_solver->set_threads(n_threads);
}
