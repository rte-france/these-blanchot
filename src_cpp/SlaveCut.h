#pragma once

#include "common.h"
#include "Worker.h"

typedef std::pair<Point, IntVector> SlaveCutData1;
typedef std::pair<SlaveCutData1, DblVector> SlaveCutData2;
typedef std::pair<SlaveCutData2, StrVector> SlaveCutData3;
typedef SlaveCutData3 SlaveCutData;

typedef std::shared_ptr<SlaveCutData> SlaveCutDataPtr;

typedef std::map<std::string, SlaveCutData> SlaveCutPackage;

class SlaveCutTrimmer;
typedef std::set<SlaveCutTrimmer> SlaveCutStorage;

class SlaveCutDataHandler;
typedef std::map<std::string, SlaveCutStorage> AllCutStorage;
typedef std::shared_ptr<SlaveCutDataHandler> SlaveCutDataHandlerPtr;

typedef std::set<SlaveCutDataHandlerPtr, Predicate> SlaveCutDataHandlerPtrSet;

void build_SlaveCutData(SlaveCutData &);

enum SlaveCutInt {
	SIMPLEXITER = 0,
	MAXINT
};

enum SlaveCutDbl {
	SLAVE_COST = 0,
	ALPHA_I,
	MAXDBL 
};

enum SlaveCutStr {
	MAXSTR = 0
};

// alpha_i >= rhs+s(x-x0) = (rhs-s.x0)+s.x

class SlaveCutDataHandler {
public:

	Point & get_subgradient();
	IntVector & get_int();
	DblVector & get_dbl();
	StrVector & get_str();

	int & get_int(SlaveCutInt);
	double & get_dbl(SlaveCutDbl);
	std::string & get_str(SlaveCutStr);

	int get_int(SlaveCutInt)const;
	double get_dbl(SlaveCutDbl)const;
	std::string const & get_str(SlaveCutStr)const;

	Point const & get_subgradient()const;
	IntVector const & get_int()const;
	DblVector const & get_dbl()const;
	StrVector const & get_str()const;
	void print(std::ostream & stream)const;

public:

	SlaveCutDataHandler(SlaveCutDataPtr const &data);
	SlaveCutDataHandler(SlaveCutDataPtr & data);
	virtual ~SlaveCutDataHandler();

	SlaveCutDataPtr _data;
};

class SlaveCutTrimmer {
public:
	SlaveCutDataHandlerPtr _data_cut;
	Point _x0;

	SlaveCutTrimmer(SlaveCutDataHandlerPtr & data, Point & x0);
	double get_const_cut()const;
	Point const & get_subgradient()const;

	bool operator<(SlaveCutTrimmer const &  other)const;

	void print(std::ostream & stream)const;



};

std::ostream & operator<<(std::ostream & stream, SlaveCutTrimmer const & rhs);

std::ostream & operator<<(std::ostream & stream, SlaveCutData const & rhs);
