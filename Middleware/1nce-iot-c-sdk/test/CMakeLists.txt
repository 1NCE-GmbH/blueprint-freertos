cmake_minimum_required ( VERSION 3.13.0 )
# set the project name
project( "NCE_IOT_C_SDK"
         VERSION 2.0.0
         LANGUAGES C)


# Use C99 if not specified.
enable_language(C ASM)
set( CMAKE_C_STANDARD 90 )
set( CMAKE_C_STANDARD_REQUIRED ON )
set(CMAKE_C_EXTENSIONS OFF)


# Set global path variables.
get_filename_component(__MODULE_ROOT_DIR "${CMAKE_CURRENT_LIST_DIR}/.." ABSOLUTE)
set(MODULE_ROOT_DIR ${__MODULE_ROOT_DIR} CACHE INTERNAL "nce_sdk repository root.")


# Set output directories.
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)


# ===================================== Coverity Analysis Configuration =================================================

# Include filepaths for source and include.
include( ${MODULE_ROOT_DIR}/nceiotcsdkFilePaths.cmake )

# Target for Coverity analysis that builds the library.
add_library( coverity_analysis
             ${NCE_SOURCES})


# SDK public include path.
target_include_directories( coverity_analysis PUBLIC ${NCE_INCLUDE_PUBLIC_DIRS} )
