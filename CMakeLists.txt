CMAKE_MINIMUM_REQUIRED(VERSION 2.8)
PROJECT(ANTARESXPANSION_V2)

SET(EXECUTABLE_OUTPUT_PATH	${CMAKE_BINARY_DIR})
SET(   LIBRARY_OUTPUT_PATH	${CMAKE_BINARY_DIR})

SET(USE_MPI TRUE)
SET(XPRESS	TRUE)
SET(CPLEX	TRUE)

IF( USE_MPI )
	SET(CMAKE_CXX_COMPILER mpicxx)
ENDIF( USE_MPI )
MESSAGE("USE_MPI is ${USE_MPI}")

IF( WIN32 )
#	SET( CMAKE_CONFIGURATION_TYPES "Debug;Release;" CACHE STRING "limited configs" FORCE)
#	SET( CMAKE_CXX_FLAGS_DEBUG          "/D_DEBUG /MTd /Zi /Ob0 /Od /RTC1" )
#	SET( CMAKE_CXX_FLAGS_RELEASE        "/MT /O2 /Ob2 /D NDEBUG")
ELSE()
	SET ( CMAKE_BUILD_TYPE "RELEASE"     )
#	SET ( CMAKE_BUILD_TYPE "DEBUG"     )
#	IF( WITH_DEBUG )
#		SET ( CMAKE_BUILD_TYPE "DEBUG"     )
#	ELSE( WITH_DEBUG )GENDIOR
#		SET ( CMAKE_BUILD_TYPE "RELEASE"     )
#	ENDIF( WITH_DEBUG )
	SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -DNDEBUG")
	SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -pedantic -fmessage-length=0 -fPIC" )
	SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wno-conversion -Wno-sign-compare")
	SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-strict-aliasing -Wno-unused-parameter")
ENDIF( WIN32 )

MESSAGE("CMAKE_BINARY_DIR : ${CMAKE_BINARY_DIR}")
MESSAGE("CMAKE_SOURCE_DIR : ${CMAKE_SOURCE_DIR}")
MESSAGE("CMAKE_C_COMPILER_VERSION : ${CMAKE_C_COMPILER_VERSION}")
MESSAGE("CMAKE_CXX_COMPILER_VERSION : ${CMAKE_CXX_COMPILER_VERSION}")

MESSAGE("XPRESSDIR is $ENV{XPRESSDIR}")
MESSAGE("BOOSTDIR  is $ENV{BOOSTDIR}")
MESSAGE("CPLEXDIR  is $ENV{CPLEX_STUDIO_DIR129}")


INCLUDE_DIRECTORIES(./src_cpp)
IF(XPRESS)
	INCLUDE_DIRECTORIES( $ENV{XPRESSDIR}/include )
ENDIF(XPRESS)
IF(CPLEX)
	INCLUDE_DIRECTORIES( $ENV{CPLEX_STUDIO_DIR129}/cplex/include/ilcplex ) 
ENDIF(CPLEX)


IF( USE_MPI )
	INCLUDE_DIRECTORIES( $ENV{BOOSTDIR} ./src_mpi )
	LINK_DIRECTORIES( $ENV{BOOSTDIR}/stage/lib )
	
	IF(WIN32)
		INCLUDE_DIRECTORIES(	$ENV{MSMPI_INC} $ENV{MSMPI_INC}/x64 )
		LINK_DIRECTORIES(		$ENV{MSMPI_LIB64} )
	ENDIF(WIN32)

ENDIF( USE_MPI )

# Ajout du dossier de XPRESS mais pas cplex
# Pour CPLEX, chemin complet donne a l'executable car dependant de la machine
IF(XPRESS)
	LINK_DIRECTORIES( $ENV{XPRESSDIR}/lib )
ENDIF(XPRESS)

