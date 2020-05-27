#include "DataSMPS.h"

void analyze_time_file(std::string const& time_path, std::string& col_stage, std::string& row_stage)
{
	std::ifstream time_file(time_path);

	if (time_file) {
		int timer = 0;
		std::string line;

		while (getline(time_file, line)) {
			if (line[0] == ' ') {

			}
		}
	}
	else {
		std::cout << "TIME FILE " << time_path << " DOES NOT EXIST" << std::endl;
		std::exit(0);
	}

	time_file.close();
}
