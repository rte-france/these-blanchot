#pragma once

#include "common.h"
class BendersOptions;
int build_input(BendersOptions const & options, CouplingMap & coupling_map);

BendersOptions build_benders_options(int argc, char** argv);

void sequential_launch(BendersOptions const &options);

void merge_mps(BendersOptions const &options);

void usage(int argc);
