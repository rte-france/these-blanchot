#include "common.h"


/*!
*  \brief Return the mps file associate to a problem
*/
std::string get_mps(std::string const & problem_name) {
	return problem_name + ".mps";
}

/*!
*  \brief Return the distance between two point using 2-norm
*/
double norm_point(Point const & x0, Point const & x1) {
	double result(0);
	for (auto & kvp : x0) {
		result += (x0.find(kvp.first)->second - x1.find(kvp.first)->second)*(x0.find(kvp.first)->second - x1.find(kvp.first)->second);
	}
	result = std::sqrt(result);
	return result;
}

/*!
*  \brief Return the absolute distance between two vector of int
*/
int norm_int(IntVector const & x0, IntVector const & x1) {
	int result(0);
	if (x0.size() != x1.size()) {
		return result;
	}
	else {
		for (int i(0); i < x0.size(); i++) {
			result += abs(x1[i] - x0[i]);
		}
		return result;
	}
}

std::ostream & operator<<(std::ostream & stream, std::vector<IntVector> const & rhs) {
	int n(rhs.size());
	for (int i(0); i < n; i++) {
		for (int j(0); j < rhs[i].size(); j++) {
			stream << std::setw(3) << rhs[i][j];
		}
		stream << std::endl;
	}
	return stream;
}
