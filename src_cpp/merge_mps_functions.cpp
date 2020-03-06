#include "merge_mps_functions.h"

/*void declare_solver(SolverAbstract::Ptr& solv, BendersOptions& options) {
	if (options.SOLVER == "") {
		std::cout << "SOLVER NON RECONU" << std::endl;
		std::exit(0);
	}
	#ifdef CPLEX
	else if (options.SOLVER == "CPLEX") {
		solv = std::make_shared< SolverCPLEX>();
	}
	#endif
	#ifdef XPRESS
	else if (options.SOLVER == "XPRESS") {
		solv = std::make_shared< SolverXPRESS>();
	}
	#endif
	else {
		std::cout << "SOLVER NON RECONU" << std::endl;
		std::exit(0);
	}
}
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

void WorkerMerge::free()
{
	_solver->free();
}

void WorkerMerge::read(std::string const& problem_name, std::string const& flags)
{
	_solver->init(problem_name.c_str());
	_solver->read_prob(problem_name.c_str(), flags.c_str());
}

void WorkerMerge::write_prob(std::string const& name, std::string const& flags)
{
	_solver->write_prob(name.c_str(), flags.c_str());
}

void WorkerMerge::fill_mps_id(std::pair<std::string, Str2Int> first_stage_var)
{
	for (auto const& x : first_stage_var.second) {
		_x_mps_id[x.first][first_stage_var.first] = x.second;
	}
}

void WorkerMerge::merge_problems(CouplingMap const& input, BendersOptions const& options) {

	for (auto const& kvp : input) {

		// Probleme name
		std::string problem_name(options.INPUTROOT + PATH_SEPARATOR + kvp.first);
		double const weight = options.slave_weight(input.size() - 1, problem_name);

		set_decalage(kvp.first);

		WorkerMerge prob(options);
		std::cout << problem_name << std::endl;
		prob.read(problem_name, "");

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

void WorkerMerge::get_obj(DblVector& obj, int first, int last)
{
	_solver->get_obj(obj.data(), first, last);
}

void WorkerMerge::get_optimal_point_and_value(Point& x0, double& val, CouplingMap & input, BendersOptions const& options) {
	DblVector ptr(get_ncols(), 0);
	get_MIP_sol(ptr.data(), NULL);

	for (auto const& kvp : input[options.MASTER_NAME]) {
		x0[kvp.first] = ptr[kvp.second];
	}

	val = get_mip_value();
}

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

void WorkerMerge::set_decalage(std::string const& prb)
{
	_decalage[prb] = get_ncols();
}

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

int WorkerMerge::get_ncols()
{
	return _solver->get_ncols();
}

void WorkerMerge::set_threads(int n_threads)
{
	_solver->set_threads(n_threads);
}
