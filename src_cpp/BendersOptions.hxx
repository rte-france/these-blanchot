//Determine the degree of detail of the output, from 1 to 3 
BENDERS_OPTIONS_MACRO(LOG_LEVEL, int, 3)

//Maximum number of iterations accepted 
BENDERS_OPTIONS_MACRO(MAX_ITERATIONS, int, -1)

//Level of precision accepted 
BENDERS_OPTIONS_MACRO(GAP, double, 1e-6)

//True if cuts need to be aggregated, false otherwise 
BENDERS_OPTIONS_MACRO(AGGREGATION, bool, false)

//Path to the folder where output files should be printed 
BENDERS_OPTIONS_MACRO(OUTPUTROOT, std::string, ".")

//True if a trace should be built, false otherwise 
BENDERS_OPTIONS_MACRO(TRACE, bool, false)

//True if similar cuts should be deleted, false otherwise 
BENDERS_OPTIONS_MACRO(DELETE_CUT, bool, false)

//Either "COMMAND" to print the log in the command prompt, or a file name to print in 
BENDERS_OPTIONS_MACRO(LOG_OUTPUT, std::string, "COMMAND")

//UNIFORM (1/n), CONSTANT (to set in SLAVE_WEIGHT_VALUE), or a txt file linking each slave to its weight 
BENDERS_OPTIONS_MACRO(SLAVE_WEIGHT, std::string, "CONSTANT")

//If SLAVE_WEIGHT is CONSTANT, set here the divisor required 
BENDERS_OPTIONS_MACRO(SLAVE_WEIGHT_VALUE, double, 1)

//Name of the master problem file, if different from 'master' 
BENDERS_OPTIONS_MACRO(MASTER_NAME, std::string, "master")

//Number of slaves to use to solve the problem 
BENDERS_OPTIONS_MACRO(SLAVE_NUMBER, int, -1)

//Number of slaves to use to solve the problem 
BENDERS_OPTIONS_MACRO(STRUCTURE_FILE, std::string, "structure.txt")

//Path to the folder where input files are stored 
BENDERS_OPTIONS_MACRO(INPUTROOT, std::string, ".")

//True if simplex bases need to be stored, false otherwise 
BENDERS_OPTIONS_MACRO(BASIS, bool, false)

//True if a statement of active cuts need to be done, false otherwise 
BENDERS_OPTIONS_MACRO(ACTIVECUTS, bool, false)

//Number of iterations before aggregation of all previous cuts, set to 0 if no aggregation needed 
BENDERS_OPTIONS_MACRO(THRESHOLD_AGGREGATION, int, 0)

//Number of iterations before aggregation of all previous cuts by iteration, set to 0 if no aggregation needed 
BENDERS_OPTIONS_MACRO(THRESHOLD_ITERATION, int, 0)

//Number of slaves to select for random aggregation, set to 0 if no random aggregation needed
BENDERS_OPTIONS_MACRO(RAND_AGGREGATION, int, 0)

//Seed to use random process
BENDERS_OPTIONS_MACRO(RAND_SEED, int, -1)

//Method use to solve the master problem (either SIMPLEX, BARRIER or BARRIER_WO_CROSSOVER) 
BENDERS_OPTIONS_MACRO(MASTER_METHOD, std::string, "SIMPLEX")

//Name of the csv output file 
BENDERS_OPTIONS_MACRO(CSV_NAME, std::string, "benders_output_trace")

//True if alpha needs to be bounded by best upper bound, false otherwise 
BENDERS_OPTIONS_MACRO(BOUND_ALPHA, bool, true)

//Set to 1 if Xpress output is wanted for the master, 2 for slaves, 3 for both, 0 otherwise 
BENDERS_OPTIONS_MACRO(XPRESS_TRACE, int, 0)

//Intial value of the in-out convexity parameter
BENDERS_OPTIONS_MACRO(ETA_IN_OUT, float, 1)

//Defines the number of iterations at which we use the LP solution to derive a cut
BENDERS_OPTIONS_MACRO(TRICK_FISCHETTI, int, 0)

//True if the in-out parameter is updated dynamically, false if constant
BENDERS_OPTIONS_MACRO(DYNAMIC_STABILIZATION, bool, true)

//Choice of the solver used to solve linear programs
BENDERS_OPTIONS_MACRO(SOLVER, std::string, "XPRESS")

//Choice of the algorithm to solve Benders - BASE, INOUT, SAMPLING 
BENDERS_OPTIONS_MACRO(ALGORITHM, std::string, "INOUT")

//Choice of the sampling strategy
BENDERS_OPTIONS_MACRO(SAMPLING_STRATEGY, std::string, "RANDOM")

//Lower bound on epigraph variables
BENDERS_OPTIONS_MACRO(THETA_LB, double, -1e6)

//Choice of initialisation type
BENDERS_OPTIONS_MACRO(SAMPLING_INIT, std::string, "FULL")


