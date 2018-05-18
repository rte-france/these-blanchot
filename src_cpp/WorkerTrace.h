#pragma once


#include "Worker.h"
#include "SlaveCut.h"

typedef std::tuple<int, std::string, std::string, int> KeyTrace;

class WorkerMasterData {
public:

	double _lb;
	double _ub;
	double _bestub;
	int _deleted_cut;
	PointPtr _x0;
	std::map<std::string,SlaveCutDataPtr> _cut_trace;

	double get_ub();
	double get_lb();
	double get_bestub();
	int get_simplexiter(std::string & slave_name);
	int get_deletedcut();
	Point get_point();
};

typedef std::shared_ptr<WorkerMasterData> WorkerMasterDataPtr;

class WorkerMasterTrace {
public:

	std::vector<WorkerMasterDataPtr> _master_trace;

	int get_ite();
};
