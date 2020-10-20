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
		if (line[0] == '*') {	}
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

	write_struct_slave(inst_folder + "/slave_init.mps", first_stage_vars, struct_file);

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

void write_struct_slave(std::string const& slave_path, StrSet const& first_stage_vars, std::ofstream & struct_file)
{
	// On parcourt le slave_init.mps ecrit pour generer la partie struct associee aux slaves
	std::ifstream slave_file_read(slave_path);
	std::string line, colname;
	StrSet written_vars;
	written_vars.clear();
	std::stringstream ss;
	while (getline(slave_file_read, line)) {
		// Si [0] != ' ' : Key_word pour definir la partie dans laquelle on est
		if (line[0] != '*' || line[0] == ' ') {
			ss << line;
			ss >> colname;
			ss.str("");
			ss.clear();

			// Si la colonne qu'on lit est une variable de premier niveau inconnue (not in written_vars)
			// on l'ajoute a struct
			if (first_stage_vars.find(colname) != first_stage_vars.end()
				&& written_vars.find(colname) == written_vars.end()) {
				struct_file
					<< std::setw(25) << std::left << "slave"
					<< std::setw(15) << std::left << colname
					<< std::setw(10) << std::left << written_vars.size() << std::endl;
				written_vars.insert(colname);
			}
			
		}

	}
	slave_file_read.close();
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

void generate_number_of_realisations(Str2Int& blocks, std::string const& sto_path)
{
	std::ifstream cor_file(sto_path);
	std::string part_type = "";

	std::string line, key1, key2, val;
	std::string keyT;
	std::stringstream ss;
	while (getline(cor_file, line)){
		ss << line;
		if (line[0] != ' ') {
			if (line[0] != '*') {
				ss >> part_type;
			}
		}
		else if (part_type == "INDEP") {
			ss >> key1 >> key2;
			keyT = key1 + " " + key2;
			if (blocks.find(keyT) == blocks.end()) {
				blocks[keyT] = 0;
			}
			blocks[keyT] += 1;
		}
		else if (part_type == "BLOCKS") {
			ss >> key1 >> keyT;
			if (key1 == "BL") {
				if (blocks.find(keyT) == blocks.end()) {
					blocks[keyT] = 0;
				}
				blocks[keyT] += 1;
			}
		}
		else {
			std::cout << "UNKNOWN PART TYPE IN .STO FILE." << std::endl;
			std::exit(0);
		}
		ss.str("");
		ss.clear();
	}
	cor_file.close();
}

void read_struct_SMPS(BendersOptions const& options, CouplingMap& coupling_map, Str2Int blocks)
{
	coupling_map.clear();
	std::ifstream summary(options.get_structure_path(), std::ios::in);
	if (!summary) {
		std::cout << "Cannot open file summary " << options.get_structure_path() << std::endl;
		std::exit(0);
	}
	std::string line;

	// 1. Getting the number of subproblems
	long int n_sp = 1;
	for (auto const& kvp : blocks) {
		n_sp *= kvp.second;
	}

	if (n_sp <= 0) {
		n_sp = 100000;
		std::cout << "Nombre de realisations total trop grand pour un long int : fixe a 100 000." << std::endl;
	}

	if (options.SLAVE_NUMBER > n_sp) {
		std::cout << options.SLAVE_NUMBER << "  " << n_sp << std::endl;
		std::cout << "test " << (options.SLAVE_NUMBER > n_sp) << std::endl;
		std::cout << "Le nombre de slave doit etre inferieur a " << n_sp << std::endl;
		std::exit(0);
	}
	else if(options.SLAVE_NUMBER >= 0){
		n_sp = options.SLAVE_NUMBER;
	}
	else if (options.SLAVE_NUMBER == -1) {
		std::cout << "COUCOU LE -1" << std::endl;
	}

	std::cout << n_sp << std::endl;

	std::string c_sp_name;
	while (std::getline(summary, line))
	{
		std::stringstream buffer(line);
		std::string problem_name;
		std::string variable_name;
		int variable_id;
		buffer >> problem_name;
		buffer >> variable_name;
		buffer >> variable_id;
		if (problem_name == "slave") {
			for (int k(0); k < n_sp; k++) {
				c_sp_name = "SP_" + std::to_string(k);
				coupling_map[c_sp_name][variable_name] = variable_id;
			}
		}
		else if (problem_name == options.MASTER_NAME) {
			coupling_map[problem_name][variable_name] = variable_id;
		}
		
	}
	
	summary.close();
}

void go_to_next_realisation(Str2Int const& blocks, Str2Int& real_counter)
{
	int ind = 0;
	auto it(blocks.begin());
	real_counter[it->first]++;

	while (real_counter[it->first] == it->second) {
		real_counter[it->first] = 0;
		it++;
		real_counter[it->first] +=1;
	}
}

double find_rand_realisation_lines(StrPair2Dbl& realisation, std::string const& sto_path, Str2Int const& real_counter)
{

	double proba_tot = 1;
	Str2Int c_counter;
	for (auto const& kvp : real_counter) {
		c_counter[kvp.first] = -1;
	}

	std::ifstream sto_file(sto_path);
	std::string part_type = "";
	std::string line, key1, key2, period, value, proba;
	std::string name_of_real;
	std::stringstream ss;

	while (getline(sto_file, line)) {
		if (line[0] != ' ') {
			if (line[0] != '*') {
				ss << line;
				ss >> part_type;
				ss.str("");
				ss.clear();
			}
		}
		else if (part_type == "INDEP") {
			ss << line;
			ss >> key1 >> key2 >> value >> period >> proba;
			ss.str("");
			ss.clear();

			name_of_real = key1 + " " + key2;
			c_counter[name_of_real] += 1;

			if (c_counter[name_of_real] == real_counter.at(name_of_real) ) {
				realisation[std::pair<std::string, std::string>(key1, key2)] = std::stod(value);
				proba_tot *= std::stod(proba);
			}
		}
		else if (part_type == "BLOCKS") {
			std::cout << "A CODER : BLOCK PART OF STO FILES" << std::endl;
			std::exit(0);
		}
	}

	sto_file.close();

	return proba_tot;
}

void read_master_cstr(BendersData& data, BendersOptions const& options)
{
	std::string master_path = "master.mps";
	std::ifstream master_file(master_path);

	std::string part_type = "";
	std::string line, key1, key2, period, value, proba;
	std::stringstream ss;

	while (getline(master_file, line)) {
		// Si [0] != ' ' : Key_word pour definir la partie dans laquelle on est
		if (line[0] == '*') {}
		else if (line[0] != ' ') {
			ss << line;
			ss >> part_type;
			ss.str("");
			ss.clear();
		}
		else if (part_type == "ROWS") {
			ss << line;
			ss >> key1 >> key2;
			ss.str("");
			ss.clear();

			data.rhs[key2]		= 0;
			data.rowtype[key2]	= key1;
			data.A[key2]		= Point();
		}
		else if (part_type == "COLUMNS"){
			ss << line;
			ss >> key1 >> key2 >> value;
			ss.str("");
			ss.clear();

			data.A[key2][key1] = std::stod(value);
		}
		else if (part_type == "RHS") {
			ss << line;
			ss >> key1 >> key2 >> value;
			ss.str("");
			ss.clear();

			data.rhs[key2] = std::stod(value);
		}
	}

	master_file.close();

}

void write_deterministic_reformulation(BendersOptions const& options, std::string const& col_stage,
	std::string const& row_stage, std::string const& corpath, SMPSData & smps_data)
{

	// 1. On fixe la seed
	std::mt19937 rdgen;
	if (options.SEED != -1) {
		std::srand(options.SEED);
		rdgen.seed(options.SEED);
	}
	else {
		unsigned seedTime = std::chrono::system_clock::now().time_since_epoch().count();
		rdgen.seed(seedTime);
		std::srand((unsigned)time(NULL));
	}
	std::uniform_real_distribution<double> dis(0.0, 1.0);

	// Lines of realisations
	StrPairVector keys;
	DblVector values;
	IntVector real_counter;
	for (int i(0); i < smps_data.nbr_entries(); i++) {
		real_counter.push_back(0);
	}

	std::ifstream corfile(corpath);
	std::string name = "NSP" + std::to_string(options.SLAVE_NUMBER) + "_s" 
		+ std::to_string(options.SEED) + ".mps";
	std::ofstream RDfile(name);

	std::string line, key1, key2, key3, value, proba;
	std::string part_type = "";
	std::string cur_col = "";
	std::string period_row = "";
	std::string period_col = "";

	std::stringstream ss;

	// Vector part ROWS
	StrVector second_stage_rows;
	StrVector second_stage_types;

	// Vector part COLUMNS
	StrVector second_stage_cols;
	StrVector second_stage_ctr;
	StrVector second_stage_values;
	std::string colname, rowname ;
	double colval;

	// Vector part BOUNDS
	StrVector bnd_type;
	StrVector bnd_col;
	StrVector bnd_val;

	// Vector part RHS
	StrVector rhs_row;
	DblVector rhs_val;
	double rhs_value;

	second_stage_rows.clear();

	while (getline(corfile, line)) {
		if (line[0] == '*') {
			RDfile << line << std::endl;
		}
		else if (line[0] != ' ') {
			
			if (part_type == "ROWS") {
				for (int k(1); k <= options.SLAVE_NUMBER; k++) {
					for (int i(0); i < second_stage_rows.size(); i++) {
						RDfile << std::setw(5) << second_stage_types[i] << "    " << second_stage_rows[i]
							<< "_" << k << std::endl;
					}
				}
			}
			else if (part_type == "COLUMNS") {
				for (int k(1); k <= options.SLAVE_NUMBER; k++) {
					for (int i(0); i < second_stage_cols.size(); i++) {
						colname = second_stage_cols[i] + "_" + std::to_string(k);
						rowname = second_stage_ctr[i];
						colval = std::stod(second_stage_values[i]);

						if (second_stage_ctr[i] != "OBJ" 
							&& second_stage_ctr[i] != "OBJ00000"
							&& second_stage_ctr[i] != "OBJECTRW") {
							rowname += "_" + std::to_string(k);
						}
						else {
							colval /= options.SLAVE_NUMBER;
						}

						RDfile << "    " << std::left << std::setw(20) << colname <<
							"    " << rowname << "       " <<
							colval << std::endl;
					}
				}
			}
			else if (part_type == "RHS") {
				for (int k(1); k <= options.SLAVE_NUMBER; k++) {

					smps_data.go_to_next_realisation(real_counter, options, rdgen, dis);
					proba = smps_data.find_rand_realisation_lines(keys, values, real_counter);

					/*for (int j(0); j < keys.size(); j++) {
						std::cout << keys[j].first << "   " << keys[j].second << "  " << values[j] << std::endl;
					}

					std::cout << std::endl;*/

					//std::cout << "TAILLE " << rhs_val.size() << std::endl;

					for (int i(0); i < rhs_val.size(); i++) {
						//std::cout << rhs_val[i] << std::endl;

						rhs_value = rhs_val[i];
						for (int j(0); j < keys.size(); j++) {
							if (keys[j].second == rhs_row[i]) {
								rhs_value = values[j];
							}
						}

						RDfile << "    RHS"
							<< "    " << rhs_row[i] << "_" << k
							<< "    " << rhs_value
							<< std::endl;
					}

					keys.clear();
					values.clear();
				}
			}
			else if (part_type == "BOUNDS") {
				for (int k(1); k <= options.SLAVE_NUMBER; k++) {
					for (int i(0); i < bnd_type.size(); i++) {
						RDfile << "    " << std::left << bnd_type[i]
							<< "    " << "BND"
							<< "    " << bnd_col[i] << "_" << k
							<< "    " << bnd_val[i] << std::endl;
					}
				}
			}


			ss << line;
			ss >> part_type;
			ss.str("");
			ss.clear();

			RDfile << line << std::endl;
			period_row = "first";
			period_col = "first";
		}
		else if (part_type == "ROWS") {
			ss << line;
			ss >> key1 >> key2;
			ss.str("");
			ss.clear();

			if (key2 == row_stage) {
				period_row = "second";
			}

			if (period_row == "second") {
				/*for (int k(1); k <= options.SLAVE_NUMBER; k++) {
					RDfile << std::setw(5) << key1 << "    " << key2 << "_" << k << std::endl;
				}*/

				second_stage_types.push_back(key1);
				second_stage_rows.push_back(key2);
			}
			else {
				RDfile << std::setw(5) << key1 << "    " << key2 << std::endl;
			}

		}
		else if (part_type == "COLUMNS") {
			ss << line;
			ss >> key1;
			
			while (ss >> key2) {
				ss >> value;

				if (key1 == col_stage) {
					period_col = "second";
				}

				if (key1 != cur_col) {
					period_row = "first";
				}

				if (period_col == "first") {

					if (std::find(second_stage_rows.begin(), second_stage_rows.end(), key2) 
						!= second_stage_rows.end()) {
						for (int k(1); k <= options.SLAVE_NUMBER; k++) {
							RDfile << "    " << std::left << std::setw(20) << key1 
								<< "     " << key2 << "_" << k << "      " << value << std::endl;
						}
					}
					else {
						RDfile << "    " << key1
							<< "    " << key2 <<
							"     " << value << std::endl;
					}
				}
				else {
					second_stage_cols.push_back(key1);
					second_stage_ctr.push_back(key2);
					second_stage_values.push_back(value);
				}
			}

			ss.str("");
			ss.clear();

		}
		else if (part_type == "RHS") {
			ss << line;
			ss >> key1;
			

			while (ss >> key2) {
				ss >> value;

				/*if (key2 == row_stage) {
					period_row = "second";
				}*/

				if (std::find(second_stage_rows.begin(), second_stage_rows.end(), key2)
					!= second_stage_rows.end()) {
					period_row = "second";
				}

				if (period_row == "first") {
					RDfile << "    " << "RHS"
						<< "    " << key2 << 
						"     " << value << std::endl;
				}
				else {
					rhs_row.push_back(key2);
					rhs_val.push_back(std::stod(value));
				}
			}

			ss.str("");
			ss.clear();

		}
		else if (part_type == "BOUNDS") {
			ss << line;
			ss >> key1 >> key2 >> key3 >> value;
			ss.str("");
			ss.clear();

			if (key3 == col_stage) {
				period_col = "second";
			}

			if (period_col == "first") {
				RDfile << line << std::endl;
			}
			else {
				bnd_type.push_back(key1);
				bnd_col.push_back(key3);
				bnd_val.push_back(value);
			}

		}
		else {

		}
	}

	RDfile.close();
	corfile.close();

	std::exit(1);
}

RdRealisation::RdRealisation(double proba)
{
	_proba = proba;
	//_rd_elements.clear();
	_keys.clear();
	_values.clear();
	_nbr_lines = 0;
}

RdRealisation::RdRealisation(double proba, std::string const& key1, std::string const& key2, double val)
{
	_proba = proba;
	//_rd_elements.clear();
	_keys.clear();
	_values.clear();

	StrPair paire(key1, key2);
	//_rd_elements[paire] = val;
	_keys.push_back(paire);
	_values.push_back(val);
	_nbr_lines = 1;
}

/*StrPair2Dbl const& RdRealisation::get_elems() const
{
	return _rd_elements;
}*/


void RdRealisation::addElement(std::string const& key1, std::string const& key2, double val)
{
	StrPair paire(key1, key2);
	/*if (_rd_elements.find(paire) == _rd_elements.end()) {
		_rd_elements[paire] = val;
	}
	else {
		std::cout << "ERROR : KEY " << paire.first << "," << paire.second << " ALREADY EXIXSTS IN KEY MAP." << std::endl;
		std::exit(0);
	}*/
	_nbr_lines += 1;
	_keys.push_back(paire);
	_values.push_back(val);
}

int RdRealisation::get_size() const
{
	return _nbr_lines;
}

SMPSData::SMPSData()
{
}

void SMPSData::read_sto_file(std::string const& sto_path)
{
	std::ifstream sto_file(sto_path);
	std::string part_type = "";

	std::string line, key1, key2, val, period, proba;
	std::string keyT = "";
	std::stringstream ss;
	while (getline(sto_file, line)) {
		ss << line;
		if (line[0] != ' ') {
			if (line[0] != '*') {
				ss >> part_type;
			}
		}
		else if (part_type == "INDEP") {
			proba = "";
			ss >> key1 >> key2 >> val >> period >> proba;
			//std::cout << key1 << "  " << key2 << "   " << period << "   " << proba << std::endl;
			if (proba == "") {
				proba = period;
			}

			if (keyT != key1 + " " + key2) {
				keyT = key1 + " " + key2;
				_rd_entries.push_back(RdRealVector());
			}
			
			_rd_entries.back().push_back(RdRealisation(std::stod(proba), key1, key2, std::stod(val)));
			/*keyT = key1 + " " + key2;
			if (_rd_entries.find(keyT) == _rd_entries.end()) {
				_rd_entries[keyT] = RdRealVector();
			}
			_rd_entries[keyT].push_back(RdRealisation(std::stod(proba), key1, key2, std::stod(val)));*/
		}

		else if (part_type == "BLOCKS") {
			std::cout << "BLOCKS NOT CODED" << std::endl;
			std::exit(0);
			if (key1 == "BL") {
				/*ss >> key1 >> key2 >> period >> proba;
				if (_rd_entries.find(keyT) == _rd_entries.end()) {
					_rd_entries[keyT] = RdRealVector();
				}
				_rd_entries[keyT].push_back(RdRealisation(std::stod(proba)));*/
			}
			else {
				/*ss >> key1 >> key2 >> val;
				_rd_entries[keyT][-1].addElement(key1, key2, std::stod(val));*/
			}
		}
		else {
			std::cout << line;
			std::cout << "UNKNOWN PART TYPE IN .STO FILE." << std::endl;
			std::exit(0);
		}

		ss.str("");
		ss.clear();
	}
	sto_file.close();
}

double SMPSData::find_rand_realisation_lines(StrPairVector& keys, DblVector& values,
	IntVector const& real_counter) const
{
	double proba_tot = 1;
	
	for (int k(0); k < real_counter.size(); k++) {
		proba_tot *= get_proba(k, real_counter[k]);
		keys.insert(keys.end(), _rd_entries[k][real_counter[k]]._keys.begin(), _rd_entries[k][real_counter[k]]._keys.end());
		//keys	= _rd_entries[k][real_counter[k]]._keys;
		//values	= _rd_entries[k][real_counter[k]]._values;
		values.insert(values.end(), _rd_entries[k][real_counter[k]]._values.begin(), _rd_entries[k][real_counter[k]]._values.end());
	}
	/*
	StrPair keys;
	for (auto const& kvp : real_counter) {
		proba_tot *= get_proba(kvp.first, kvp.second);
		for (auto const& keyVal : get_lines(kvp.first, kvp.second)) {
			keys = std::make_pair(keyVal.first.first, keyVal.first.second);
			realisation[keys] = keyVal.second;
		}
	}*/

	return proba_tot;
}

double SMPSData::get_proba(int num, int id) const
{
	return _rd_entries[num][id]._proba;
}

void SMPSData::go_to_next_realisation(IntVector& real_counter, BendersOptions const& options, 
	std::mt19937& gen, std::uniform_real_distribution<double>& dis) const
{
	double rd_val;
	double cumul = 0.0;
	int ind = 0;
	if (options.SLAVE_NUMBER == -1) {
		real_counter[ind] += 1;
		while (real_counter[ind] == _rd_entries[ind].size()) {
			real_counter[ind] = 0;
			ind++;
			real_counter[ind] += 1;
		}
	}
	else {
		for (int i(0); i < real_counter.size(); i++) {
			ind = 0;
			cumul = _rd_entries[i][ind]._proba;
			rd_val = dis(gen);
			while (cumul < rd_val) {
				ind += 1;
				cumul += _rd_entries[i][ind]._proba;
			}
			real_counter[i] = ind;
			//std::cout << "  " << real_counter[i];
		}
		//std::cout << std::endl;
	}

}

int SMPSData::nbr_entries() const
{
	return _rd_entries.size();
}
