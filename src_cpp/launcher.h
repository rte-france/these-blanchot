#pragma once

#include "common.h"
class BendersOptions;
int build_input(std::string const & root, std::string const & summary_name, problem_names & input);
void sequential_launch(std::string const & root, std::string const & structure, BendersOptions const &options);

void merge_mps(std::string const & root, std::string const & structure, BendersOptions const &options);