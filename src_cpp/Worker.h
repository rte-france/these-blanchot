#pragma once

#include "common.h"
#include "BendersOptions.h"
#include "Solver.h"


void XPRS_CC optimizermsg(XPRSprob prob, void* worker, const char *sMsg, int nLen, int nMsglvl);
class Worker;
typedef std::shared_ptr<Worker> WorkerPtr;

/*!
* \class Worker
* \brief Mother-class Worker 
*
*  This class opens and sets a problem from a mps and a mapping variable map
*/
class Worker
{
public:
	Worker();
	virtual void init(Str2Int const & variable_map, std::string const & path_to_mps) = 0;
	virtual ~Worker();
	virtual void get_value(double & lb) = 0;
	virtual void get_simplex_ite(int & result) = 0;
	virtual void free() = 0;

	virtual void get_ncols(int & ncols) = 0;
	virtual void get_nrows(int & nrows) = 0;

public:
	std::string _path_to_mps;
	Str2Int _name_to_id; /*!< Link between the variable name and its identifier */
	Int2Str _id_to_name; /*!< Link between the identifier of a variable and its name*/
	
public:
	/*!
	*  \brief Generate an error message
	*
	*  Method to manage the different errors we could encounter during the optimization process
	*
	*  \param sSubName : 
	*/
	std::list<std::ostream *> & stream();
	virtual void solve(int & lp_status) = 0;

public:
	AbstractSolver* solver;
	std::list<std::ostream * >_stream;
	bool _is_master;
};




void errormsg(XPRSprob & xprs,  const char *sSubName, int nLineNo, int nErrCode);
void optimizermsg(XPRSprob prob, void* worker, const char *sMsg, int nLen, int nMsglvl);

/*StrVector XPRS_LP_STATUS = {
	"XPRS_LP_UNSTARTED",
	"XPRS_LP_OPTIMAL",
	"XPRS_LP_INFEAS",
	"XPRS_LP_CUTOFF",
	"XPRS_LP_UNFINISHED",
	"XPRS_LP_UNBOUNDED",
	"XPRS_LP_CUTOFF_IN_DUAL",
	"XPRS_LP_UNSOLVED",
	"XPRS_LP_NONCONVEX"
};*/