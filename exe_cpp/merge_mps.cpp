#include "merge_mps_functions.h"

int main(int argc, char** argv)
{

	usage(argc);

	BendersOptions options(build_benders_options(argc, argv));
	options.print(std::cout);

	Timer timer;
	CouplingMap input;
	build_input(options, input);

	// Declaration du WorkerMerge avec le probleme vide
	WorkerMerge full(options, input, "full");

	// Lecture et ajout de tous les problemes dans full
	full.merge_problems(input, options);

	std::cout << "Solving" << std::endl;

	// Resolution sequentielle
	full.set_threads(1);

	int status = 0;
	full.solve_integer(status, options, "full");

	Point x0;
	double val(0);
	full.get_optimal_point_and_value(x0, val, input, options);

	std::cout << "Optimal value " << val << std::endl;

	print_solution(std::cout, x0, true, status);
	std::cout << "Problem ran in " << timer.elapsed() << " seconds" << std::endl;
	full.free();

	return 0;
}