# ===========================================================================
# CMake configuration
# ===========================================================================

# ===========================================================================
# Targets
# ===========================================================================

# ---------------------------------------------------------------------------
# Merge MPS Exe
# ---------------------------------------------------------------------------

set(SRCS
	${CMAKE_CURRENT_SOURCE_DIR}/test.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/functions_test.cpp
)
    
set(HEADERS
	${CMAKE_CURRENT_SOURCE_DIR}/functions_test.h
)

add_executable (test_solver
	${CMAKE_CURRENT_SOURCE_DIR}/test.cpp	
    ${CMAKE_CURRENT_SOURCE_DIR}/functions_test.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/functions_test.h
)

target_link_libraries (test_solver
	PUBLIC
		solvers
)
