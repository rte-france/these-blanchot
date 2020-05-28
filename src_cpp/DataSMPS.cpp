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

	time_file.close();
}

/*!< Generates master prob, struct file and base of slave problems slave_init.mps*/
void generate_base_of_instance(std::string const& cor_path, std::string const& inst_folder, 
	StrSet first_stage_vars, std::string const& col_stage, std::string const& row_stage) {

	std::ifstream cor_file(cor_path);

	std::ofstream master_file(inst_folder	+ "/master.mps");
	std::ofstream slave_file(inst_folder	+ "/slave_init.mps");
	std::ofstream struct_file(inst_folder	+ "/structure.txt");

	std::string current_part = "";
	
	// Contient les noms des contraintes pour chaque stage (master & slave)
	std::vector<StrSet> rows;
	rows.resize(2);

	StrSet written_vars;
	int period_row = 0;
	int period_col = 0;

	// On parcourt cor_file pour ecrire respectivement les lignes dans master ou slave
	std::string line, mot;
	std::stringstream ss;
	while (getline(cor_file, line)) {
		
		// Si [0] != ' ' : Key_word pour definir la partie dans laquelle on est
		if (line[0] == '*') {

		}
		else if (line[0] != ' ') {
			ss << line;
			ss >> current_part;
			ss.str("");
			ss.clear();

			period_col = 0;
			period_row = 0;
			master_file << line << std::endl;
			slave_file << line << std::endl;
		}
		else if (current_part == "ROWS") {
			analyze_row_line(line, rows, row_stage, master_file, slave_file, period_row);
		}
		else if (current_part == "COLUMNS") {
			analysze_col_line(line, col_stage, master_file, slave_file, struct_file,
				rows, written_vars, first_stage_vars, period_col);
		}
		else if (current_part == "RHS") {
			analyze_rhs_line(line, master_file, slave_file, rows);
		}
	}

	cor_file.close();
	master_file.close();
	slave_file.close();
	struct_file.close();
}

void analyze_row_line(std::string const& line, std::vector<StrSet>& rows, std::string const& row_stage, 
	std::ofstream& master_file, std::ofstream& slave_file, int& period_row)
{
	std::stringstream ss(line);
	std::string c_rowtype, c_rowname;
	ss >> c_rowtype >> c_rowname;

	// 1. On ajoute l'objectif dans les deux periodes master & slave
	if(c_rowtype == "N"){
		rows[0].insert(c_rowname);
		rows[1].insert(c_rowname);
		slave_file << line << std::endl;
	}

	// row_stage est la premiere ligne du stage 2
	// quand on la rencontre, on incremente la periode
	if (c_rowname == row_stage) {
		period_row += 1;
	}
	rows[period_row].insert(c_rowname);

	if (period_row == 0) {
		master_file << line << std::endl;
	}
	else if(period_row == 1) {
		slave_file << line << std::endl;
	}
	else {
		std::cout << "INVALID PERIOD : THE PROBLEM HAS MORE THAN 2 STAGES." << std::endl;
		std::exit(0);
	}
}

void analysze_col_line(std::string const& line, std::string const& col_stage, std::ofstream& master_file, 
	std::ofstream& slave_file, std::ofstream& struct_file, std::vector<StrSet>& rows, 
	StrSet& written_vars, StrSet& first_stage_vars, int& current_period)
{
	std::string c_colname, c_rowname, c_rowval;
	std::stringstream ss(line);
	ss >> c_colname;

	if (c_colname == col_stage) {
		current_period = 1;
	}

	while (ss >> c_rowname) {
		ss >> c_rowval;
		write_mps_line(current_period, master_file, slave_file, c_colname, c_rowname, c_rowval, rows);
	}

	if (current_period == 0) {
		write_struct_master(c_colname, struct_file, written_vars, first_stage_vars);
	}

}

void write_mps_line(int period, std::ofstream& master_file, std::ofstream& slave_file, 
	std::string const& colname, std::string const& rowname, std::string const& val, 
	std::vector<StrSet> const& rows)
{
	if (rows[0].find(rowname) != rows[0].end() && period == 0) {
		master_file << "    " << std::left 
			<< std::setw(10) << colname
			<< std::setw(10) << rowname
			<< std::setw(10) << val << std::endl;
	}
	else {
		slave_file << "    " << std::left
			<< std::setw(10) << colname
			<< std::setw(10) << rowname
			<< std::setw(10) << val << std::endl;
	}
}

void write_struct_master(std::string const& colname, std::ofstream& struct_file, 
	StrSet& written_vars, StrSet& first_stage_vars)
{
	if (written_vars.find(colname) == written_vars.end()) {
		struct_file
			<< std::setw(25) << std::left << "master"
			<< std::setw(15) << std::left << colname
			<< std::setw(10) << std::left << written_vars.size() << std::endl;
		written_vars.insert(colname);
		first_stage_vars.insert(colname);
	}
}

void analyze_rhs_line(std::string const& line, std::ofstream& master_file, 
	std::ofstream& slave_file, std::vector<StrSet> const& rows)
{
	std::stringstream ss(line);
	std::string c_type, c_rowname, c_rhs;

	ss >> c_type;
	while (ss >> c_rowname) {
		ss >> c_rhs;
		write_mps_line(0, master_file, slave_file, c_type, c_rowname, c_rhs, rows);
	}
}


