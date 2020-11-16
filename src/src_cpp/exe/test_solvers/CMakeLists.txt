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
	${CMAKE_CURRENT_SOURCE_DIR}/functions_test.h
	${CMAKE_CURRENT_SOURCE_DIR}/catch2.hpp

)

target_link_libraries (test_solver
	PUBLIC
		solvers
)
