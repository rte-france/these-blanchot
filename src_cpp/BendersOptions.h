#pragma once 

#include "common.h"

const std::string DEFAULT_OPTIONS_TXT = "options_default.txt";
/*!
* \class BendersOptions
* \brief Class containing the options
*/
class BendersOptions {
public:

#define BENDERS_OPTIONS_MACRO(name__, type__, default__) type__ name__;
#include "BendersOptions.hxx"
#undef BENDERS_OPTIONS_MACRO

	BendersOptions();

	void read(std::string const & file_name);
	void print(std::ostream  & stream)const;

	void write_default();

	std::string get_master_path() const;
	std::string get_structure_path() const;
	std::string get_slave_path(std::string const & slave_name) const;

	double slave_weight(int nslaves, std::string const &)const;

	Str2Dbl _weights;
};


