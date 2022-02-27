# Copyright (c) 2022 Bartek Fabiszewski
# http://www.fabiszewski.net
#
# This file is part of libmobi.
# Licensed under LGPL, either version 3, or any later.
# See <http://www.gnu.org/licenses/>

cmake_minimum_required(VERSION 3.12)

project(LIBMOBI C)

set(CMAKE_C_STANDARD 99)

file(STRINGS ${LIBMOBI_SOURCE_DIR}/configure.ac VERSION_LINE REGEX "AC_INIT\\(\\[libmobi\\], \\[(.*)\\]\\)")
string(REGEX MATCH "([0-9]+\\.[0-9]+)" PACKAGE_VERSION "${VERSION_LINE}")
message(STATUS "libmobi version ${PACKAGE_VERSION}")
add_definitions(-DPACKAGE_VERSION="${PACKAGE_VERSION}")
string(REPLACE "." ";" VERSION_LIST ${PACKAGE_VERSION})
list(GET VERSION_LIST 0 PACKAGE_VERSION_MAJOR)
list(GET VERSION_LIST 1 PACKAGE_VERSION_MINOR)

# Option to enable encryption
option(USE_ENCRYPTION "Enable encryption" ON)

# Option to enable static tools compilation
option(TOOLS_STATIC "Enable static tools compilation" OFF)

# Option to use libxml2
option(USE_LIBXML2 "Use libxml2 instead of internal xmlwriter" ON)

# Option to use zlib
option(USE_ZLIB "Use zlib" ON)

# Option to enable XMLWRITER
option(USE_XMLWRITER "Enable xmlwriter (for opf support)" ON)

# Option to enable debug
option(MOBI_DEBUG "Enable debug" OFF)

# Option to enable debug alloc
option(MOBI_DEBUG_ALLOC "Enable debug alloc" OFF)

option(BUILD_SHARED_LIBS "Build using shared libraries" ON)

if(TOOLS_STATIC)
    set(BUILD_SHARED_LIBS OFF)
endif(TOOLS_STATIC)

if(USE_ENCRYPTION)
    add_definitions(-DUSE_ENCRYPTION)
endif(USE_ENCRYPTION)

if(USE_XMLWRITER)
    add_definitions(-DUSE_XMLWRITER)
    if(USE_LIBXML2)
        add_definitions(-DUSE_LIBXML2)
        find_package(LibXml2 REQUIRED)
        include_directories(${LIBXML2_INCLUDE_DIR})
    endif(USE_LIBXML2)
endif(USE_XMLWRITER)

if(MOBI_DEBUG)
    add_definitions(-DMOBI_DEBUG)
    add_compile_options(-pedantic -Wall -Wextra -Werror)
endif(MOBI_DEBUG)

if(MOBI_DEBUG_ALLOC)
    add_definitions(-DMOBI_DEBUG_ALLOC)
endif(MOBI_DEBUG_ALLOC)

if(USE_ZLIB)
    find_package(ZLIB REQUIRED)
    include_directories(${ZLIB_INCLUDE_DIR})
else()
    add_definitions(-DUSE_MINIZ)
endif(USE_ZLIB)

include(CheckIncludeFile)
include(CheckFunctionExists)
check_include_file(unistd.h HAVE_UNISTD_H)
if(HAVE_UNISTD_H)
    add_definitions(-DHAVE_UNISTD_H)
endif(HAVE_UNISTD_H)
check_function_exists(getopt HAVE_GETOPT)
if(HAVE_GETOPT)
    add_definitions(-DHAVE_GETOPT)
endif(HAVE_GETOPT)

add_subdirectory(src)
add_subdirectory(tools)