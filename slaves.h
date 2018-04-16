#pragma once

#ifndef DEF_SLAVES
#define DEF_SLAVES

#include <string>
#include <vector>

class Slaves
{
public:
	
	void set_point(std::vector<double> TrialValues, int size);
	bool solve();
	double get_bound();
	void init(std::string filename);
	int size();
	void get_cut(std::vector<double> BendersCut, int size);
	int size(std::vector<double> BendersCut);

private:

};

#endif