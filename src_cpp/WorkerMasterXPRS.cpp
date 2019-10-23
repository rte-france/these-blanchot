#include "WorkerMasterXPRS.h"


WorkerMasterXPRS::WorkerMasterXPRS() {
}


WorkerMasterXPRS::~WorkerMasterXPRS() {
}

/*!
*  \brief Constructor of a Master Problem
*
*  Construct a Master Problem by loading mps and mapping files and adding the variable alpha
*
*  \param variable_map : map linking each variable to its id in the problem
*  \param path_to_mps : path to the problem mps file
*  \param options : set of benders options
*  \param nslaves : number of slaves
*/
WorkerMasterXPRS::WorkerMasterXPRS(Str2Int const & variable_map, std::string const & path_to_mps, BendersOptions const & options, int nslaves) :WorkerMaster() {
	init(variable_map, path_to_mps);
	_is_master = true;
	if (options.XPRESS_TRACE == 1 || options.XPRESS_TRACE == 3) {
		XPRSsetintcontrol(_xprs, XPRS_OUTPUTLOG, XPRS_OUTPUTLOG_FULL_OUTPUT);
	}
	else {
		XPRSsetintcontrol(_xprs, XPRS_OUTPUTLOG, XPRS_OUTPUTLOG_NO_OUTPUT);
	}
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
	// add the variable alpha
	std::string const alpha("alpha");
	auto const it(_name_to_id.find(alpha));
	if (it == _name_to_id.end()) {
		double lb(-1e10); /*!< Lower Bound */
		double ub(+1e20); /*!< Upper Bound*/
		double obj(+1);
		double zero(0);
		std::vector<int> start(1, 0);
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
				matval[i + 1] = -1;
			}
			XPRSaddrows(_xprs, 1, nslaves + 1, rowtype.data(), rowrhs.data(), NULL, mstart.data(), mclind.data(), matval.data());
		}
	}
	else {
		std::cout << "ERROR a variable named alpha is in input" << std::endl;
	}
}


/*!
*  \brief Initialization of a problem 
*
*  \param variable_map : map linking each problem name to its variables and their ids
*
*  \param problem_name : name of the problem
*/
void WorkerMasterXPRS::init(Str2Int const & variable_map, std::string const & path_to_mps){
	_path_to_mps = path_to_mps;
	_stream.push_back(&std::cout);
	XPRScreateprob(&_xprs);
	XPRSsetintcontrol(_xprs, XPRS_OUTPUTLOG, XPRS_OUTPUTLOG_FULL_OUTPUT);
	XPRSsetintcontrol(_xprs, XPRS_OUTPUTLOG, XPRS_OUTPUTLOG_NO_OUTPUT);
	XPRSsetintcontrol(_xprs, XPRS_THREADS, 1);
	XPRSsetcbmessage(_xprs, optimizermsg, this);
	XPRSreadprob(_xprs, path_to_mps.c_str(), "");

	//std::ifstream file(_path_to_mapping.c_str());
	_name_to_id = variable_map;
	for(auto const & kvp : variable_map) {
		_id_to_name[kvp.second] = kvp.first;
	}
	_is_master = false;

	XPRS_LP_STATUS = {
		"XPRS_LP_UNSTARTED",
		"XPRS_LP_OPTIMAL",
		"XPRS_LP_INFEAS",
		"XPRS_LP_CUTOFF",
		"XPRS_LP_UNFINISHED",
		"XPRS_LP_UNBOUNDED",
		"XPRS_LP_CUTOFF_IN_DUAL",
		"XPRS_LP_UNSOLVED",
		"XPRS_LP_NONCONVEX"
	};
}

/*!
*  \brief Method to solve a problem
*
*  \param lp_status : problem status after optimization
*/
void WorkerMasterXPRS::solve(int & lp_status){
	int status(0);
	XPRSsetintcontrol(_xprs, XPRS_PRESOLVE, 0);
	
	status = XPRSlpoptimize(_xprs, "");
	XPRSgetintattrib(_xprs, XPRS_LPSTATUS, &lp_status);

	if (lp_status != XPRS_LP_OPTIMAL) {
		std::cout << "lp_status is : " << lp_status << std::endl;
		std::stringstream buffer;
		
		buffer << _path_to_mps << "_lp_status_";
		buffer << XPRS_LP_STATUS[lp_status];
		buffer<< ".mps";
		std::cout << "lp_status is : " << XPRS_LP_STATUS[lp_status] << std::endl;
		std::cout << "written in " << buffer.str() << std::endl;
		XPRSwriteprob(_xprs, buffer.str().c_str(), "x");
		std::exit(0);
	}
	else if (status) {
		std::cout << "Worker::solve() status " << status<<", "<<_path_to_mps << std::endl;	
	}
}

/*!
*  \brief Return the optimal value of a problem
*
*  \param lb : double which receives the optimal value
*/
void WorkerMasterXPRS::get_value(double & lb){
	XPRSgetdblattrib(_xprs, XPRS_LPOBJVAL, &lb);
}

/*!
*  \brief Get the number of iteration needed to solve a problem
*
*  \param result : result
*/
void WorkerMasterXPRS::get_simplex_ite(int & result){
	XPRSgetintattrib(_xprs, XPRS_SIMPLEXITER, &result);
}

