CMAKE_MINIMUM_REQUIRED(VERSION 2.8)
PROJECT(STAGE_ENZO)


MESSAGE("CMAKE_BINARY_DIR : ${CMAKE_BINARY_DIR}")
MESSAGE("CMAKE_SOURCE_DIR : ${CMAKE_SOURCE_DIR}")

SET(EXECUTABLE_OUTPUT_PATH	${CMAKE_BINARY_DIR})
SET(   LIBRARY_OUTPUT_PATH	${CMAKE_BINARY_DIR})

set(CMAKE_CXX_COMPILER mpicxx)

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
	SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x -pedantic -fmessage-length=0 -fPIC" )
	SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wno-conversion -Wno-sign-compare")
	SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-strict-aliasing -Wno-unused-parameter")
ENDIF( WIN32 )

INCLUDE_DIRECTORIES(	
	.
	../src_cpp
	$ENV{XPRESSDIR}/include
	D:/software/boost_1_67_0
	$ENV{MSMPI_INC}
	$ENV{MSMPI_INC}/x64
	)
LINK_DIRECTORIES(
	$ENV{XPRESSDIR}/lib
	D:/software/boost_1_67_0/stage/lib
	$ENV{MSMPI_LIB64}
	)


FILE(
	GLOB
	STAGE_ENZO_FILES
	./*.cc
	./*.cpp
	./*.h  
	./*.hxx
	../src_cpp/*.cc
	../src_cpp/*.cpp
	../src_cpp/*.h  
	../src_cpp/*.hxx
	)
	
ADD_EXECUTABLE(benders ${STAGE_ENZO_FILES} )
TARGET_LINK_LIBRARIES(benders xprs msmpi libboost_serialization-vc141-mt-x64-1_67)