// projet_benders.cpp : définit le point d'entrée pour l'application console.
//
#include "Worker.h"

int main(int argc, char** argv)
{
	std::string const root = "D:\\STAGES\\Enzo\\data\\cas_autonomie\\";
	std::string const mps_master = root + "master.mps";
	std::string const mps_slave = root + "s1.mps";
	
	std::string const mapping_master = root + "master_coupling_variables.txt";
	std::string const mapping_slave = root + "s1_coupling_variables.txt";
	
	Benders benders(mps_master, mapping_master, mps_slave, mapping_slave);
	benders.run();
	return 0;
}