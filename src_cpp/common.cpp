#include "common.h"



std::string get_mps(std::string const & problem_name) {
	return problem_name + ".mps";
}
std::string get_mapping(std::string const & problem_name) {
	return problem_name + "_coupling_variables.txt";
}
