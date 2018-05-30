// projet_benders.cpp : définit le point d'entrée pour l'application console.
//
#include "launcher.h"
#include "BendersOptions.h"


int main(int argc, char** argv)
{
	//options.print(std::cout);
	usage(argc);
	std::string const root(argv[1]);
	std::string const summary_name(root + PATH_SEPARATOR + argv[2]);
	
	BendersOptions options;
	options.read(argv[3]);
	options.MASTER_NAME = root + PATH_SEPARATOR + options.MASTER_NAME;
	sequential_launch(root, summary_name, options);

	return 0;
}