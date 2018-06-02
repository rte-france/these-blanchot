// projet_benders.cpp : définit le point d'entrée pour l'application console.
//
#include "Worker.h"
#include "Timer.h"
#include "Benders.h"
#include "BendersMPI.h"
#include "launcher.h"

int main(int argc, char** argv)
{
	mpi::environment env;
	mpi::communicator world;

	if (world.rank() == 0)
		usage(argc);

	std::string const root(argv[1]);
	std::string const summary_name(argv[2]);
	if (world.size() == 1) {

		BendersOptions options;
		if (argc > 3) {
			options.read(argv[3]);
		}
		else {
			options.write_default();
		}
		options.MASTER_NAME = root + PATH_SEPARATOR + options.MASTER_NAME;
		sequential_launch(root, summary_name, options);
	}
	else {
		Timer timer;
		XPRSinit("");
		CouplingMap input;
		build_input(root, summary_name, input);

		BendersOptions options;
		if (argc > 3) {
			options.read(argv[3]);
		}
		else {
			options.write_default();
		}
		BendersMpi bendersMpi;
		options.MASTER_NAME = root + PATH_SEPARATOR + options.MASTER_NAME;
		bendersMpi.load(input, env, world, options);
		bendersMpi.run(env, world, std::cout);
		bendersMpi.free(env, world);
		XPRSfree();
		world.barrier();
		if (world.rank() == 0) {
			std::cout << "Problem ran in " << timer.elapsed() << " seconds" << std::endl;
		}
	}
	return(0);
}
