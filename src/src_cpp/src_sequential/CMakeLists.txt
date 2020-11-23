# ===========================================================================
# CMake configuration
# ===========================================================================

# ===========================================================================
# Targets
# ===========================================================================

add_library (sequentialcore STATIC 
	${CMAKE_CURRENT_SOURCE_DIR}/Timer.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/common.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/BendersOptions.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/Worker.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/WorkerTrace.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/WorkerSlave.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/SlaveCut.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/SimplexBasis.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/WorkerMaster.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/BendersFunctions.cpp

	${CMAKE_CURRENT_SOURCE_DIR}/Timer.h
	${CMAKE_CURRENT_SOURCE_DIR}/common.h
	${CMAKE_CURRENT_SOURCE_DIR}/BendersOptions.h
	${CMAKE_CURRENT_SOURCE_DIR}/BendersOptions.hxx
	${CMAKE_CURRENT_SOURCE_DIR}/Worker.h
	${CMAKE_CURRENT_SOURCE_DIR}/WorkerTrace.h
	${CMAKE_CURRENT_SOURCE_DIR}/WorkerSlave.h
	${CMAKE_CURRENT_SOURCE_DIR}/SlaveCut.h
	${CMAKE_CURRENT_SOURCE_DIR}/SimplexBasis.h
	${CMAKE_CURRENT_SOURCE_DIR}/WorkerMaster.h
	${CMAKE_CURRENT_SOURCE_DIR}/BendersFunctions.h
)

target_include_directories (sequentialcore
	PUBLIC
		${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries (sequentialcore
	PUBLIC
		solvers
)

add_library (${PROJECT_NAME}::sequentialcore ALIAS sequentialcore)
