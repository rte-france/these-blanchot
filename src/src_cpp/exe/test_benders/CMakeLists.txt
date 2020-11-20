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
	${CMAKE_CURRENT_SOURCE_DIR}/commonTest.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/bendersOptionsTest.cpp

	${CMAKE_CURRENT_SOURCE_DIR}/workerTest.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/slaveCutTest.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/simplexBasisTest.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/workerTraceTest.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/workerSlaveTest.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/workerMasterTest.cpp

	${CMAKE_CURRENT_SOURCE_DIR}/define_datas.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/define_datas.hpp

	${CMAKE_CURRENT_SOURCE_DIR}/catch2.hpp
)

target_link_libraries (test_benders
	PUBLIC
		solvers
		${PROJECT_NAME}::sequentialcore
)
