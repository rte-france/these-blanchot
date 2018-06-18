#include "launcher.h"
#include "xprs.h"
#include "Benders.h"
#include "Timer.h"

#include "BendersOptions.h"


BendersOptions build_benders_options(int argc, char** argv) {
	BendersOptions result;
	if (argc == 4) {
		result.read(argv[3]);
		result.INPUTROOT = argv[1];
		std::string const summary_name(argv[2]);
		result.STRUCTURE_FILE = summary_name;
	}
	else if (argc == 2){
		result.read(argv[1]);
	}
	else if (argc == 3) {
		result.read(argv[2]);
		result.INPUTROOT = argv[1];
	}
	return result;
}
/*!
*  \brief Build the input from the structure file
*
*	Function to build the map linking each problem name to its variables and their id
*
*  \param root : root of the structure file
*
*  \param summary_name : name of the structure file
*
*  \param coupling_map : empty map to increment
*/
int build_input(BendersOptions const & options, CouplingMap & coupling_map) {
	coupling_map.clear();
	std::ifstream summary(options.get_structure_path(), std::ios::in);
	if (!summary) {
		std::cout << "Cannot open file summary " << options.get_structure_path() << std::endl;
		return 0;
	}
	std::string line;

	while (std::getline(summary, line))
	{
		std::stringstream buffer(line);
		std::string problem_name;
		std::string variable_name;
		int variable_id;
		buffer >> problem_name;
		buffer >> variable_name;
		buffer >> variable_id;
		coupling_map[problem_name][variable_name] = variable_id;
	}
	int n(0);
	if (options.SLAVE_NUMBER >= 0) {
		CouplingMap trimmer;
		for (auto const & problem : coupling_map) {
			if (problem.first == options.MASTER_NAME)
				trimmer.insert(problem);
			else if (n< options.SLAVE_NUMBER){
				trimmer.insert(problem);
				++n;
			}
		}
		coupling_map = trimmer;
	}
	summary.close();
	return 0;
}


/*!
*  \brief Execute the Benders algorithm in sequential
*/
void sequential_launch(BendersOptions const & options) {
	Timer timer;
	XPRSinit("");
	CouplingMap input;
	build_input(options, input);
	Benders benders(input, options);
	benders.run(std::cout);
	benders.free();
	XPRSfree();
	std::cout << "Problem ran in " << timer.elapsed() << " seconds" << std::endl;
}



void merge_mps(BendersOptions const &options) {
	XPRSinit("");
	CouplingMap input;
	build_input(options, input);
	XPRSprob full;
	XPRScreateprob(&full);

	XPRSfree();
}

void usage(int argc) {
	if (argc < 2) {
		std::cout << "usage is : <exe> <root_dir> <structure_file> <option_file> " << std::endl;
		BendersOptions input;
		input.write_default();
		std::exit(0);
	}
	else {
		std::cout << "argc = " << argc << std::endl;
	}
}

