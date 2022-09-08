cmake_minimum_required (VERSION 2.8)

#-----------------------------------------------------------#
# Force MPI compilers
#-----------------------------------------------------------#
include(CMakeForceCompiler)
CMAKE_FORCE_CXX_COMPILER(mpicxx "MPI C++ Compiler")
CMAKE_FORCE_C_COMPILER(mpicc "MPI C Compiler")

include(CheckIncludeFileCXX)
#-----------------------------------------------------------#
# Starting project definition
#-----------------------------------------------------------#
project(OMNISCIO)

#-----------------------------------------------------------#
# EXTERNAL_ROOT is a variable set once and for all to
# indicate where are all the required libraries. By default
# CMake will look for libraries in this directory.
#-----------------------------------------------------------#
set(EXTERNAL_ROOT ~/local)

#-----------------------------------------------------------#
add_definitions (-g -Wall -rdynamic)
add_definitions (-Wextra -pedantic -Wno-long-long) # for development only
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC")
#-----------------------------------------------------------#

#-----------------------------------------------------------#
# Set the location of CMake Modules
#-----------------------------------------------------------#
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} 
		"${OMNISCIO_SOURCE_DIR}/cmake/Modules/")

check_include_file_cxx(cxxabi.h HAVE_CXXABI)
if(HAVE_CXXABI)
	add_definitions (-DHAVE_CXXABI)
endif(HAVE_CXXABI)

set(DEP_LIBRARIES dl)

#-----------------------------------------------------------#
include_directories(${OMNISCIO_SOURCE_DIR})
include_directories(${OMNISCIO_SOURCE_DIR}/include)
include_directories(${OMNISCIO_SOURCE_DIR}/src)

#-----------------------------------------------------------#
# Set subdirectories for the compilation process
#-----------------------------------------------------------#
add_subdirectory (lib)
add_subdirectory (test)
add_subdirectory (bin)
#-----------------------------------------------------------#
# Set the install files and directories
#-----------------------------------------------------------#
install(FILES "include/omniscio.h" DESTINATION include)
install(FILES "lib/libomniscio.a" DESTINATION lib)

