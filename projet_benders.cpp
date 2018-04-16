// projet_benders.cpp : définit le point d'entrée pour l'application console.
//
#include <iostream>
#include <iomanip>
#include <string>
#include <limits>
#include <vector>
#include "slaves.h"
#include "master.h"
#include "functions.h"

int main(int argc, char** argv)
{
	bool STOP(false);
	int iter(0);
	std::string filename;
	double lb;
	double ub;
	double SlavesBound;
	bool ResMaster;
	double MasterValue;
	int SizeTrial(0);
	int SizeBenders(0);

	Master master;
	Slaves slaves;

	master.init(filename);
	slaves.init(filename);

	std::vector<double> TrialValues;
	SizeTrial = master.fix_variables(TrialValues);


	lb = std::numeric_limits<float>::min();
	ub = std::numeric_limits<float>::max();

	STOP = stopping_criterion(lb, ub);

	std::vector<double> BendersCut;
	SizeBenders = slaves.size(BendersCut);


	while (!STOP) {
		iter++;

		slaves.set_point(TrialValues, SizeTrial);
		slaves.solve();
		SlavesBound = slaves.get_bound();


		if (SlavesBound < ub)
		{
			ub = SlavesBound;
		}
		slaves.get_cut(BendersCut, SizeBenders);

		master.add_cut(BendersCut, SizeBenders);
		master.solve();
		ResMaster = true;
		if (ResMaster)
		{
			MasterValue = master.get_value();
			master.get_point(TrialValues,SizeTrial);
		}
		if (MasterValue > lb)
		{
			lb = MasterValue;
		}
		// reserve des espace
		std::setw(10);
		// pour les flottants
		std::setprecision(10);
		std::cout << "Iteration : " << iter << std::endl;
		std::cout << "Master Value : " << MasterValue << std::endl;
		std::cout << "Upper bound : " << ub << "lower bound : " << lb << std::endl;

		for (int i = 0; i<SizeBenders; i++)
		{
			std::cout << "Benders cut from subproblem " << i << " : " << BendersCut[i] << std::endl;
		}

		STOP = stopping_criterion(lb, ub, ResMaster);
	}

	return 0;
}