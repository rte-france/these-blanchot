// projet_benders.cpp�: d�finit le point d'entr�e pour l'application console.
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

	BendersOptions options(build_benders_options(argc, argv));
	if (world.rank() == 0) {
		std::cout << "INPUTROOT      : " << options.INPUTROOT << std::endl;
		std::cout << "MASTER_NAME    : " << options.MASTER_NAME << std::endl;
		std::cout << "STRUCTURE_FILE : " << options.STRUCTURE_FILE << std::endl;
	}
	if (world.size() == 1) {
		sequential_launch(options);
	}
	else {
		Timer timer;
		XPRSinit("");
		CouplingMap input;
		build_input(options, input);
		BendersMpi bendersMpi;
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
