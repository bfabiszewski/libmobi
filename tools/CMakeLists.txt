# Copyright (c) 2022 Bartek Fabiszewski
# http://www.fabiszewski.net
#
# This file is part of libmobi.
# Licensed under LGPL, either version 3, or any later.
# See <http://www.gnu.org/licenses/>

include_directories(${LIBMOBI_SOURCE_DIR}/src)

add_library(common OBJECT common.c)

add_executable(mobitool mobitool.c)
target_link_libraries(mobitool PUBLIC mobi)
target_link_libraries(mobitool PRIVATE common)
add_executable(mobimeta mobimeta.c)
target_link_libraries(mobimeta PUBLIC mobi)
target_link_libraries(mobimeta PRIVATE common)
add_executable(mobidrm mobidrm.c)
target_link_libraries(mobidrm PUBLIC mobi)
target_link_libraries(mobidrm PRIVATE common)

if(USE_XMLWRITER)
# miniz.c zip functions are needed for epub creation
    set(zip_SOURCES
        ${LIBMOBI_SOURCE_DIR}/src/miniz.c
        ${LIBMOBI_SOURCE_DIR}/src/miniz.h
    )
    add_library(zip OBJECT ${zip_SOURCES})
    target_compile_definitions(zip PRIVATE
                               MINIZ_NO_ZLIB_COMPATIBLE_NAMES
                               _POSIX_C_SOURCE=200112L)
    target_link_libraries(mobitool PRIVATE zip)
    target_link_libraries(mobimeta PRIVATE zip)
    if (TOOLS_STATIC AND USE_MINIZ)
        target_link_options(mobitool PRIVATE "-Wl,--allow-multiple-definition")
    endif (TOOLS_STATIC AND USE_MINIZ)
endif(USE_XMLWRITER)
