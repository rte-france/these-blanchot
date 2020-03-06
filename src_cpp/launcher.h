#pragma once

#include "common.h"
#include "SolverAbstract.h"
//#include "merge_mps_functions.h"
//#include <xprs.h>

class BendersOptions;
int build_input(BendersOptions const & options, CouplingMap & coupling_map);

BendersOptions build_benders_options(int argc, char** argv);

void sequential_launch(BendersOptions const &options);

void usage(int argc);

enum Attribute {
	INT_INDEX,
	INT_VECTOR_INDEX,
	CHAR_VECTOR_INDEX,
	DBL_VECTOR_INDEX,
	MAX_ATTRIBUTE
};

enum IntAttribute {
	NROWS,
	NCOLS,
	NELES,
	MAX_INT_ATTRIBUTE
};

enum IntVectorAttribute {
	MSTART,
	MINDEX,
	MAX_INT_VECTOR_ATTRIBUTE,
};

enum CharVectorAttribute {
	ROWTYPE,
	COLTYPE,
	MAX_CHAR_VECTOR_ATTRIBUTE
};

enum DblVectorAttribute {
	MVALUE,
	RHS,
	RANGE,
	OBJ,
	LB,
	UB,
	MAX_DBL_VECTOR_ATTRIBUTE
};

typedef std::tuple<IntVector, std::vector<IntVector>, std::vector<CharVector>, 
	std::vector<DblVector> > raw_standard_lp_data;

