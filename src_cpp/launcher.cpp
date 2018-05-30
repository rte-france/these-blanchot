#include "launcher.h"
#include "xprs.h"
#include "Benders.h"
#include "Timer.h"

#include "BendersOptions.h"

int build_input(std::string const & root, std::string const & summary_name, CouplingMap & coupling_map) {
	coupling_map.clear();
	std::ifstream summary(summary_name, std::ios::in);
	if (!summary) {
		std::cout << "Cannot open file " << summary_name << std::endl;
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
		problem_name = root + PATH_SEPARATOR + problem_name;
		buffer >> variable_name;
		buffer >> variable_id;
		coupling_map[problem_name].insert(std::pair<std::string, int>(variable_name,variable_id));
	}
	summary.close();
	return 0;
}

void sequential_launch(std::string const & root, std::string const & structure, BendersOptions const & options) {
	Timer timer;
	XPRSinit("");
	CouplingMap input;
	build_input(root, structure, input);
	Benders benders(input, options);
	benders.run(std::cout);
	benders.free();
	XPRSfree();
	std::cout << "Problem ran in " << timer.elapsed() << " seconds" << std::endl;
}