// projet_benders.cpp : définit le point d'entrée pour l'application console.
//
#include "Worker.h"

int main(int argc, char** argv)
{
	std::string const root = "D:\\STAGES\\Enzo\\data\\cas1\\";
	WorkerMaster master(root + "master.mps", root + "master_coupling_variables.txt");
	WorkerSlave slave(root + "s1.mps", root + "s1_coupling_variables.txt");
	return 0;
}