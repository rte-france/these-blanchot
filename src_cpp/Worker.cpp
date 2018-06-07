#include "Worker.h"


std::list<std::ostream *> & Worker::stream() {
	return _stream;
}

/************************************************************************************\
* Name:         optimizermsg                                                         *
* Purpose:      Display Optimizer error messages and warnings.                       *
* Arguments:    const char *sMsg    Message string                                   *
*               int nLen            Message length                                   *
*               int nMsgLvl         Message type                                     *
* Return Value: None.                                                                *
\************************************************************************************/

void XPRS_CC optimizermsg(XPRSprob prob, void* worker, const char *sMsg, int nLen,
	int nMsglvl) {
	Worker * ptr = (Worker*)worker;
	if (!worker)
		throw std::invalid_argument("optimizermsg data is not Worker");
	switch (nMsglvl) {

		/* Print Optimizer error messages and warnings */
	case 4: /* error */
	case 3: /* warning */
	case 2: /* dialogue */
	case 1: /* information */
		for (auto const & stream : ptr->stream())
			*stream << sMsg << std::endl;
		break;
		/* Exit and flush buffers */
	default:
		fflush(NULL);
		break;
	}
}

/************************************************************************************\
* Name:         errormsg                                                             *
* Purpose:      Display error information about failed subroutines.                  *
* Arguments:    const char *sSubName    Subroutine name                              *
*               int nLineNo             Line number                                  *
*               int nErrCode            Error code                                   *
* Return Value: None                                                                 *
\************************************************************************************/

void Worker::errormsg(const char *sSubName, int nLineNo, int nErrCode) {
	int nErrNo; /* Optimizer error number */
				/* Print error message */
	printf("The subroutine %s has failed on line %d\n", sSubName, nLineNo);

	/* Append the error code if it exists */
	if (nErrCode != -1)
		printf("with error code %d.\n\n", nErrCode);

	/* Append Optimizer error number, if available */
	if (nErrCode == 32) {
		XPRSgetintattrib(_xprs, XPRS_ERRORCODE, &nErrNo);
		printf("The Optimizer eror number is: %d\n\n", nErrNo);
	}

	/* Free memory close files and exit */
	XPRSdestroyprob(_xprs);
	XPRSfree();
	exit(nErrCode);
}
Worker::Worker() {

}

Worker::~Worker() {
	
}

void Worker::free() {
	XPRSdestroyprob(_xprs);
}

/*!
*  \brief Return the optimal value of a problem
*
*  \param lb : double which receives the optimal value
*/
void Worker::get_value(double & lb) {
	XPRSgetdblattrib(_xprs, XPRS_LPOBJVAL, &lb);
}

/*!
*  \brief Initialization of a problem 
*
*
*  \param variable_map : map linking each problem name to its variables and their ids
*  \param problem_name : name of the problem
*/
void Worker::init(std::map<std::string, int> const & variable_map, std::string const & path_to_mps) {

	_stream.push_back(&std::cout);
	XPRScreateprob(&_xprs);
	//XPRSsetintcontrol(_xprs, XPRS_OUTPUTLOG, XPRS_OUTPUTLOG_FULL_OUTPUT);
	XPRSsetintcontrol(_xprs, XPRS_OUTPUTLOG, XPRS_OUTPUTLOG_NO_OUTPUT);
	XPRSsetintcontrol(_xprs, XPRS_THREADS, 1);
	XPRSsetcbmessage(_xprs, optimizermsg, this);
	XPRSreadprob(_xprs, path_to_mps.c_str(), "");

	//std::ifstream file(_path_to_mapping.c_str());
	_name_to_id = variable_map;
	for(auto & kvp : variable_map) {
		_id_to_name[kvp.second] = kvp.first;
	}
}

void Worker::solve() {
	int status = XPRSlpoptimize(_xprs, "");
	int lp_status;
	if (status) {
		std::cout << "Worker::solve() status " << status << std::endl;
		std::exit(0);
	}
	XPRSgetintattrib(_xprs, XPRS_LPSTATUS, &lp_status);
	if (lp_status != XPRS_LP_OPTIMAL) {
		std::cout << "lp_status is : " << lp_status << std::endl;
		std::exit(0);
	}

}

/*!
*  \brief Get the number of needed iteration to solve a problem
*
*  \param result : result
*/
void Worker::get_simplex_ite(int & result) {
	XPRSgetintattrib(_xprs, XPRS_SIMPLEXITER, &result);
}