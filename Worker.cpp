#include "Worker.h"


bool Worker::IsInit = false;

std::list<std::ostream *> & Worker::stream() {
	return _stream;
}

/************************************************************************************\
* Name:         optimizermsg                                                              *
* Purpose:      Display Optimizer error messages and warnings.                            *
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

Worker::Worker(std::string const & mps,  std::string const & mapping) {

	_stream.push_back(&std::cout);

	if (!IsInit) {
		XPRSinit("");
		IsInit = true;
	}
	_path_to_mps = mps;
	_path_to_mapping = mapping;

	XPRScreateprob(&_xprs);
	XPRSsetcbmessage(_xprs, optimizermsg, this);
	XPRSreadprob(_xprs, mps.c_str(), "");

	std::ifstream file(mapping.c_str());
	std::string line;
	while (std::getline(file, line)) {
		if (!line.empty() && line.front() != '#') {
			std::stringstream buffer(line);
			std::string var_name;
			int var_id;
			buffer >> var_name;
			buffer >> var_id;
			_name_to_id[var_name] = var_id;
			_id_to_name[var_id] = var_name;

		}

	}
}


void Benders::run() {

}