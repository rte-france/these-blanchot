#include "common.h"



std::string get_mps(std::string const & problem_name) {
	return problem_name + ".mps";
}
std::string get_mapping(std::string const & problem_name) {
	return problem_name + "_coupling_variables.txt";
}
double norm_point(Point const & x0, Point const & x1) {
	double result(0);
	for (auto & kvp : x0) {
		result += (x0.find(kvp.first)->second - x1.find(kvp.first)->second)*(x0.find(kvp.first)->second - x1.find(kvp.first)->second);
	}
	result = std::sqrt(result);
	return result;
}

