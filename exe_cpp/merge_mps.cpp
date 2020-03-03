#include "merge_mps_functions.h"

int main(int argc, char** argv)
{

	usage(argc);
	BendersOptions options(build_benders_options(argc, argv));
	options.print(std::cout);

	CouplingMap input;
	build_input(options, input);

	// Declaration du WorkerMerge avec le probleme vide
	WorkerMerge full(options, input, "full");


	for (auto const & kvp : input) {

		// Probleme name
		std::string problem_name(options.INPUTROOT + PATH_SEPARATOR + kvp.first);
		double const weight = options.slave_weight(input.size() - 1, problem_name);

		full.set_decalage(kvp.first);
		
		WorkerMerge prob(options);
		prob.read(problem_name);

		if (kvp.first != options.MASTER_NAME) {
			prob.chg_obj(options, weight);
		}
		
		StandardLp lpData(prob);
		lpData.append_in(full);

		if (kvp.first == options.MASTER_NAME) {
			full->write_prob("full.lp", "l");
		}

		prob->free();
		for (auto const & x : kvp.second) {
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

	std::cout << "Solving" << std::endl;

	// Resolution sequentielle
	full->set_threads(1);

	int status = 0;
	full->solve_integer(status, "full");

	Point x0;
	ncols = full->get_ncols();
	DblVector ptr(ncols, 0);
	full->get_MIP_sol(ptr.data(), NULL);
	
	for (auto const & kvp : input[options.MASTER_NAME]) {
		x0[kvp.first] = ptr[kvp.second];
	}

	double val(0);
	full->get_mip_value(val);
	std::cout << "Optimal value " << val << std::endl;

	print_solution(std::cout, x0, true);

	full->free();

	return 0;
}