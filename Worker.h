#pragma once

#include "common.h"
#include "xprs.h"


class Worker;
typedef std::shared_ptr<Worker> WorkerPtr;
/*! \class Worker
* \brief Mother-class Worker 
*
*  This class opens and sets a problem from a mps and a mapping files
*/

class Worker
{
public:
	/*!
	*  \brief Constructor
	*
	*  Constructor of class Worker
	*
	*  \param mps : path to the mps file (master or slave)
	*  \param mapping : path to the relevant mapping file 
	*/
	Worker();
	void init(std::string const & mps, std::string const & mapping); 
	virtual ~Worker();

	/*!
	*  \brief Set a new Lower Bound
	*
	*  Method to update the Lower Bound
	*
	*  \param lb : Current Lower Bound
	*/
	void get_value(double & lb) {
		XPRSgetdblattrib(_xprs, XPRS_LPOBJVAL, &lb);
	}

	void get_simplex_ite(int & result);

public:
	std::string _path_to_mps; /*!< path to the mps file*/
	std::string _path_to_mapping; /*!< path to the relevant mapping file*/

	std::map< std::string, int> _name_to_id; /*!< Link between the variable name and its identifier */
	std::map< int, std::string> _id_to_name; /*!< Link between the identifier of a variable and its name*/
public:

	/*!
	*  \brief Generate an error message
	*
	*  Method to manage the different errors we could encounter during the optimization process
	*
	*  \param sSubName : 
	*/
	void errormsg(const char *sSubName, int nLineNo, int nErrCode); 
	std::list<std::ostream *> & stream();

	/*!
	*  \brief Solve the problem
	*
	*  Method to solve the problem stocked in the instance Worker
	*/
	void solve() {
		XPRSlpoptimize(_xprs, "");
	}


public:
	XPRSprob _xprs; /*!< Problem stocked in the instance Worker*/
	std::list<std::ostream * >_stream; /*!< */
};

