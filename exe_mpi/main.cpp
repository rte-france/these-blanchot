// projet_benders.cpp : définit le point d'entrée pour l'application console.
//
#include "Worker.h"
#include "Benders.h"
#include "BendersMPI.h"
#include "launcher.h"

int main(int argc, char** argv)
{
	mpi::environment env;
	mpi::communicator world;

	if (argc < 2) {
		std::cout << "first argument is the directory where files are, second is the txt file containing problems' names" << std::endl;
		return 0;
	}
	else {
		std::cout << "argc = " << argc << std::endl;
	}
	std::string const root(argv[1]);
	std::string const summary_name(root + "\\" + argv[2]);
	if (world.size() == 1) {
		sequential_launch(root, summary_name);
	}
	else {
		XPRSinit("");
		problem_names input;
		build_input(root, summary_name, input);
		BendersMpi bendersMpi;
		bendersMpi.load(input, env, world);
		bendersMpi.run(env, world);
		bendersMpi.free(env, world);
		XPRSfree();
	}
	return(0);
}