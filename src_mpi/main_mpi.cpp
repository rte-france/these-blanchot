// projet_benders.cpp : définit le point d'entrée pour l'application console.
//
#include "Worker.h"
#include "Benders.h"
#include "BendersMPI.h"

int main(int argc, char** argv)
{

	XPRSinit("");

	if (argc < 2) {
		std::cout << "first argument is the directory where files are, second is the txt file containing problems' names" << std::endl;
		return 0;
	}
	else {
		std::cout << "argc = " << argc << std::endl;
	}
	std::string const root(argv[1]);
	std::cout << "root = " << root << std::endl;

	std::string const summary_name(root + "\\" + argv[2]);
	std::ifstream summary(summary_name, std::ios::in);
	if (!summary) {
		std::cout << "Cannot open file " << summary_name << std::endl;
		return 0;
	}

	problem_names input;
	std::string problem_line;

	while (std::getline(summary, problem_line))
	{
		input.insert(root + "\\" + problem_line);
	}

	summary.close();

	mpi::environment env;
	mpi::communicator world;
	BendersMpi bendersMpi;
	bendersMpi.load(input, env, world);
	bendersMpi.run(env, world);

	return 0;
}