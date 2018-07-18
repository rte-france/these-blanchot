#include "WorkerMaster.h"
#include "xprs.h"

WorkerMaster::WorkerMaster() {
}


WorkerMaster::~WorkerMaster() {
}

/*!
*  \brief Return optimal variables of a problem
*
*  Set optimal variables of a problem which has the form (min(x,alpha) : f(x) + alpha)
*
*  \param x0 : reference to an empty map list
*
*  \param alpha : reference to an empty double
*/
void WorkerMaster::get(Point & x0, double & alpha, std::vector<double> & alpha_i) {
	x0.clear();
	std::vector<double> ptr(_id_alpha_i.back()+1, 0);
	int status = XPRSgetsol(_xprs, ptr.data(), NULL, NULL, NULL);
	for (auto const & kvp : _id_to_name) {
		x0[kvp.second] = ptr[kvp.first];
	}
	alpha = ptr[_id_alpha];
	for (int i(0); i < _id_alpha_i.size(); ++i) {
		alpha_i[i] = ptr[_id_alpha_i[i]];
	}
}

/*!
*  \brief Set dual values of a problem in a vector
*
*  \param dual : reference to a vector of double
*/
void WorkerMaster::get_dual_values(std::vector<double> & dual) {
	int rows;
	XPRSgetintattrib(_xprs, XPRS_ROWS, &rows);
	dual.resize(rows);
	XPRSgetlpsol(_xprs, NULL, NULL, dual.data(), NULL);
}

/*!
*  \brief Return number of constraint in a problem
*/
int WorkerMaster::get_number_constraint() {
	int rows;
	XPRSgetintattrib(_xprs, XPRS_ROWS, &rows);
	return rows;
}

/*!
*  \brief Delete nrows last rows of a problem
*
*  \param nrows : number of rows to delete
*/
void WorkerMaster::delete_constraint(int const nrows) {
	std::vector<int> mindex(nrows, 0);
	int const nconstraint(get_number_constraint());
	for (int i(0); i < nrows; i++) {
		mindex[i] = nconstraint - nrows + i;
	}
	XPRSdelrows(_xprs, nrows, mindex.data());
}


/*!
*  \brief Write a problem in a lp file
*
*  \param it : number of the problem
*/

void WorkerMaster::write(int it) {
	std::stringstream name;
	name << "master_" << it << ".lp";
	XPRSwriteprob(_xprs, name.str().c_str(), "l");
}

/*!
*  \brief Add benders cut to a problem
*
*  \param s : optimal slave variables
*  \param x0 : optimal Master variables
*  \param rhs : optimal slave value
*/
void WorkerMaster::add_cut(Point const & s, Point const & x0, double const & rhs) {
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
		rowrhs.front() += (s.find(kvp.first)->second * x0.find(kvp.first)->second);
		mclind[kvp.second] = kvp.second;
		matval[kvp.second] = s.find(kvp.first)->second;
	}

	mclind.back() = _id_alpha;
	matval.back() = -1;
	mstart.back() = (int)matval.size();

	XPRSaddrows(_xprs, nrows, ncoeffs, rowtype.data(), rowrhs.data(), NULL, mstart.data(), mclind.data(), matval.data());
}


/*!
*  \brief Add several benders cut to a problem
*
*  \param i : identifier of a slave problem
*  \param s : optimal slave variables
*  \param x0 : optimal Master variables
*  \param rhs : optimal slave value
*/
void WorkerMaster::add_cut_slave(int i, Point const & s, Point const & x0, double const & rhs) {
	//std::cout << "adding " << i <<" | " << rhs << std::endl;
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
	mclind.back() = _id_alpha_i[i];
	matval.back() = -1;
	mstart.back() = (int)matval.size();

	XPRSaddrows(_xprs, nrows, ncoeffs, rowtype.data(), rowrhs.data(), NULL, mstart.data(), mclind.data(), matval.data());
}

