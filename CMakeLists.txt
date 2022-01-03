cmake_minimum_required(VERSION 3.16)

project(libmobi VERSION 0.9)

set(LIBMOBI_SRCS
    src/buffer.c
    src/compression.c
    src/debug.c
    src/index.c
    src/memory.c
    src/meta.c
    src/parse_rawml.c
    src/read.c
    src/sha1.c
    src/structure.c
    src/util.c
    src/write.c
)

include_directories(src)

option(USE_XMLWRITER "Enable xmlwriter (for opf support)" ON)
if(USE_XMLWRITER)
    list(APPEND LIBMOBI_SRCS src/opf.c)
    add_compile_definitions(USE_XMLWRITER)
endif()

option(USE_LIBXML2 "Use libxml2" ON)
if(USE_LIBXML2)
    find_package(LIBXML2 REQUIRED)
    include_directories(${LIBXML2_INCLUDE_DIRS})
    add_compile_definitions(USE_LIBXML2)
    list(APPEND LIBMOBI_LIBS ${LIBXML2_LIBRARIES})
else()
    list(APPEND LIBMOBI_SRCS src/xmlwriter.c)
endif()

option(USE_ENCRYPTION "Enable encryption" ON)
if(USE_ENCRYPTION)
    list(APPEND LIBMOBI_SRCS src/encryption.c)
    add_compile_definitions(USE_ENCRYPTION)
endif()

option(USE_ZLIB "Use zlib" ON)
if(USE_ZLIB)
    find_package(ZLIB REQUIRED)
    include_directories(${ZLIB_INCLUDE_DIRS})
    add_compile_definitions(USE_ZLIB)
    list(APPEND LIBMOBI_LIBS ${ZLIB_LIBRARIES})
else()
    list(APPEND LIBMOBI_SRCS src/miniz.c)
    add_compile_definitions(USE_MINIZ)
endif()

option(STATIC_BUILD "Build static library" OFF)
if(STATIC_BUILD)
    add_library(mobi STATIC ${LIBMOBI_SRCS})
else()
    add_library(mobi SHARED ${LIBMOBI_SRCS})
endif()
target_link_libraries(mobi ${LIBMOBI_LIBS})
