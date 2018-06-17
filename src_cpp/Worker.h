#pragma once

#include "common.h"
#include "xprs.h"


void XPRS_CC optimizermsg(XPRSprob prob, void* worker, const char *sMsg, int nLen, int nMsglvl);
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
	*  \param mps : path to mps file
	*  \param mapping : path to the relevant mapping file 
	*/
	Worker();
	void init(std::map<std::string, int> const & variable_map, std::string const & path_to_mps);
	virtual ~Worker();

	void get_value(double & lb);

	void get_simplex_ite(int & result);

	void free();
public:
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

	void solve(int & lp_status);


public:
	XPRSprob _xprs; /*!< Problem stocked in the instance Worker*/
	std::list<std::ostream * >_stream;
};

