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

//True if similar cuts should be deleted, false otherwise 
BENDERS_OPTIONS_MACRO(DELETE_CUT, bool, false)

//Either "COMMAND" to print the log in the command prompt, or a file name to print in 
BENDERS_OPTIONS_MACRO(LOG_OUTPUT, std::string, "COMMAND")

//UNIFORM (1/n), CONSTANT (to set in SLAVE_WEIGHT_VALUE), or a txt file linking each slave to its weight 
BENDERS_OPTIONS_MACRO(SLAVE_WEIGHT, std::string, "UNIFORM")

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

//Number of slaves to select for random aggregation, set to 0 if no random aggregation needed
BENDERS_OPTIONS_MACRO(RAND_AGGREGATION, int, 0)

//Method use to solve the master problem (either SIMPLEX, BARRIER or BARRIER_WO_CROSSOVER)
BENDERS_OPTIONS_MACRO(MASTER_METHOD, std::string, "SIMPLEX")

//Set to 1 if Xpress output is wanted for the master, 2 for slaves, 3 for both, 0 otherwise 
BENDERS_OPTIONS_MACRO(XPRESS_TRACE, int, 0)

//Choice of the solver to use (XPRESS only for now)
BENDERS_OPTIONS_MACRO(SOLVER, std::string, "CPLEX")

//Bool to say if non optimal problem should be written in a file before exit
BENDERS_OPTIONS_MACRO(WRITE_ERRORED_PROB,bool, true)

// Algorithm used to solve the problem (BASE, IN-OUT, ENHANCED_MULTICUT)
BENDERS_OPTIONS_MACRO(ALGORITHM, std::string, "BASE")

// Method to sample scenarios if ALGORITHM == ENHANCED_MULTICUT (ORDERED -- TO ADD : RANDOM, ORDERED_RD, MAX_GAP ?)
BENDERS_OPTIONS_MACRO(SORTING_METHOD, std::string, "ORDERED")

// Number of slaves solved at each iteration on each machine if ALGORITHM == ENHANCED_MULTICUT
BENDERS_OPTIONS_MACRO(BATCH_SIZE, int, 1)

// Time limit for Benders decomposition ( -1 for no limit )
BENDERS_OPTIONS_MACRO(TIME_LIMIT, double, -1)

// LB set on epigraph variables
BENDERS_OPTIONS_MACRO(THETA_LB, double, -1e10)

// Value of solver presolve for master problem
BENDERS_OPTIONS_MACRO(MASTER_PRESOLVE, int, 1)

// Value of scaling for master problem
BENDERS_OPTIONS_MACRO(MASTER_SCALING, int, 0)

// Value of solver presolve for slaves
BENDERS_OPTIONS_MACRO(SLAVE_PRESOLVE, int, 1)

// Value of scaling for slaces
BENDERS_OPTIONS_MACRO(SLAVE_SCALING, int, 0)

// Iterations when printing a log line, only for ENHANCED_MULTICUT
BENDERS_OPTIONS_MACRO(LOG_NUMBER_ITE, int, 1)

// Step size of enhanced multicut algorithm (x(k) = x(k-1) + step_size* ( xMaster - x(k-1) )
BENDERS_OPTIONS_MACRO(STEP_SIZE, float, 1.0)

// Bool to say if the optimal solution appear in the log
BENDERS_OPTIONS_MACRO(PRINT_SOLUTION, bool, 1)

// Bool to say if the optimal solution appear in the log
BENDERS_OPTIONS_MACRO(CUT_MASTER_TOL, double, -1e-1)

// Bool to say if the optimal solution appear in the log
BENDERS_OPTIONS_MACRO(ALPHA_STRAT, std::string, "DYNAMIQUE")

// Bool to say if the options appear in the log
BENDERS_OPTIONS_MACRO(PRINT_OPTIONS, bool, 1)

// memory parameter (0 without memory, beta in [0;1[)
BENDERS_OPTIONS_MACRO(BETA, double, 0.0)

// memory parameter (0 without memory, beta in [0;1[)
BENDERS_OPTIONS_MACRO(MEMORY_TYPE, std::string, "WITHOUT")

// DECOMPOSED or SMPS
BENDERS_OPTIONS_MACRO(DATA_FORMAT, std::string, "DECOMPOSED")

// Name of .tim file if data format is SMPS
BENDERS_OPTIONS_MACRO(TIMEFILE_NAME, std::string, "instance.tim")

// Name of .cor file if data format is SMPS
BENDERS_OPTIONS_MACRO(CORFILE_NAME, std::string, "instance.cor")

// Name of .sto file if data format is SMPS
BENDERS_OPTIONS_MACRO(STOFILE_NAME, std::string, "instance.sto")

// Seed for std::rand
BENDERS_OPTIONS_MACRO(SEED, int, -1)

// Set to 1 to activate numerical emphasis of solver, better precision but a little bit slower
BENDERS_OPTIONS_MACRO(NUMERICAL_EMPHASIS, bool, 0)
