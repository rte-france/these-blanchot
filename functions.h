#pragma once

#ifndef FUNCTIONS_INCLUDED
#define FUNCTIONS_INCLUDED

#include <iostream>
#include <string>

bool ismps(std::string filename);
bool islp(std::string filename);
bool stopping_criterion(double lb, double ub, bool ResMaster = true);


#endif