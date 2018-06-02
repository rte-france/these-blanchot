//Determine the degree of detail of the output, from 1 to 3
BENDERS_OPTIONS_MACRO(LOG_LEVEL, int, 1)

//Maximum number of iterations accepted
BENDERS_OPTIONS_MACRO(MAX_ITERATIONS, int, -1)

//Level of precision accepted
BENDERS_OPTIONS_MACRO(GAP, double, 1e-6)

//True if cuts need to be aggregated, false otherwise
BENDERS_OPTIONS_MACRO(AGGREGATION, bool, false)

//Path to the folder where output files should be printed
BENDERS_OPTIONS_MACRO(ROOTPATH, std::string, ".")

//True if a trace should be built, false otherwise
BENDERS_OPTIONS_MACRO(TRACE, bool, true)

//True if similar cuts should be deleted, false otherwise
BENDERS_OPTIONS_MACRO(DELETE_CUT, bool, true)

//Either "COMMAND" to print the log in the command prompt, or a file name to print in
BENDERS_OPTIONS_MACRO(LOG_OUTPUT, std::string, "COMMAND")

//UNIFORM (1/n), NULL (1), or a txt file linking each slave to its weight
BENDERS_OPTIONS_MACRO(SLAVE_WEIGHT, std::string, "ONES")

//Name of the master problem file, if different from 'master'
BENDERS_OPTIONS_MACRO(MASTER_NAME, std::string, "master")

//Number of slaves to use to solve the problem
BENDERS_OPTIONS_MACRO(SLAVE_NUMBER, int, -1)