void WorkerMasterXPRS::free(){
	XPRSdestroyprob(_xprs);
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
void WorkerMasterXPRS::get(Point & x0, double & alpha, DblVector & alpha_i) {
	x0.clear();
	std::vector<double> ptr(_id_alpha_i.back()+1, 0);
	XPRSgetsol(_xprs, ptr.data(), NULL, NULL, NULL);
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
void WorkerMasterXPRS::get_dual_values(std::vector<double> & dual) {
	int rows;
	XPRSgetintattrib(_xprs, XPRS_ROWS, &rows);
	dual.resize(rows);
	XPRSgetlpsol(_xprs, NULL, NULL, dual.data(), NULL);
}

/*!
*  \brief Return number of constraint in a problem
*/
int WorkerMasterXPRS::get_number_constraint() {
	int rows;
	XPRSgetintattrib(_xprs, XPRS_ROWS, &rows);
	return rows;
}

/*!
*  \brief Delete nrows last rows of a problem
*
*  \param nrows : number of rows to delete
*/
void WorkerMasterXPRS::delete_constraint(int const nrows) {
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

void WorkerMasterXPRS::write(int it) {
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
void WorkerMasterXPRS::add_cut(Point const & s, Point const & x0, double const & rhs) {
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
*  \brief Add benders cut to a problem
*
*  \param s : optimal slave variables
*  \param sx0 : subgradient times x0
*  \param rhs : optimal slave value
*/
void WorkerMasterXPRS::add_dynamic_cut(Point const & s, double const & sx0, double const & rhs) {
	// cut is -rhs >= alpha  + s^(x-x0)
	int nrows(1);
	int ncoeffs(1 + (int)_name_to_id.size());
	std::vector<char> rowtype(1, 'L');
	std::vector<double> rowrhs(1, 0);
	std::vector<double> matval(ncoeffs, 1);
	std::vector<int> mstart(nrows + 1, 0);
	std::vector<int> mclind(ncoeffs);

	rowrhs.front() -= rhs;
	rowrhs.front() += sx0;

	for (auto const & kvp : _name_to_id) {
		mclind[kvp.second] = kvp.second;
		matval[kvp.second] = s.find(kvp.first)->second;
	}

	mclind.back() = _id_alpha;
	matval.back() = -1;
	mstart.back() = (int)matval.size();

	XPRSaddrows(_xprs, nrows, ncoeffs, rowtype.data(), rowrhs.data(), NULL, mstart.data(), mclind.data(), matval.data());
}

/*!
*  \brief Add benders cut to a problem
*
*  \param i : identifier of a slave problem
*  \param s : optimal slave variables
*  \param sx0 : subgradient times x0
*  \param rhs : optimal slave value
*/
void WorkerMasterXPRS::add_cut_by_iter(int const i, Point const & s, double const & sx0, double const & rhs) {
	// cut is -rhs >= alpha  + s^(x-x0)
	int nrows(1);
	int ncoeffs(1 + (int)_name_to_id.size());
	std::vector<char> rowtype(1, 'L');
	std::vector<double> rowrhs(1, 0);
	std::vector<double> matval(ncoeffs, 1);
	std::vector<int> mstart(nrows + 1, 0);
	std::vector<int> mclind(ncoeffs);

	rowrhs.front() -= rhs;
	rowrhs.front() += sx0;

	for (auto const & kvp : _name_to_id) {
		mclind[kvp.second] = kvp.second;
		matval[kvp.second] = s.find(kvp.first)->second;
	}
	mclind.back() = _id_alpha_i[i];
	matval.back() = -1;
	mstart.back() = (int)matval.size();

	XPRSaddrows(_xprs, nrows, ncoeffs, rowtype.data(), rowrhs.data(), NULL, mstart.data(), mclind.data(), matval.data());
}


/*!
*  \brief Add one benders cut to a problem
*
*  \param i : identifier of a slave problem
*  \param s : optimal slave variables
*  \param x0 : optimal Master variables
*  \param rhs : optimal slave value
*/
void WorkerMasterXPRS::add_cut_slave(int i, Point const & s, Point const & x0, double const & rhs) {
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

/*!
*  \brief Fix an upper bound and the variable alpha of a problem
*
*  \param bestUB : bound to fix
*/
void WorkerMasterXPRS::fix_alpha(double const & bestUB) {
	std::vector<char> boundtype(1, 'U');
	XPRSchgbounds(_xprs, 1, &_id_alpha, boundtype.data(), &bestUB);
}


void WorkerMasterXPRS::get_ncols(int & ncols){
	XPRSgetintattrib(_xprs, XPRS_COLS, &ncols);
}

void WorkerMasterXPRS::get_nrows(int & nrows){
	XPRSgetintattrib(_xprs, XPRS_ROWS, &nrows);
}

void WorkerMasterXPRS::getlb_variables(BendersData & data, BendersOptions const & options, int nvars){
	XPRSgetlb(_xprs , data.global_lb.data(), 0, nvars - 1);
}

void WorkerMasterXPRS::getub_variables(BendersData & data, BendersOptions const & options, int nvars){
	XPRSgetub(_xprs , data.global_ub.data(), 0, nvars - 1);
}

void WorkerMasterXPRS::chgbounds(int nvars, std::vector<int> index_vars, std::vector<char> bnd_types, std::vector<double> values){
	XPRSchgbounds(_xprs, nvars, index_vars.data(), bnd_types.data(), values.data());
}

/*!
*  \brief Save values of alpha variables of the last iteration in order to compute to expected decrease of a subproblem
*
*/
void WorkerMasterXPRS::save_alpha_values(BendersData & data){
	data.previous_alpha_i = data.alpha_i;
}