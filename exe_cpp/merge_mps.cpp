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

		prob.free();
		full.fill_mps_id(kvp);
	}

	full.add_coupling_constraints();

	std::cout << "Solving" << std::endl;

	// Resolution sequentielle
	full.set_threads(1);

	int status = 0;
	full.solve_integer(status);

	Point x0;
	DblVector ptr(full.get_ncols(), 0);
	full.get_MIP_sol(ptr.data(), NULL);
	
	for (auto const & kvp : input[options.MASTER_NAME]) {
		x0[kvp.first] = ptr[kvp.second];
	}

	double val = full.get_mip_value();
	std::cout << "Optimal value " << val << std::endl;

	print_solution(std::cout, x0, true);

	full.free();

	return 0;
}