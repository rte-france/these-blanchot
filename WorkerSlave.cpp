#include "WorkerSlave.h"

void WorkerSlave::write(int it) {
	std::stringstream name;
	name << "slave_" << it << ".lp";
	XPRSwriteprob(_xprs, name.str().c_str(), "l");
}

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