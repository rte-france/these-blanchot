#include "WorkerMasterCPLX.h"

#ifdef CPLEX
WorkerMasterCPLX::WorkerMasterCPLX() {
}


WorkerMasterCPLX::~WorkerMasterCPLX() {
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
WorkerMasterCPLX::WorkerMasterCPLX(Str2Int const & variable_map, std::string const & path_to_mps, BendersOptions const & options,  AbstractSolver* solver, int nslaves) :WorkerMaster() {
	// On cree le pointeur interne vers l'environnement CPLEX
	solver->get_env(_cplx);

	init(variable_map, path_to_mps);
	_is_master = true;

	if (options.XPRESS_TRACE == 1 || options.XPRESS_TRACE == 3) {
		CPXsetintparam(_cplx, CPXPARAM_Simplex_Display, 1);
	}
	else {
		CPXsetintparam(_cplx, CPXPARAM_Simplex_Display, 0);
	}
	// 4 barrier
	// 2 dual
	if (options.MASTER_METHOD == "BARRIER") {
		CPXsetintparam (_cplx, CPXPARAM_LPMethod, CPX_ALG_BARRIER);
	}
	else if (options.MASTER_METHOD == "BARRIER_WO_CROSSOVER") {
		CPXsetintparam (_cplx, CPXPARAM_LPMethod, CPX_ALG_BARRIER);
		CPXsetintparam (_cplx, CPXPARAM_Barrier_Crossover, CPX_ALG_NONE);
	}
	else {
		CPXsetintparam (_cplx, CPXPARAM_LPMethod, CPX_ALG_DUAL);
	}

	// add the variable alpha
	std::string const alpha("alpha");
	auto const it(_name_to_id.find(alpha));
	if (it == _name_to_id.end()) {
		double lb(options.THETA_LB); /*!< Lower Bound */
		double ub(+1e20); /*!< Upper Bound*/
		double obj(+1);
		double zero(0);
		std::vector<int> start(1, 0);
		_id_alpha = CPXgetnumcols(_cplx, _prb);

		//CPXaddcols(_cplx, _prb, 1, 0, &obj, start.data(), NULL, NULL, &lb, &ub, alpha.c_str() );
		CPXaddcols(_cplx, _prb, 1, 0, &obj, start.data(), NULL, NULL, &lb, &ub, NULL ); 

		_id_alpha_i.resize(nslaves, -1);
		for (int i(0); i < nslaves; ++i) {
			_id_alpha_i[i] = CPXgetnumcols(_cplx, _prb); 
			std::stringstream buffer;
			buffer << "alpha_" << i;
			//CPXaddcols(_cplx, _prb, 1, 0, &zero, start.data(), NULL, NULL, &lb, &ub, buffer.str().c_str() ); /* Add variable alpha_i and its parameters */
			CPXaddcols(_cplx, _prb, 1, 0, &zero, start.data(), NULL, NULL, &lb, &ub, NULL ); /* Add variable alpha_i and its parameters */
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
			//XPRSaddrows(_xprs, 1, nslaves + 1, rowtype.data(), rowrhs.data(), NULL, mstart.data(), mclind.data(), matval.data());
			CPXaddrows(_cplx, _prb, 0, 1, nslaves + 1, rowrhs.data(), rowtype.data(), mstart.data(), mclind.data(), matval.data(), NULL, NULL);
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
void WorkerMasterCPLX::init(Str2Int const & variable_map, std::string const & path_to_mps){
	_path_to_mps = path_to_mps;
	_stream.push_back(&std::cout);
	int status;
	_prb = CPXcreateprob(_cplx, &status, "master");
	CPXsetintparam(_cplx, CPXPARAM_Threads, 1);
	// XPRSsetintcontrol(_xprs, XPRS_OUTPUTLOG, XPRS_OUTPUTLOG_FULL_OUTPUT);
	// XPRSsetintcontrol(_xprs, XPRS_OUTPUTLOG, XPRS_OUTPUTLOG_NO_OUTPUT);
	// XPRSsetcbmessage(_xprs, optimizermsg, this);
	std::string name = path_to_mps + ".mps";
	status = CPXreadcopyprob (_cplx, _prb, name.c_str(), NULL);

	//std::ifstream file(_path_to_mapping.c_str());
	_name_to_id = variable_map;
	for(auto const & kvp : variable_map) {
		_id_to_name[kvp.second] = kvp.first;
	}
	_is_master = false;

	CPLEX_LP_STATUS = {
		"0",
		"CPX_STAT_OPTIMAL",
		"CPX_STAT_UNBOUNDED",
		"CPX_STAT_INFEASIBLE",
		"CPX_STAT_INForUNBD",
		"CPX_STAT_OPTIMAL_INFEAS",
		"CPX_STAT_NUM_BEST"
	};

}

/*!
*  \brief Method to solve a problem
*
*  \param lp_status : problem status after optimization
*/
void WorkerMasterCPLX::solve(int & lp_status){
	int status(0);

	CPXsetintparam(_cplx, CPXPARAM_Preprocessing_Presolve, 0);
	CPXsetintparam(_cplx, CPXPARAM_Read_Scale, -1);
	CPXsetdblparam(_cplx, CPXPARAM_Simplex_Tolerances_Optimality, 1e0);

	status = CPXlpopt ((_cplx), _prb);
	CPXsolution(_cplx, _prb, &lp_status, NULL, NULL, NULL, NULL, NULL);

	if (lp_status != CPX_STAT_OPTIMAL) {
		std::cout << "lp_status is : " << lp_status << std::endl;
		std::stringstream buffer;
		
		buffer << _path_to_mps << "_lp_status_";
		buffer << CPLEX_LP_STATUS[lp_status];
		buffer<< ".mps";
		std::cout << "lp_status is : " << CPLEX_LP_STATUS[lp_status] << std::endl;
		std::cout << "written in " << buffer.str() << std::endl;
		std::string typ = "mps";
		status = CPXwriteprob(_cplx, _prb, buffer.str().c_str(), typ.c_str() );
		std::exit(0);
	}
	else if (status) {
		std::cout << "Worker::solve() status " << status << ", " << _path_to_mps << std::endl;	
	}
}

/*!
*  \brief Return the optimal value of a problem
*
*  \param lb : double which receives the optimal value
*/
void WorkerMasterCPLX::get_value(double & lb){
	CPXsolution(_cplx, _prb, NULL, &lb, NULL, NULL, NULL, NULL);
}

/*!
*  \brief Get the number of iteration needed to solve a problem
*
*  \param result : result
*/
void WorkerMasterCPLX::get_simplex_ite(int & result){
	result = CPXgetitcnt(_cplx, _prb);
}

void WorkerMasterCPLX::free(){
	CPXfreeprob(_cplx, &_prb);
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
void WorkerMasterCPLX::get(Point & x0, double & alpha, DblVector & alpha_i) {
	x0.clear();
	std::vector<double> ptr(_id_alpha_i.back()+1, 0);
	CPXsolution(_cplx, _prb, NULL, NULL, ptr.data(), NULL, NULL, NULL);
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
void WorkerMasterCPLX::get_dual_values(std::vector<double> & dual) {
	dual.resize( get_number_constraint() );
	CPXsolution(_cplx, _prb, NULL, NULL, dual.data(), NULL, NULL, NULL);
}

/*!
*  \brief Return number of constraint in a problem
*/
int WorkerMasterCPLX::get_number_constraint() {
	return CPXgetnumrows(_cplx, _prb);
}

/*!
*  \brief Delete nrows last rows of a problem
*
*  \param nrows : number of rows to delete
*/
void WorkerMasterCPLX::delete_constraint(int const nrows) {
	std::vector<int> mindex(nrows, 0);
	int const nconstraint(get_number_constraint());
	for (int i(0); i < nrows; i++) {
		mindex[i] = nconstraint - nrows + i;
	}
	CPXdelrows(_cplx, _prb, nconstraint-nrows, nconstraint-1);
}


/*!
*  \brief Write a problem in a lp file
*
*  \param it : number of the problem
*/

void WorkerMasterCPLX::write(int it) {
	std::stringstream name;
	name << "master_" << it << ".lp";
	std::string typ = "lp";
	CPXwriteprob(_cplx, _prb, name.str().c_str(), typ.c_str() );
}

/*!
*  \brief Add benders cut to a problem
*
*  \param s : optimal slave variables
*  \param x0 : optimal Master variables
*  \param rhs : optimal slave value
*/
void WorkerMasterCPLX::add_cut(Point const & s, Point const & x0, double const & rhs) {
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

	CPXaddrows(_cplx, _prb, 0, nrows, ncoeffs, rowrhs.data(), rowtype.data(), mstart.data(), mclind.data(), matval.data(), NULL, NULL);
}

/*!
*  \brief Add benders cut to a problem
*
*  \param s : optimal slave variables
*  \param sx0 : subgradient times x0
*  \param rhs : optimal slave value
*/
void WorkerMasterCPLX::add_dynamic_cut(Point const & s, double const & sx0, double const & rhs) {
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

	CPXaddrows(_cplx, _prb, 0, nrows, ncoeffs, rowrhs.data(), rowtype.data(), mstart.data(), mclind.data(), matval.data(), NULL, NULL);
}

/*!
*  \brief Add benders cut to a problem
*
*  \param i : identifier of a slave problem
*  \param s : optimal slave variables
*  \param sx0 : subgradient times x0
*  \param rhs : optimal slave value
*/
void WorkerMasterCPLX::add_cut_by_iter(int const i, Point const & s, double const & sx0, double const & rhs) {
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

	CPXaddrows(_cplx, _prb, 0, nrows, ncoeffs, rowrhs.data(), rowtype.data(), mstart.data(), mclind.data(), matval.data(), NULL, NULL);
}


/*!
*  \brief Add one benders cut to a problem
*
*  \param i : identifier of a slave problem
*  \param s : optimal slave variables
*  \param x0 : optimal Master variables
*  \param rhs : optimal slave value
*/
void WorkerMasterCPLX::add_cut_slave(int i, Point const & s, Point const & x0, double const & rhs) {
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
		//std::cout << kvp.second << "  " << matval[kvp.second] << std::endl;
 	}
	mclind.back() = _id_alpha_i[i];
	matval.back() = -1;
	mstart.back() = (int)matval.size();

	CPXaddrows(_cplx, _prb, 0, nrows, ncoeffs, rowrhs.data(), rowtype.data(), mstart.data(), mclind.data(), matval.data(), NULL, NULL);

}

/*!
*  \brief Fix an upper bound and the variable alpha of a problem
*
*  \param bestUB : bound to fix
*/
void WorkerMasterCPLX::fix_alpha(double const & bestUB) {
	std::vector<char> boundtype(1, 'U');
	CPXchgbds(_cplx, _prb, 1, &_id_alpha, boundtype.data(), &bestUB);

}


void WorkerMasterCPLX::get_ncols(int & ncols){
	ncols = CPXgetnumcols(_cplx, _prb);
}

void WorkerMasterCPLX::get_nrows(int & nrows){
	nrows = CPXgetnumrows(_cplx, _prb);
	
}

void WorkerMasterCPLX::getlb_variables(BendersData & data, BendersOptions const & options, int nvars){
	CPXgetlb(_cplx, _prb, data.global_lb.data(), 0, nvars-1);
}

void WorkerMasterCPLX::getub_variables(BendersData & data, BendersOptions const & options, int nvars){
	CPXgetub(_cplx, _prb, data.global_ub.data(), 0, nvars-1);	
}

void WorkerMasterCPLX::chgbounds(int nvars, std::vector<int> index_vars, std::vector<char> bnd_types, std::vector<double> values){
	CPXchgbds(_cplx, _prb, nvars, index_vars.data(), bnd_types.data(), values.data());
}

/*!
*  \brief Save values of alpha variables of the last iteration in order to compute to expected decrease of a subproblem
*
*/
void WorkerMasterCPLX::save_alpha_values(BendersData & data){
	data.previous_alpha_i = data.alpha_i;
}

#endif