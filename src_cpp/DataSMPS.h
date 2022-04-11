#pragma once
#include "common.h"
#include "BendersFunctions.h"

class RdRealisation {
public:
	double _proba;
	int _nbr_lines;
	//StrPair2Dbl _rd_elements;
	StrPairVector _keys;
	DblVector _values;

public:
	RdRealisation(double proba);
	RdRealisation(double proba, std::string const& key1, std::string const& key2, double val);
	RdRealisation(double proba, StrPairVector const& keys, DblVector const& val);

	//StrPair2Dbl const& get_elems() const;

public:
	void addElement(std::string const& key1, std::string const& key2, double val);
	int get_size() const;
};


typedef std::vector<RdRealisation> RdRealVector;
typedef std::vector<RdRealVector> RandEntries;
typedef std::map<std::string, RdRealVector> Str2RdVector;

class SMPSData {
public:
	RandEntries _rd_entries;

public:
	SMPSData();
	void read_sto_file(std::string const& sto_path);
	double find_rand_realisation_lines(StrPairVector& keys, DblVector& values, IntVector const& real_counter) const;

	double get_proba(int num, int id) const;
	void go_to_next_realisation(IntVector& real_counter, BendersOptions const& options, 
		std::mt19937& gen, std::uniform_real_distribution<double>& dis) const;
	int nbr_entries() const;
};


void analyze_time_file(std::string const& time_path, std::string& col_stage, 
	std::string& row_stage);

void generate_base_of_instance(std::string const& cor_path, std::string const& inst_folder, 
	StrSet first_stage_vars, std::string const& col_stage, std::string const& row_stage);

void analyze_row_line(std::string const& line, std::vector<StrSet>& rows, 
	std::string const& row_stage, std::ofstream& master_file, std::ofstream& slave, 
	int& period_row);

void analysze_col_line(std::string const& line, std::string const& col_stage, 
	std::ofstream& master_file, std::ofstream& slave_file, std::ofstream& struct_file, 
	std::vector<StrSet>& rows, StrSet& written_vars, StrSet& first_stage_vars, 
	int& current_period);

void write_mps_line(int period, std::ofstream& master_file, std::ofstream& slave_file, 
	std::string const& colname, std::string const& rowname, std::string const& val, 
	std::vector<StrSet> const& rows);

void write_struct_master(std::string const& colname, std::ofstream& struct_file, 
	StrSet& written_vars, StrSet& first_stage_vars);

void write_struct_slave(std::string const& slave_path, StrSet const& first_stage_vars, 
	std::ofstream& struct_file);

void analyze_rhs_line(std::string const& line, std::ofstream& master_file, 
	std::ofstream& slave_file, std::vector<StrSet> const& rows);

void analyze_bounds_line(std::string const& line, std::ofstream& master_file,
	std::ofstream& slave_file, std::vector<StrSet> const& rows);

void generate_number_of_realisations(Str2Int& blocks, std::string const& sto_path);

void read_struct_SMPS(BendersOptions const& options, CouplingMap& coupling_map, Str2Int blocks);

void go_to_next_realisation(Str2Int const& blocks, Str2Int& real_counter);

double find_rand_realisation_lines(StrPair2Dbl& realisation, std::string const& sto_path,
	Str2Int const& real_counter);

void read_master_cstr(BendersData & data, BendersOptions const& options);

void write_deterministic_reformulation(BendersOptions const& options, std::string const& col_stage, 
	std::string const& row_stage, std::string const& corpath, SMPSData & smps_data);