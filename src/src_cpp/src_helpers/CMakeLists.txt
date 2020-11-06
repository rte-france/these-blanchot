# ===========================================================================
# CMake configuration
# ===========================================================================

# ===========================================================================
# External requirements
# ===========================================================================

# ===========================================================================
# Targets
# ===========================================================================

# --------------------------------------------------------------------------
# unit tests
# --------------------------------------------------------------------------

add_library (helpers STATIC
	${CMAKE_CURRENT_SOURCE_DIR}/ortools_utils.cc
	)

message(${CMAKE_CURRENT_SOURCE_DIR})
message($ENV{CPLEX_STUDIO_DIR129})

target_include_directories (helpers
	PUBLIC
		${CMAKE_CURRENT_SOURCE_DIR}
		$ENV{CPLEX_STUDIO_DIR129}/cplex/include/ilcplex
)

target_link_libraries (helpers 
	PUBLIC
		debug		$ENV{CPLEX_STUDIO_DIR129}/cplex/lib/x64_windows_vs2017/stat_mdd/cplex1290.lib
		optimized	$ENV{CPLEX_STUDIO_DIR129}/cplex/lib/x64_windows_vs2017/stat_mda/cplex1290.lib
)
