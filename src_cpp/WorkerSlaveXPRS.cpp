#include "WorkerSlaveXPRS.h"
#include "launcher.h"


WorkerSlaveXPRS::WorkerSlaveXPRS() {
}

/*!
*  \brief Constructor of a Worker Slave
*
*  \param variable_map : Map of linking each variable of the problem to its id
*
*  \param problem_name : Name of the problem
*
*/
WorkerSlaveXPRS::WorkerSlaveXPRS(Str2Int const & variable_map, std::string const & path_to_mps, double const & slave_weight, BendersOptions const & options) :WorkerSlave() {
	init(variable_map, path_to_mps);
	if (options.XPRESS_TRACE == 2 || options.XPRESS_TRACE == 3) {
		XPRSsetintcontrol(_xprs, XPRS_OUTPUTLOG, XPRS_OUTPUTLOG_FULL_OUTPUT);
	}
	else {
		XPRSsetintcontrol(_xprs, XPRS_OUTPUTLOG, XPRS_OUTPUTLOG_NO_OUTPUT);
	}
	int mps_ncols;
	XPRSgetintattrib(_xprs, XPRS_COLS, &mps_ncols);
	DblVector o(mps_ncols, 0);
	IntVector sequence(mps_ncols);
	for (int i(0); i < mps_ncols; ++i) {
		sequence[i] = i;
	}
	XPRSgetobj(_xprs, o.data(), 0, mps_ncols - 1);
	//std::cout << "slave_weight : " << slave_weight << std::endl;
	for (auto & c : o) {
		c *= slave_weight;
	}
	XPRSchgobj(_xprs, mps_ncols, sequence.data(), o.data());
	XPRSsetintcontrol(_xprs, XPRS_DEFAULTALG, 2);
}

WorkerSlaveXPRS::~WorkerSlaveXPRS() {

}


/*!
*  \brief Initialization of a problem 
*
*  \param variable_map : map linking each problem name to its variables and their ids
*
*  \param problem_name : name of the problem
*/
void WorkerSlaveXPRS::init(Str2Int const & variable_map, std::string const & path_to_mps){
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
void WorkerSlaveXPRS::solve(int & lp_status){
	int status(0);
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
void WorkerSlaveXPRS::get_value(double & lb){
	XPRSgetdblattrib(_xprs, XPRS_LPOBJVAL, &lb);
}

/*!
*  \brief Get the number of iteration needed to solve a problem
*
*  \param result : result
*/
void WorkerSlaveXPRS::get_simplex_ite(int & result){
	XPRSgetintattrib(_xprs, XPRS_SIMPLEXITER, &result);
}

void WorkerSlaveXPRS::free(){
	XPRSdestroyprob(_xprs);
}

/*!
*  \brief Write in a problem in an lp file 
*
* Method to write a problem in an lp file
*
*  \param it : id of the problem
*/
void WorkerSlaveXPRS::write(int it) {
	std::stringstream name;
	name << "slave_" << it << ".lp";
	XPRSwriteprob(_xprs, name.str().c_str(), "l");
}

/*!
*  \brief Fix a set of variables to constant in a problem
*
*  Method to set variables in a problem by fixing their bounds
*
*  \param x0 : Set of variables to fix
*/
void WorkerSlaveXPRS::fix_to(Point const & x0) {
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

	XPRSchgbounds(_xprs, nbnds, indexes.data(), bndtypes.data(), values.data());
}

/*!
*  \brief Get LP solution value of a problem
*
*  \param s : Empty point which receives the solution
*/
void WorkerSlaveXPRS::get_subgradient(Point & s) {
	s.clear();
	int ncols;
	XPRSgetintattrib(_xprs, XPRS_COLS, &ncols);
	std::vector<double> ptr(ncols, 0);
	XPRSgetlpsol(_xprs, NULL, NULL, NULL, ptr.data());
	for (auto const & kvp : _id_to_name) {
		s[kvp.second] = +ptr[kvp.first];
	}

}

/*!
*  \brief Get simplex basis of a problem
*
*  Method to store simplex basis of a problem, and build the distance matrix
*/
SimplexBasis WorkerSlaveXPRS::get_basis() {
	int ncols;
	int nrows;
	IntVector cstatus;
	IntVector rstatus;
	XPRSgetintattrib(_xprs, XPRS_COLS, &ncols);
	XPRSgetintattrib(_xprs, XPRS_ROWS, &nrows);
	cstatus.resize(ncols);
	rstatus.resize(nrows);
	XPRSgetbasis(_xprs, rstatus.data(), cstatus.data());
	return std::make_pair(rstatus, cstatus);
}

void WorkerSlaveXPRS::get_ncols(int & ncols){
	XPRSgetintattrib(_xprs, XPRS_COLS, &ncols);
}

void WorkerSlaveXPRS::get_nrows(int & nrows){
	XPRSgetintattrib(_xprs, XPRS_ROWS, &nrows);
}