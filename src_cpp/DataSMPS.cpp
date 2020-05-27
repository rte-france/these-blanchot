#include "DataSMPS.h"

/*!< .tim file contains the first row et col names of each stage
	We are searching for the ones of stage 2
*/
void analyze_time_file(std::string const& time_path, std::string& col_stage, std::string& row_stage)
{
	std::ifstream time_file(time_path);

	if (time_file) {
		int timer = 0;
		std::string line;
		std::string mot;
		int counter = 0;
		while (getline(time_file, line)) {
			if (line[0] == ' ') {
				counter = 0;
				if (timer == 1) {
					std::stringstream ss(line);
					while (counter < 2) {
						if (counter == 0) {
							ss >> col_stage;
						}
						else if (counter == 1) {
							ss >> row_stage;
						}
						counter++;
					}
				}
				timer++;
			}
		}
	}
	else {
		std::cout << "TIME FILE " << time_path << " DOES NOT EXIST" << std::endl;
		std::exit(0);
	}
	std::cout << "COL : " << col_stage << std::endl;
	std::cout << "ROW : " << row_stage << std::endl;
	std::exit(0);

	time_file.close();
}
