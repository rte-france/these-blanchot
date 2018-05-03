#pragma once

#include "common_mpi.h"
#include "Worker.h"
#include "WorkerSlave.h"
#include "WorkerMaster.h"

typedef std::pair<Point, IntVector> SlaveCutData1;
typedef std::pair<SlaveCutData1, DblVector> SlaveCutData2;
typedef std::pair<SlaveCutData2, StrVector> SlaveCutData3;
typedef SlaveCutData3 SlaveCutData;

typedef std::map<std::string, SlaveCutData> SlaveCutPackage;
typedef std::map<int, SlaveCutPackage> AllCutStorage;
//
//typedef std::set<SlaveCutTrimmer> SlaveCutStorage;


void build_SlaveCutData(SlaveCutData &);

enum SlaveCutInt {
	SIMPLEXITER = 0,
	MAXINT
};

enum SlaveCutDbl {
	SLAVE_COST = 0,

	MAXDBL
};

enum SlaveCutStr {
	MAXSTR=0
};

// alpha_i >= rhs+s(x-x0) = (rhs-s.x0)+s.x

class SlaveCutDataHandler {
public:

	void init();


	Point & get_point();
	IntVector & get_int();
	DblVector & get_dbl();
	StrVector & get_str();

	int & get_int(SlaveCutInt );
	double & get_dbl(SlaveCutDbl);
	std::string & get_str(SlaveCutStr);

public :

	SlaveCutDataHandler(SlaveCutData & data);
	virtual ~SlaveCutDataHandler();

	SlaveCutData & _data;
};

class SlaveCutTrimmer {
public:
	SlaveCutData _data_cut;
	Point _x0;

	SlaveCutTrimmer();
	SlaveCutTrimmer(SlaveCutData const & data, Point const & x0);
	double get_const_cut();
	bool operator<(SlaveCutTrimmer & other);

};