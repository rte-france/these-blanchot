#include "WorkerSlave.h"

WorkerSlave::WorkerSlave() {

}

WorkerSlave::WorkerSlave(std::string const & mps, std::string const & mapping) {
	init(mps, mapping);

}
WorkerSlave::~WorkerSlave() {

}
void WorkerSlave::write(int it) {
	std::stringstream name;
	name << "slave_" << it << ".lp";
	XPRSwriteprob(_xprs, name.str().c_str(), "l");
}

/*!
*  \brief Fix the trial values in the slave
*
*  Method to set trial values in the slave problem by fixing their bounds
*
*  \param x0 : Trial values
*/
void WorkerSlave::fix_to(Point const & x0) {
	int nbnds((int)_name_to_id.size());
	std::vector<int> indexes(nbnds);
	std::vector<char> bndtypes(nbnds, 'B');
	std::vector<double> values(nbnds);

	int i(0);
	for (auto const & kvp : _id_to_name) {
		indexes[i] = kvp.first;
		values[i] = x0.find(kvp.second)->second;
		++i;
	}

	int status = XPRSchgbounds(_xprs, nbnds, indexes.data(), bndtypes.data(), values.data());
}

/*!
*  \brief Solve slave problem and get LP solution values
*
*  \param s : Empty point which receives the solution
*/
void WorkerSlave::get_subgradient(Point & s) {
	s.clear();
	int ncols;
	XPRSgetintattrib(_xprs, XPRS_COLS, &ncols);
	std::vector<double> ptr(ncols, 0);
	int status = XPRSgetlpsol(_xprs, NULL, NULL, NULL, ptr.data());
	for (auto const & kvp : _id_to_name) {
		s[kvp.second] = +ptr[kvp.first];
	}
}