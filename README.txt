Benders Solver of Antares XPansion V2
================================================

This is an optimizer solver for stochastic Two-stage programs.
It implements a Benders Decomposition. The problem data needs to be given in a decomposed way :
	- a master problem in MPS format
	- a MPS file for each subproblem, with the master variables as actual variables in it
	- a structure file, linking the column ID of each master variable in each problem (master and all the subproblems)


Requirements
================================================

* Need of an otpimisation solver already linked with the Benders implementation. 
	Available solvers :
		- FICO XPRESS

	To do :
		- IBM ILOG CPLEX
		- COIN OR

* CMake 2.8 or later


Optionnal requirements
================================================

* Boost : to use parallel version of the code


Installation
================================================

1. Set the value of USE_MPI :
	- TRUE if boost is available
	- IF TRUE : add boost library to CMakeListe

2. Set the available solvers and link libraries



Content
================================================
CMakeListe.txt		: to compile the code
src_test/			: unit_test code and datas, with the Catch2 header file (https://github.com/catchorg/Catch2)
src_mpi/			: MPI implemetation of Benders (only available if boost is installed)
src_cpp/			: Main implementation of Benders Decomposition
exe_mpi/			: executable of Benders with OpenMP parallelisation
exe_cpp/			: executables of Benders Decomposition and Deterministic Reformulation solver


Data format
================================================
A problem has to be created as a folder with :

	- A master problem to MPS format (default name : master.mps)
	- Every subproblem in a MPS format with the master variables
	- a structure file to link the master variables between the different problems
	- (OPTIONNAL) an option file

	--> An exemple is given in src_test/mini_instance_LP/

	OPTIONS DESCRIPTION :

	LOG_LEVEL			int 		DEFAULT 3 				level of detail of log output (from 1 to 3) 					
	MAX_ITERATIONS		int			DEFAULT -1 				Number of iterations limit of Benders (-1 : no limit)	
	SLAVE_NUMBER 		int 		DEFAULT -1 				Number of slaves to use to solve the problem (-1 for all slaves present in the structure file)
	RAND_AGGREGATION 	int 		DEFAULT 0 				Number of slaves to select for random aggregation, set to 0 if no random aggregation needed
	XPRESS_TRACE 		int 		DEFAULT 0 				Set to 1 if Xpress output is wanted for the master, 2 for slaves, 3 for both, 0 otherwise

	GAP					float		DEFAULT 1e-6 			Optimiality gap of Benders 
	SLAVE_WEIGHT_VALUE  float 		DEFAULT 1				If SLAVE_WEIGHT is CONSTANT, set here the divisor required 

	AGGREGATION 		bool 		DEFAULT false 			Use classic (TRUE) or Multicut (FALSE) Benders 					
	DELETE_CUT			bool 		DEFAULT false 			Check if a cut already exist to add only new cuts
	ACTIVECUTS 			bool 		DEFAULT false 			True if a statement of active cuts need to be done, false otherwise
	WRITE_ERRORED_PROB  bool 		DEFAULT true 			Bool to say if non optimal problem (infeasible, unbounded, ...) should be written in a file before exit

	LOG_OUTPUT			string  	DEFAULT COMMAND 		Name of the file in which to print the log (COMMAND for terminal)
	SLAVE_WEIGHT 		string 		DEFAULT CONSTANT 		UNIFORM (1/n), CONSTANT (to set in SLAVE_WEIGHT_VALUE), or a txt file linking each slave to its weight 	
	MASTER_NAME 		string 		DEFAULT master 			Name of the master problem file, if different from 'master' 	
	STRUCTURE_FILE 		string 		DEFAULT structure.txt 	Name of the structure file
	INPUTROOT 			string 		DEFAULT . 				Path to the folder where input files are stored 	
	MASTER_METHOD 		string 		DEFAULT SIMPLEX 		Method use to solve the master problem (either SIMPLEX, BARRIER or BARRIER_WO_CROSSOVER) 
	SOLVER 				string 		DEFAULT XPRESS 			Choice of the solver to use (XPRESS only for now)
	ALGORITHM 			string 		DEFAULT IN-OUT 			Algorithm to solve Benders Decomposition (BASE, IN-OUT)


Utilistion
================================================

The solver can be launch from a terminal. It has to be launch from the folder set by the option INPUTROOT 
4 cases of possibles :

1. Windows
	a. USE_MPI = FALSE
		path_to_executable/benderssequential.exe .<OPTIONNAL:options_file>

	b. USE_MPI = TRUE
		mpiexec -n NUMBER_OF_CORES bendersmpi.exe .<OPTIONNAL:options_file>

2. Unix
	a. USE_MPI = FALSE
		path_to_executable/benderssequential .<OPTIONNAL:options_file>

	b. USE_MPI = TRUE
		mpiexec -n NUMBER_OF_CORES bendersmpi .<OPTIONNAL:options_file>

	