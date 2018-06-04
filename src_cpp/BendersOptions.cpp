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
	}
	else {
		write_default();
	}
}

void BendersOptions::print(std::ostream & stream)const {
#define BENDERS_OPTIONS_MACRO(name__, type__, default__) stream<<std::setw(20)<<#name__<<std::setw(40)<<name__<<std::endl;
#include "BendersOptions.hxx"
#undef BENDERS_OPTIONS_MACRO
	stream << std::endl;
}
