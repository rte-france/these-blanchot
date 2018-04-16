#pragma once

#include <string>
#include <vector>


class Master
{
public:

	void add_cut(std::vector<double> BendersCut, int size);
	void solve();
	double get_value();
	void get_point(std::vector<double> TrialValues, int size);
	void init(std::string filename);
	int fix_variables(std::vector<double> Trialvalues);
	
private:

};