void WorkerMaster::add_random_cut(IntVector const & random_slaves, DblVector const & slave_weight_coeff, Point const & s, Point const & x0, double const & rhs) {
	int ncols((int)_name_to_id.size());
	// cut is -rhs >= alpha  + s^(x-x0)
	int nrows(1);
	int ncoeffs((int)_name_to_id.size());
	std::vector<char> rowtype(1, 'L');
	std::vector<double> rowrhs(1, 0);
	std::vector<double> matval(ncoeffs, 1);
	std::vector<int> mstart(nrows + 1, 0);
	std::vector<int> mclind(ncoeffs);

	rowrhs.front() -= rhs;
	for (auto const & kvp : _name_to_id) {
		rowrhs.front() += (s.find(kvp.first)->second * x0.find(kvp.first)->second);
		mclind[kvp.second] = kvp.second;
		matval[kvp.second] = s.find(kvp.first)->second;
	}

	for (int i(0); i < random_slaves.size(); i++) {
		mclind.push_back(_id_alpha_i[random_slaves[i]]);
		matval.push_back(-slave_weight_coeff[random_slaves[i]]);
		mstart.back() = (int)matval.size();
		ncoeffs++;
	}

	XPRSaddrows(_xprs, nrows, ncoeffs, rowtype.data(), rowrhs.data(), NULL, mstart.data(), mclind.data(), matval.data());
}

/*!
*  \brief Constructor of a Master Problem
*
*  Construct a Master Problem by loading mps and mapping files and adding the variable alpha
*
*  \param mps : path to mps file
*  \param mapping : path to mapping
*  \param nslaves : number of Slaves problem
*/
WorkerMaster::WorkerMaster(std::map<std::string, int> const & variable_map, std::string const & path_to_mps, DblVector const & slave_weight, BendersOptions const & options, int nslaves) :Worker() {
	init(variable_map, path_to_mps);
	// 4 barrier
	// 2 dual
	if (options.MASTER_METHOD == "BARRIER") {
		XPRSsetintcontrol(_xprs, XPRS_DEFAULTALG, 4);
	}
	else if (options.MASTER_METHOD == "BARRIER_WO_CROSSOVER") {
		XPRSsetintcontrol(_xprs, XPRS_DEFAULTALG, 4);
		XPRSsetintcontrol(_xprs, XPRS_CROSSOVER, 0);
	}
	else {
		XPRSsetintcontrol(_xprs, XPRS_DEFAULTALG, 2);
	}
	//XPRSsetintcontrol(_xprs, XPRS_OUTPUTLOG, XPRS_OUTPUTLOG_NO_OUTPUT);
	// add the variable alpha
	std::string const alpha("alpha");
	auto const it(_name_to_id.find(alpha));
	if (it == _name_to_id.end()) {
		double lb(-1e10); /*!< Lower Bound */
		double ub(+1e20); /*!< Upper Bound*/
		double obj(+1);
		double zero(0);
		std::vector<int> start(2, 0);
		XPRSgetintattrib(_xprs, XPRS_COLS, &_id_alpha); /* Set the number of columns in _id_alpha */
		XPRSaddcols(_xprs, 1, 0, &obj, start.data(), NULL, NULL, &lb, &ub); /* Add variable alpha and its parameters */
		XPRSaddnames(_xprs, 2, alpha.c_str(), _id_alpha, _id_alpha);
		_id_alpha_i.resize(nslaves, -1);
		for (int i(0); i < nslaves; ++i) {
			XPRSgetintattrib(_xprs, XPRS_COLS, &_id_alpha_i[i]);
			XPRSaddcols(_xprs, 1, 0, &zero, start.data(), NULL, NULL, &lb, &ub); /* Add variable alpha_i and its parameters */
			std::stringstream buffer;
			buffer << "alpha_" << i;
			XPRSaddnames(_xprs, 2, buffer.str().c_str(), _id_alpha_i[i], _id_alpha_i[i]);
		}
		{
			std::vector<char> rowtype(1, 'E');
			std::vector<double> rowrhs(1, 0);
			std::vector<double> matval(nslaves+1, 0);
			std::vector<int> mclind(nslaves + 1);
			std::vector<int> mstart(1 + 1, 0);
			mclind[0] = _id_alpha;
			matval[0] = 1;

			for (int i(0); i < nslaves; ++i) {
				mclind[i + 1] = _id_alpha_i[i];
				matval[i + 1] = -slave_weight[i];
			}
			XPRSaddrows(_xprs, 1, nslaves + 1, rowtype.data(), rowrhs.data(), NULL, mstart.data(), mclind.data(), matval.data());
		}
	}
	else {
		std::cout << "ERROR a variable named alpha is in input" << std::endl;
	}
}

void WorkerMaster::fix_alpha(double const & bestUB) {
	std::vector<char> boundtype(1, 'U');
	int status = XPRSchgbounds(_xprs, 1, &_id_alpha, boundtype.data(), &bestUB);
}