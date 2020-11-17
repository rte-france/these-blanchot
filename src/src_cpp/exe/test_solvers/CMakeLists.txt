# ===========================================================================
# CMake configuration
# ===========================================================================

# ===========================================================================
# Targets
# ===========================================================================

# ---------------------------------------------------------------------------
# Merge MPS Exe
# ---------------------------------------------------------------------------

add_executable (test_solver
	${CMAKE_CURRENT_SOURCE_DIR}/test.cpp	
	${CMAKE_CURRENT_SOURCE_DIR}/functions_test.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/test_reading_problem.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/test_modifying_problem.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/test_solving_problem.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/define_datas.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/functions_test.h
	${CMAKE_CURRENT_SOURCE_DIR}/catch2.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/define_datas.hpp
)

target_link_libraries (test_solver
	PUBLIC
		solvers
)
