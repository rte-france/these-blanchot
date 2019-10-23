#include "WorkerSlaveCPLX.h"
#include "launcher.h"


WorkerSlaveCPLX::WorkerSlaveCPLX() {
}

/*!
*  \brief Constructor of a Worker Slave
*
*  \param variable_map : Map of linking each variable of the problem to its id
*
*  \param problem_name : Name of the problem
*
*/
WorkerSlaveCPLX::WorkerSlaveCPLX(Str2Int const & variable_map, std::string const & path_to_mps, double const & slave_weight, BendersOptions const & options, AbstractSolver* solver) :WorkerSlave() {
	solver->get_env(_cplx);

	init(variable_map, path_to_mps);
	int status;

	if (options.XPRESS_TRACE == 2 || options.XPRESS_TRACE == 3) {
		status = CPXsetintparam(_cplx, CPXPARAM_Simplex_Display, 1);
	}
	else {
		status = CPXsetintparam(_cplx, CPXPARAM_Simplex_Display, 0);
	}

	int mps_ncols;
	mps_ncols = CPXgetnumcols(_cplx, _prb);

	DblVector o(mps_ncols, 0);
	IntVector sequence(mps_ncols);
	for (int i(0); i < mps_ncols; ++i) {
		sequence[i] = i;
	}
	status = CPXgetobj(_cplx, _prb, o.data(), 0, mps_ncols - 1);

	//std::cout << "slave_weight : " << slave_weight << std::endl;
	for (auto & c : o) {
		c *= slave_weight;
	}
	status = CPXchgobj(_cplx, _prb, mps_ncols, sequence.data(), o.data());

	status = CPXsetintparam (_cplx, CPXPARAM_LPMethod, CPX_ALG_DUAL);
}

WorkerSlaveCPLX::~WorkerSlaveCPLX() {

}


/*!
*  \brief Initialization of a problem 
*
*  \param variable_map : map linking each problem name to its variables and their ids
*
*  \param problem_name : name of the problem
*/
void WorkerSlaveCPLX::init(Str2Int const & variable_map, std::string const & path_to_mps){
	_path_to_mps = path_to_mps;
	_stream.push_back(&std::cout);

	int status(0);
	_prb = CPXcreateprob(_cplx, &status, path_to_mps.c_str());

	CPXsetintparam(_cplx, CPXPARAM_Threads, 1);
	// XPRSsetintcontrol(_xprs, XPRS_OUTPUTLOG, XPRS_OUTPUTLOG_FULL_OUTPUT);
	// XPRSsetintcontrol(_xprs, XPRS_OUTPUTLOG, XPRS_OUTPUTLOG_NO_OUTPUT);
	//XPRSsetintcontrol(_xprs, XPRS_THREADS, 1);
	//XPRSsetcbmessage(_xprs, optimizermsg, this);
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
void WorkerSlaveCPLX::solve(int & lp_status){
	int status(0);

	CPXsetintparam(_cplx, CPXPARAM_Preprocessing_Presolve, 0);
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
		std::cout << "Worker::solve() status " << status<<", "<<_path_to_mps << std::endl;	
	}
}

/*!
*  \brief Return the optimal value of a problem
*
*  \param lb : double which receives the optimal value
*/
void WorkerSlaveCPLX::get_value(double & lb){
	CPXsolution(_cplx, _prb, NULL, &lb, NULL, NULL, NULL, NULL);
}

/*!
*  \brief Get the number of iteration needed to solve a problem
*
*  \param result : result
*/
void WorkerSlaveCPLX::get_simplex_ite(int & result){
	result = CPXgetitcnt(_cplx, _prb);
}

void WorkerSlaveCPLX::free(){
	CPXfreeprob(_cplx, &_prb);
}

/*!
*  \brief Write in a problem in an lp file 
*
* Method to write a problem in an lp file
*
*  \param it : id of the problem
*/
void WorkerSlaveCPLX::write(int it) {
	std::stringstream name;
	name << "master_" << it << ".lp";
	std::string typ = "lp";
	CPXwriteprob(_cplx, _prb, name.str().c_str(), typ.c_str() );
}

/*!
*  \brief Fix a set of variables to constant in a problem
*
*  Method to set variables in a problem by fixing their bounds
*
*  \param x0 : Set of variables to fix
*/
void WorkerSlaveCPLX::fix_to(Point const & x0) {
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

	CPXchgbds(_cplx, _prb, 	nbnds, indexes.data(), bndtypes.data(), values.data() );
}

/*!
*  \brief Get LP solution value of a problem
*
*  \param s : Empty point which receives the solution
*/
void WorkerSlaveCPLX::get_subgradient(Point & s) {
	s.clear();
	int ncols;
	get_ncols(ncols);
	std::vector<double> ptr(ncols, 0.0);
	double test; 
	CPXsolution(_cplx, _prb, NULL, &test, NULL, NULL, NULL, ptr.data());

	double epsilon = 1e0;

	for (auto const & kvp : _id_to_name) {
		s[kvp.second] = +ptr[kvp.first];
	}

}

/*!
*  \brief Get simplex basis of a problem
*
*  Method to store simplex basis of a problem, and build the distance matrix
*/
SimplexBasis WorkerSlaveCPLX::get_basis() {
	int ncols;
	int nrows;
	IntVector cstatus;
	IntVector rstatus;
	get_ncols(ncols);
	get_nrows(nrows);
	cstatus.resize(ncols);
	rstatus.resize(nrows);
	//XPRSgetbasis(_xprs, rstatus.data(), cstatus.data());
	return std::make_pair(rstatus, cstatus);
}

void WorkerSlaveCPLX::get_ncols(int & ncols){
	ncols = CPXgetnumcols(_cplx, _prb);
}

void WorkerSlaveCPLX::get_nrows(int & nrows){
	nrows = CPXgetnumrows(_cplx, _prb);
}