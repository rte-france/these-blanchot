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
	_solver->set_output_log_level(options.XPRESS_TRACE);

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

void WorkerMerge::read(std::string const& problem_name)
{
	_solver->init(problem_name.c_str());
}

void WorkerMerge::get_obj(DblVector& obj, int first, int last)
{
	_solver->get_obj(obj.data(), first, last);
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

	for (auto& c : o) {
		c *= weight;
	}
	_solver->chg_obj(mps_ncols, sequence.data(), obj.data());
}

void WorkerMerge::set_decalage(std::string const& prb)
{
	_decalage[prb] = get_ncols();
}

int WorkerMerge::get_ncols()
{
	return _solver->get_ncols();
}
