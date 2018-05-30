
BENDERS_OPTIONS_MACRO(LOG_LEVEL, int, 1)
BENDERS_OPTIONS_MACRO(MAX_ITERATIONS, int, -1)
BENDERS_OPTIONS_MACRO(GAP, double, 1e-6)
BENDERS_OPTIONS_MACRO(AGGREGATION, bool, false)
BENDERS_OPTIONS_MACRO(ROOTPATH, std::string, "D:\\repos\\stage-enzo\\build\\Release")
BENDERS_OPTIONS_MACRO(TRACE, bool, true)
BENDERS_OPTIONS_MACRO(DELETE_CUT, bool, true)

//Either "COMMAND" to print the log in the command prompt, or a file name to print in
BENDERS_OPTIONS_MACRO(LOG_OUTPUT, std::string, "COMMAND")

//UNIFORM (1/n), NULL (1), or a txt file linking each slave to its weight
BENDERS_OPTIONS_MACRO(SLAVE_WEIGHT, std::string, "ONES")

BENDERS_OPTIONS_MACRO(MASTER_NAME, std::string, "master")