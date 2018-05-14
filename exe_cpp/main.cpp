// projet_benders.cpp : définit le point d'entrée pour l'application console.
//
#include "launcher.h"
#include "BendersOptions.h""


int main(int argc, char** argv)
{

	BendersOptions options;
	options.read("options.txt");
	//options.print(std::cout);

	if (argc < 2) {
		std::cout << "first argument is the directory where files are, second is the txt file containing problems' names" << std::endl;
		return 0;
	}
	else {
		std::cout << "argc = " << argc << std::endl;
	}
	std::string const root(argv[1]);
	std::string const summary_name(root + "\\" + argv[2]);

	sequential_launch(root, summary_name, options);
	return 0;
}