#include "WorkerTrace.h"

int WorkerMasterTrace::get_ite() {
	return _master_trace.size();
}


double WorkerMasterData::get_ub() {
	return _ub;
}

double WorkerMasterData::get_lb() {
	return _lb;
}

double WorkerMasterData::get_bestub() {
	return _bestub;
}

int WorkerMasterData::get_simplexiter(std::string & slave_name) {
	SlaveCutDataHandler handler(_cut_trace[slave_name]);
	return handler.get_int(SIMPLEXITER);
}

int WorkerMasterData::get_deletedcut() {
	return _deleted_cut;
}

Point WorkerMasterData::get_point() {
	return *_x0;
}


