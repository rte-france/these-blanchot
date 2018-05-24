#include "common.h"



std::string get_mps(std::string const & problem_name) {
	return problem_name + ".mps";
}
std::string get_mapping(std::string const & problem_name) {
	return problem_name + "_coupling_variables.txt";
}
double norm_point(Point & x0, Point & x1) {
	double result(0);
	for (auto & kvp : x0) {
		result += (x0[kvp.first] - x1[kvp.first])*(x0[kvp.first] - x1[kvp.first]);
	}
	result = std::sqrt(result);
	return result;
}