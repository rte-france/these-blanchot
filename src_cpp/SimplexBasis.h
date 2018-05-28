#pragma once

#include "common.h"
#include "Worker.h"

typedef std::pair<IntVector, IntVector> SimplexBasis;

typedef std::shared_ptr<SimplexBasis> SimplexBasisPtr;

class SlaveCutTrimmer;
typedef std::set<SlaveCutTrimmer> SlaveCutStorage;

class SlaveCutDataHandler;
typedef std::shared_ptr<SlaveCutDataHandler> SlaveCutDataHandlerPtr;

typedef std::set<SlaveCutDataHandlerPtr, Predicate> SlaveCutDataHandlerPtrSet;

// alpha_i >= rhs+s(x-x0) = (rhs-s.x0)+s.x

class SimplexBasisHandler {
public:

	IntVector & get_col();
	IntVector & get_row();

	IntVector const & get_col()const;
	IntVector const & get_row()const;
	//void print(std::ostream & stream)const;

public:

	SimplexBasisHandler(SimplexBasisPtr const &data);
	SimplexBasisHandler(SimplexBasisPtr & data);
	virtual ~SimplexBasisHandler();

	bool operator<(SimplexBasisHandler const &  other)const;


	SimplexBasisPtr _data;
};

//class SlaveCutTrimmer {
//public:
//	SlaveCutDataHandlerPtr _data_cut;
//	Point _x0;
//
//	SlaveCutTrimmer(SlaveCutDataHandlerPtr & data, Point & x0);
//	double get_const_cut()const;
//	Point const & get_subgradient()const;
//
//	bool operator<(SlaveCutTrimmer const &  other)const;
//
//	void print(std::ostream & stream)const;
//
//
//
//};
//
//std::ostream & operator<<(std::ostream & stream, SlaveCutTrimmer const & rhs);
//
//std::ostream & operator<<(std::ostream & stream, SlaveCutData const & rhs);