FILE(
	GLOB
	SEQUENTIAL_FILES
	./src_cpp/*.cc
	./src_cpp/*.cpp
	./src_cpp/*.h  
	./src_cpp/*.hxx
)

####################################################################
# LIBRARY : sequentialcore / mpicore
####################################################################
ADD_LIBRARY(sequentialcore STATIC ${SEQUENTIAL_FILES} )
IF( USE_MPI )
	FILE(
		GLOB
		MPI_FILES
		./src_mpi/*.cc
		./src_mpi/*.cpp
		./src_mpi/*.h  
		./src_mpi/*.hxx
	)
	ADD_LIBRARY(mpicore STATIC ${MPI_FILES} )
ENDIF( USE_MPI )

####################################################################
# EXECUTABLE : merge_mps
####################################################################
ADD_EXECUTABLE(merge_mps ./exe_cpp/merge_mps.cpp)
TARGET_LINK_LIBRARIES(merge_mps sequentialcore)
IF(XPRESS)
	TARGET_LINK_LIBRARIES(merge_mps xprs)
ENDIF(XPRESS)
IF(CPLEX)
	TARGET_LINK_LIBRARIES(merge_mps 
		debug		$ENV{CPLEX_STUDIO_DIR129}/cplex/lib/x64_windows_vs2017/stat_mdd/cplex1290.lib
		optimized	$ENV{CPLEX_STUDIO_DIR129}/cplex/lib/x64_windows_vs2017/stat_mda/cplex1290.lib
	)
ENDIF(CPLEX)

####################################################################
# EXECUTABLE : bendersmpi
####################################################################
IF( USE_MPI )
	ADD_EXECUTABLE(bendersmpi  exe_mpi/main.cpp)
	TARGET_LINK_LIBRARIES(bendersmpi sequentialcore mpicore)
	
	IF(XPRESS)
		TARGET_LINK_LIBRARIES(bendersmpi xprs)
	ENDIF(XPRESS)

	IF(WIN32)
		TARGET_LINK_LIBRARIES(bendersmpi msmpi 
			debug		libboost_serialization-vc142-mt-gd-x64-1_72 
			optimized	libboost_serialization-vc142-mt-x64-1_72
		)
		IF(CPLEX)
			TARGET_LINK_LIBRARIES(bendersmpi 
				debug		$ENV{CPLEX_STUDIO_DIR129}/cplex/lib/x64_windows_vs2017/stat_mdd/cplex1290.lib
				optimized	$ENV{CPLEX_STUDIO_DIR129}/cplex/lib/x64_windows_vs2017/stat_mda/cplex1290.lib
			)
		ENDIF(CPLEX)

	ELSE(WIN32)
		TARGET_LINK_LIBRARIES(bendersmpi boost_serialization boost_mpi-mt.so)
		# /!\ CPLEX pas ajoute pour linux

	ENDIF(WIN32)

####################################################################
# EXECUTABLE : benderssequential
####################################################################
ELSE( USE_MPI )
	ADD_EXECUTABLE(benderssequential exe_cpp/main.cpp)
	TARGET_LINK_LIBRARIES(benderssequential sequentialcore )
	IF(XPRESS)
		TARGET_LINK_LIBRARIES(benderssequential xprs)
	ENDIF(XPRESS)
	
	IF(CPLEX)
		IF(WIN32)
			TARGET_LINK_LIBRARIES(benderssequential
				debug		$ENV{CPLEX_STUDIO_DIR129}/cplex/lib/x64_windows_vs2017/stat_mdd/cplex1290.lib
				optimized	$ENV{CPLEX_STUDIO_DIR129}/cplex/lib/x64_windows_vs2017/stat_mda/cplex1290.lib
			)
		ELSE(WIN32)
			# /!\ CPLEX pas ajoute pour linux

		ENDIF(WIN32)
	ENDIF(CPLEX)
ENDIF( USE_MPI )

####################################################################
# EXECUTABLE : test_benders
####################################################################
ADD_EXECUTABLE(test_benders ./src_test/test_benders.cpp)
TARGET_LINK_LIBRARIES(test_benders sequentialcore)
IF(USE_MPI)
	TARGET_LINK_LIBRARIES(test_benders mpicore)
	IF(WIN32)
		TARGET_LINK_LIBRARIES(test_benders msmpi 
			debug		libboost_serialization-vc142-mt-gd-x64-1_72 
			optimized	libboost_serialization-vc142-mt-x64-1_72
		)
	ELSE(WIN32)
		
	ENDIF(WIN32)
ENDIF(USE_MPI)
IF(XPRESS)
	TARGET_LINK_LIBRARIES(test_benders xprs)
ENDIF(XPRES)
IF(CPLEX)
	IF(WIN32)
		TARGET_LINK_LIBRARIES(test_benders
			debug		$ENV{CPLEX_STUDIO_DIR129}/cplex/lib/x64_windows_vs2017/stat_mdd/cplex1290.lib
			optimized	$ENV{CPLEX_STUDIO_DIR129}/cplex/lib/x64_windows_vs2017/stat_mda/cplex1290.lib
		)
		
	ELSE(WIN32)
		# /!\ CPLEX pas ajoute pour linux
	ENDIF(WIN32)
ENDIF(CPLEX)