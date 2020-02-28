// projet_benders.cpp : définit le point d'entrée pour l'application console.
//
#include "launcher.h"
#include "Worker.h"
#include "BendersOptions.h"
#include "BendersFunctions.h"
#include <cplex.h>


int main(int argc, char** argv)
{

	usage(argc);
	BendersOptions options(build_benders_options(argc, argv));
	options.print(std::cout);

	//XPRSinit("");
	CouplingMap input;
	build_input(options, input);
	
	//XPRSprob full;
	SolverAbstract::Ptr full;
	//XPRScreateprob(&full);
	
	//XPRSsetcbmessage(full, optimizermsg, NULL);
	//XPRSsetintcontrol(full, XPRS_OUTPUTLOG, XPRS_OUTPUTLOG_FULL_OUTPUT);
	full->load_lp("full", 0, 0, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
	//XPRSloadlp(full, "full", 0, 0, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
	Str2Int _decalage;
	int ncols(0);
	int nslaves(input.size());
	CouplingMap x_mps_id;


	for (auto const & kvp : input) {

		std::string problem_name(options.INPUTROOT + PATH_SEPARATOR + kvp.first);
		ncols = full->get_ncols();
		//XPRSgetintattrib(full, XPRS_COLS, &ncols);
		_decalage[kvp.first] = ncols;

		//XPRSprob prob;
		SolverAbstract::Ptr prob;

		//XPRScreateprob(&prob);
		prob->init(problem_name.c_str());
		//XPRSsetcbmessage(prob, optimizermsg, NULL);
		//XPRSsetintcontrol(prob, XPRS_OUTPUTLOG, XPRS_OUTPUTLOG_NO_OUTPUT);
		//XPRSreadprob(prob, problem_name.c_str(), "");
		
		if (kvp.first != options.MASTER_NAME) {

			int mps_ncols = prob->get_ncols();
			
			//XPRSgetintattrib(prob, XPRS_COLS, &mps_ncols);
			DblVector o(mps_ncols, 0);
			IntVector sequence(mps_ncols);
			for (int i(0); i < mps_ncols; ++i) {
				sequence[i] = i;
			}
			prob->get_obj(o.data(), 0, mps_ncols - 1);
			//XPRSgetobj(prob, o.data(), 0, mps_ncols - 1);
			double const weigth = options.slave_weight(nslaves, problem_name);
			for (auto & c : o) {
				c *= weigth;
			}
			prob->chg_obj(mps_ncols, sequence.data(), o.data());
			//XPRSchgobj(prob, mps_ncols, sequence.data(), o.data());
		}
		
		StandardLp lpData(prob);
		lpData.append_in(full);

		if (kvp.first == options.MASTER_NAME) {
			full->write_prob("full.lp", "l");
			//XPRSwriteprob(full, "full.lp", "l");
		}

		prob->free();
		//XPRSdestroyprob(prob);
		for (auto const & x : kvp.second) {
			//std::cout << x.first << " " << x.second << std::endl;
			x_mps_id[x.first][kvp.first] = x.second;
		}
	}


	IntVector mstart;
	IntVector cindex;
	DblVector values;
	int nrows(0);
	int neles(0);
	size_t neles_reserve(0);
	size_t nrows_reserve(0);
	for (auto const & kvp : x_mps_id) {
		neles_reserve += kvp.second.size()*(kvp.second.size() - 1);
		nrows_reserve += kvp.second.size()*(kvp.second.size() - 1) / 2;
	}
	std::cout << "About to add " << nrows_reserve << " coupling constraints" << std::endl;
	values.reserve(neles_reserve);
	cindex.reserve(neles_reserve);
	mstart.reserve(nrows_reserve + 1);
	// adding coupling constraints
	for (auto const & kvp : x_mps_id) {
		std::string const name(kvp.first);
		std::cout << name << std::endl;
		bool is_first(true);
		int id1(-1);
		std::string first_mps;
		for (auto const & mps : kvp.second) {
			if (is_first) {
				is_first = false;
				first_mps = mps.first;
				id1 = mps.second + _decalage.find(first_mps)->second;
			}
			else {
				int id2 = mps.second + _decalage.find(mps.first)->second;
				//std::cout << id1 << " - " << id2 << std::endl;
				// x[id1] - x[id2] = 0
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
	full->add_rows(nrows, neles, sense.data(), rhs.data(), NULL, mstart.data(), cindex.data(), values.data());
	//XPRSaddrows(full, nrows, neles, sense.data(), rhs.data(), NULL, mstart.data(), cindex.data(), values.data());

	//std::cout << "Writting mps file" << std::endl;
	//XPRSwriteprob(full, "full.mps", "");
	//std::cout << "Writting lp file" << std::endl; 
	//XPRSwriteprob(full, "full.lp", "l");
	std::cout << "Solving" << std::endl;
	
	// Resolution sequentielle
	
	// AJOUTER SET IN CONTROL THREADS !!!!!!
	//XPRSsetintcontrol(full, XPRS_THREADS, 1);
	int status = 0;
	full->solve_integer(status, "full.mps");
	//XPRSmipoptimize(full, "");

	Point x0;
	ncols = full->get_ncols();
	//XPRSgetintattrib(full, XPRS_COLS, &ncols);
	DblVector ptr(ncols, 0);
	full->get_MIP_sol(ptr.data(), NULL);
	//XPRSgetmipsol(full, ptr.data(), NULL);
	
	for (auto const & kvp : input[options.MASTER_NAME]) {
		x0[kvp.first] = ptr[kvp.second];
	}
	print_solution(std::cout, x0, true);

	full->free();
	//XPRSdestroyprob(full);
	

	// AJOUTER UN TEST QUE LE FREE A BIEN ETE APPELE A CET ETAPE !!!!

	//XPRSfree();

	return 0;
}