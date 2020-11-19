# ===========================================================================
# CMake configuration
# ===========================================================================

# ===========================================================================
# Targets
# ===========================================================================

# ---------------------------------------------------------------------------
# Merge MPS Exe
# ---------------------------------------------------------------------------

add_executable (test_benders
	${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/timerTest.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/catch2.hpp
)

target_link_libraries (test_benders
	PUBLIC
		solvers
		${PROJECT_NAME}::sequentialcore
)
