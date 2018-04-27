#include "launcher.h"
#include "xprs.h"
#include "Benders.h"

int build_input(std::string const & root, std::string const & summary_name, problem_names & input) {
	input.clear();
	std::ifstream summary(summary_name, std::ios::in);
	if (!summary) {
		std::cout << "Cannot open file " << summary_name << std::endl;
		return 0;
	}
	std::string problem_name;
	while (std::getline(summary, problem_name))
	{
		input.insert(root + "\\" + problem_name);
	}

	summary.close();
	return 0;
}
void sequential_launch(std::string const & root, std::string const & structure) {
	XPRSinit("");
	problem_names input;
	build_input(root, structure, input);
	Benders benders(input);
	benders.run();
	benders.free();
	XPRSfree();
}