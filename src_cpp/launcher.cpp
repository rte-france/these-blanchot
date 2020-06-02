#include "launcher.h"
#include "Benders.h"
#include "Timer.h"
#include "DataSMPS.h"

#include "BendersOptions.h"

/*!
*  \brief Get Benders Options from command line
*
*  \param argc : number of elements
*
*  \param argv : elements on command line
*/
BendersOptions build_benders_options(int argc, char** argv) {
	BendersOptions result;
	result.read(argv[1]);
	return result;
}

/*!
*  \brief Build the input from the structure file
*
*	Function to build the map linking each problem name to its variables and their id
*
*  \param root : root of the structure file
*
*  \param summary_name : name of the structure file
*
*  \param coupling_map : empty map to increment
*/
int build_input(BendersOptions const & options, CouplingMap & coupling_map) {
	coupling_map.clear();
	std::ifstream summary(options.get_structure_path(), std::ios::in);
	if (!summary) {
		std::cout << "Cannot open file summary " << options.get_structure_path() << std::endl;
		return 0;
	}
	std::string line;

	while (std::getline(summary, line))
	{
		std::stringstream buffer(line);
		std::string problem_name;
		std::string variable_name;
		int variable_id;
		buffer >> problem_name;
		buffer >> variable_name;
		buffer >> variable_id;
		coupling_map[problem_name][variable_name] = variable_id;
	}
	int n(0);
	if (options.SLAVE_NUMBER >= 0) {
		CouplingMap trimmer;
		for (auto const & problem : coupling_map) {
			if (problem.first == options.MASTER_NAME)
				trimmer.insert(problem);
			else if (n < options.SLAVE_NUMBER){
				trimmer.insert(problem);
				++n;
			}
		}
		coupling_map = trimmer;
	}
	
	summary.close();
	return 0;
}


/*!
*  \brief Execute the Benders algorithm in sequential
*
*  \param options : set of Benders options
*/
void sequential_launch(BendersOptions const & options) {
	Timer timer;

	CouplingMap input;

	std::string col_stage = "";
	std::string row_stage = "";
	StrSet first_stage_vars;
	Str2Int blocks;
	SMPSData smps_data;
	
	if (options.DATA_FORMAT == "SMPS") {
		std::string time_path = options.INPUTROOT + '/' + options.TIMEFILE_NAME;
		std::string cor_path = options.INPUTROOT + '/' + options.CORFILE_NAME;
		std::string sto_path = options.INPUTROOT + '/' + options.STOFILE_NAME;

		analyze_time_file(time_path, col_stage, row_stage);
		generate_base_of_instance(cor_path, options.OUTPUTROOT, first_stage_vars, col_stage, row_stage);
		generate_number_of_realisations(blocks, sto_path);
		smps_data.read_sto_file(sto_path);
		read_struct_SMPS(options, input, blocks);
	}
	else if (options.DATA_FORMAT == "DECOMPOSED") {
		build_input(options, input);
	}
		
	Benders benders(input, options, smps_data);
	benders.run(std::cout);
	benders.free();
	std::cout << "Problem ran in " << timer.elapsed() << " seconds" << std::endl;
}

/*!
*  \brief How to call for the algorithm
*
*  \param argc : number of arguments in command line
*/
void usage(int argc) {
	if (argc < 2) {
		std::cout << "usage is : <exe> <option_file> " << std::endl;
		BendersOptions input;
		input.write_default();
		std::exit(0);
	}
}

