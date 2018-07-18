#include "BendersOptions.h"

BendersOptions::BendersOptions() {

#define BENDERS_OPTIONS_MACRO(name__, type__, default__) name__ = default__;
#include "BendersOptions.hxx"
#undef BENDERS_OPTIONS_MACRO

}
void BendersOptions::write_default(){
std::ofstream file("options_default.txt");
print(file);
file.close();

}
std::string BendersOptions::get_master_path() const {
	return INPUTROOT + PATH_SEPARATOR + MASTER_NAME;
}

std::string BendersOptions::get_structure_path() const {
	return INPUTROOT + PATH_SEPARATOR + STRUCTURE_FILE;
}
std::string BendersOptions::get_slave_path(std::string const & slave_name) const {
	return INPUTROOT + PATH_SEPARATOR + slave_name;
}

void BendersOptions::read(std::string const & file_name) {
	std::ifstream file(file_name.c_str());
	if (file.good()) {
		std::string line;
		std::string name;
		while (std::getline(file, line))
		{
			std::stringstream buffer(line);
			buffer >> name;
#define BENDERS_OPTIONS_MACRO(name__, type__, default__) if(#name__ == name) buffer >> name__;
#include "BendersOptions.hxx"
#undef BENDERS_OPTIONS_MACRO
		}


		if (SLAVE_WEIGHT != "UNIFORM" && SLAVE_WEIGHT != "CONSTANT") {
			std::string line;
			std::string filename = INPUTROOT + PATH_SEPARATOR + SLAVE_WEIGHT;
			std::ifstream file(filename);
			if (!file) {
				std::cout << "Cannot open file " << filename << std::endl;
			}
			while (std::getline(file, line))
			{
				std::stringstream buffer(line);
				std::string problem_name;
				buffer >> problem_name;
				buffer >> _weights[problem_name];
			}
		}

	}
	else {
		write_default();
	}
}

void BendersOptions::print(std::ostream & stream)const {
#define BENDERS_OPTIONS_MACRO(name__, type__, default__) stream<<std::setw(30)<<#name__<<std::setw(50)<<name__<<std::endl;
#include "BendersOptions.hxx"
#undef BENDERS_OPTIONS_MACRO
	stream << std::endl;
}

double BendersOptions::slave_weight(int nslaves, std::string const & name)const
{
	if (SLAVE_WEIGHT == "UNIFORM") {
		return 1 / static_cast<double>(nslaves);
	}
	else if (SLAVE_WEIGHT == "CONSTANT") {
		double weight(SLAVE_WEIGHT_VALUE);
		return 1 / weight;
	}
	else {
		return _weights.find(name)->second;
	}
}