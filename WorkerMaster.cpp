#include "WorkerMaster.h"
#include "xprs.h"

WorkerMaster::WorkerMaster() {

}


WorkerMaster::~WorkerMaster() {

}

/*!
*  \brief Solve the Master problem and set the optimal variables in x0
*
*  Method to get the optimal variables x0 and alpha
*
*  \param x0 : empty map list
*  \param alpha : empty double
*/
void WorkerMaster::get(Point & x0, double & alpha) {
	x0.clear();
	std::vector<double> ptr(_name_to_id.size() + 1, 0);
	int status = XPRSgetsol(_xprs, ptr.data(), NULL, NULL, NULL);
	for (auto const & kvp : _id_to_name) {
		x0[kvp.second] = ptr[kvp.first];
	}
	alpha = ptr[_id_alpha];
}

/*!
*  \brief Write the master problem
*
*  Method to write the master problem in a lp file
*
*  \param it : number of the problem
*/

void WorkerMaster::write(int it) {
	std::stringstream name;
	name << "master_" << it << ".lp";
	XPRSwriteprob(_xprs, name.str().c_str(), "l");
}

/*!
*  \brief Add cut to the Master Problem
*
*
*  \param s : optimal slave variables
*  \param x0 : optimal Master variables
*  \param rhs : optimal slave value
*/
void WorkerMaster::add_cut(Point const & s, Point const & x0, double rhs) {
	int ncols((int)_name_to_id.size());
	// cut is -rhs >= alpha  + s^(x-x0)
	int nrows(1);
	int ncoeffs(1 + (int)_name_to_id.size());
	std::vector<char> rowtype(1, 'L');
	std::vector<double> rowrhs(1, 0);
	std::vector<double> matval(ncoeffs, 1);
	std::vector<int> mstart(nrows + 1, 0);
	std::vector<int> mclind(ncoeffs);

	rowrhs.front() -= rhs;

	for (auto const & kvp : _name_to_id) {
		rowrhs.front() += s.find(kvp.first)->second * x0.find(kvp.first)->second;
		mclind[kvp.second] = kvp.second;
		matval[kvp.second] = s.find(kvp.first)->second;
	}
	mclind.back() = _id_alpha;
	matval.back() = -1;
	mstart.back() = (int)matval.size();

	XPRSaddrows(_xprs, nrows, ncoeffs, rowtype.data(), rowrhs.data(), NULL, mstart.data(), mclind.data(), matval.data());
}

/*!
*  \brief Constructor of Master Problem
*
*  Construct the Master Problem by loading the mps and mapping files and adding the variable alpha
*
*  \param mps : path to the mps file
*  \param mapping : path to the mapping
*/
WorkerMaster::WorkerMaster(std::string const & mps, std::string const & mapping) :Worker() {
	init(mps, mapping);
	// add the variable alpha
	std::string const alpha("alpha");
	auto const it(_name_to_id.find(alpha));
	if (it == _name_to_id.end()) {
		double lb(-1e10); /*!< Lower Bound */
		double ub(+1e20); /*!< Upper Bound*/
		double obj(+1);
		int zero(0);
		std::vector<int> start(2, 0);
		XPRSgetintattrib(_xprs, XPRS_COLS, &_id_alpha); /* Set the number of columns in _id_alpha */
		XPRSaddcols(_xprs, 1, 0, &obj, start.data(), NULL, NULL, &lb, &ub); /* Add variable alpha and its parameters */
		XPRSaddnames(_xprs, 2, alpha.c_str(), _id_alpha, _id_alpha);
	}
	else {
		std::cout << "ERROR a variable named alpha is in input" << std::endl;
	}
}
