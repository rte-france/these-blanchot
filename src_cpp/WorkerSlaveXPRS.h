#pragma once

#include "WorkerSlave.h"
#include "xprs.h"
#include "SlaveCut.h"
#include "SimplexBasis.h"


void XPRS_CC optimizermsg(XPRSprob prob, void* worker, const char *sMsg, int nLen, int nMsglvl);

/*! 
* \class WorkerSlave
* \brief Class daughter of Worker Class, build and manage a slave problem
*/
class WorkerSlaveXPRS : public WorkerSlave {
public :
	XPRSprob _xprs;
	StrVector XPRS_LP_STATUS;

public:
	WorkerSlaveXPRS();
	WorkerSlaveXPRS(Str2Int const & variable_map, std::string const & path_to_mps, double const & slave_weight, BendersOptions const & options);
	virtual ~WorkerSlaveXPRS();

public :
	void init(Str2Int const & variable_map, std::string const & path_to_mps);
	void get_value(double & lb);
	void get_simplex_ite(int & result) ;
	void free();
	void solve(int & lp_status);
	void get_ncols(int & ncols);
	void get_nrows(int & nrows);

public:
	void write(int it);
	void fix_to(Point const & x0);
	void get_subgradient(Point & s);
	SimplexBasis get_basis();


};



void errormsg(XPRSprob & xprs,  const char *sSubName, int nLineNo, int nErrCode);
void optimizermsg(XPRSprob prob, void* worker, const char *sMsg, int nLen, int nMsglvl);


