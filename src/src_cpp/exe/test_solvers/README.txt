Date: 16.11.2020

#=============================================================================================
#       Launching command
#=============================================================================================
./PATH_TO_EXECUTABLE/test_solver(.exe) -f path_to_file/test_file.txt


#=============================================================================================
#       test_file format and description
#=============================================================================================
Brief: The test_file describes the solver to use, the path to an instance file, and the results to give to the checker

Format:
TEST_NAME               Name of the test
SOLVER                  SOLVER_NAME (XPRESS || CPLEX || ALL)
INSTANCE                path_to_mps_file/file.mps
VARIABLES               number of variables
INTEGER_VARIABLES       number of integer variables
CONSTRAINTS             number of constraints
NON_ZEROS_ELEMENTS      number of non zero elements in the matrix excluding the objectif
OBJECTIVE               objective elements, including the zeros separated by spaces
MATVAL                  vector matval of sparse matrix representation, separated by spaces
MIND                    vector of indices of variables in sparse matrix representation separated by spaces
MSART                   vector of offsets in matval and mind of elements by rows of sparse matrix representation separated by spaces of size CONSTRAINTS + 1
RHS                     vector of right-hand sides separated by spaces, including 0-rhs
ROW_TYPES               vector of row_types (characters) separated by spaces ('L' : lower, 'G': grater, 'E': equal, 'R': range)
COL_TYPES               vector of column types, separated by spaces ('C': continuous, 'B': binary, 'I': integer)
LOWER_BOUNDS            vector of lower bounds of variables separated by spaces
UPPER_BOUNDS            vector of upper bounds of variables separated by spaces
OPITMAL_VALUE           optimal value of problem described in file.mps
LP_VALUE                optimal value of continuous relaxation of the problem described in file.mps
OPTIMAL_SOLUTON         vector of optimal value of variables in a solution of the problem described in file.mps including zeros (many solutions can exist, this is only for easy problems where there is unicity)