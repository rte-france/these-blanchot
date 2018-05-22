#include "launcher.h"
#include "xprs.h"
#include "Benders.h"
#include "Timer.h"

#include "BendersOptions.h"

int build_input(std::string const & root, std::string const & summary_name, problem_names & input) {
	input.clear();
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
		buffer >> problem_name;
		input.insert(root + PATH_SEPARATOR + problem_name);
	}

	summary.close();
	return 0;
}
void sequential_launch(std::string const & root, std::string const & structure, BendersOptions const & options) {
	Timer timer;
	XPRSinit("");
	problem_names input;
	build_input(root, structure, input);
	Benders benders(input, options);
	benders.run(std::cout);
	benders.free();
	XPRSfree();
	std::cout << "Problem ran in " << timer.elapsed() << " seconds" << std::endl;
}