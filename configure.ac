#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.62])
AC_INIT([libmobi], [0.11])
AC_CONFIG_SRCDIR([src/buffer.c])

# Enable automake
AM_INIT_AUTOMAKE([1.11 -Wall foreign subdir-objects])
# all defined C macros (HAVE_*) will be saved to this file
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_MACRO_DIR([m4])

# Checks for programs.
AC_PROG_CC
m4_version_prereq([2.70], [], [AC_PROG_CC_C99])
AM_PROG_CC_C_O
AC_PROG_INSTALL
m4_ifdef([AM_PROG_AR], [AM_PROG_AR])

# Init libtool
m4_ifdef([LT_INIT], [LT_INIT], [AC_PROG_LIBTOOL])

# Checks for libraries.

# Checks for header files.
AC_HEADER_STDBOOL
AC_CHECK_HEADERS([stdlib.h string.h utime.h unistd.h sys/resource.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_INT32_T
AC_TYPE_INT64_T
AC_TYPE_INT8_T
AC_TYPE_SIZE_T
AC_TYPE_UINT16_T
AC_TYPE_UINT32_T
AC_TYPE_UINT64_T
AC_TYPE_UINT8_T

# Checks for library functions.
AC_FUNC_MKTIME
AC_CHECK_FUNCS([memmove memset mkdir strdup strpbrk strrchr strstr strtoul utime])

# check for getopt() function
AC_MSG_CHECKING([for getopt])
saved_CFLAGS="$CFLAGS"
CFLAGS="-Werror"
AC_COMPILE_IFELSE(
    [AC_LANG_PROGRAM(
        [[#if HAVE_UNISTD_H
          # include <unistd.h>
          #endif]],
        [[return getopt(0, NULL, NULL);]])],
    [have_getopt=yes
     AC_DEFINE([HAVE_GETOPT], [1], [Define whether getopt() function is available])],
    [have_getopt=no])
CFLAGS="$saved_CFLAGS"
AC_MSG_RESULT([$have_getopt])
AM_CONDITIONAL([USE_INTERNAL_GETOPT], [test x$have_getopt = xno])

# Check for oracle solaris studio c compiler
AC_CHECK_DECL([__SUNPRO_C], [SUNCC=yes], [SUNCC=no])

# Get rid of extended attributes in release archive on macOS
case "$host" in
    *-*-darwin*)
        am__tar="COPY_EXTENDED_ATTRIBUTES_DISABLE=1 COPYFILE_DISABLE=1 ${am__tar}"
esac

# Check for -fvisibility=hidden to determine if we can do GNU-style
# visibility attributes for symbol export control
AC_MSG_CHECKING([for visibility hidden compiler flag])
VISIBILITY_HIDDEN=
if test x$SUNCC = xyes; then
    # check if we can use -xldscope=hidden
    saved_CFLAGS="$CFLAGS"
    CFLAGS="-xldscope=hidden"
    AC_COMPILE_IFELSE(
        [AC_LANG_PROGRAM([[]], [[]])],
        [enable_fvisibility_hidden=yes],
        [enable_fvisibility_hidden=no])
    CFLAGS="$saved_CFLAGS"

    AS_IF([test x$enable_fvisibility_hidden = xyes], [VISIBILITY_HIDDEN="-xldscope=hidden"])
else
    case "$host" in
        *-*-mingw*)
            # on mingw32 we do -fvisibility=hidden and __declspec(dllexport)
            VISIBILITY_HIDDEN="-fvisibility=hidden"
            ;;
        *)
            # on other compilers, check if we can do -fvisibility=hidden
            saved_CFLAGS="$CFLAGS"
            CFLAGS="-fvisibility=hidden -Werror"
            AC_COMPILE_IFELSE(
                [AC_LANG_PROGRAM([[]], [[]])],
                [enable_fvisibility_hidden=yes],
                [enable_fvisibility_hidden=no])
            CFLAGS="$saved_CFLAGS"

            AS_IF([test x$enable_fvisibility_hidden = xyes], [VISIBILITY_HIDDEN="-fvisibility=hidden"])
            ;;
    esac
fi
AC_MSG_RESULT([$VISIBILITY_HIDDEN])
AC_SUBST([VISIBILITY_HIDDEN])

# MinGW seems to need this
case "$host" in
    *-*-mingw*)
        NO_UNDEFINED="-no-undefined"
        AVOID_VERSION="-avoid-version"
        ISO99_SOURCE="-D_ISOC99_SOURCE=1"
        WIN32=yes
        ;;
    *)
        NO_UNDEFINED=
        AVOID_VERSION=
        ISO99_SOURCE=
        WIN32=no
        ;;
esac
AC_SUBST([NO_UNDEFINED])
AC_SUBST([AVOID_VERSION])
AC_SUBST([ISO99_SOURCE])
AC_SUBST([WIN32])

# Override default versioning of Darwin linker
case "$host" in
    *-*-darwin*)
        case "$host" in
            # exclude ppc as it breaks linker
            ppc-* | powerpc-*)
                DARWIN_LDFLAGS=
                ;;
            *)
                MAJOR=`echo "${PACKAGE_VERSION}" | cut -d . -f 1`
                DARWIN_LDFLAGS="-Wl,-compatibility_version,${MAJOR} -Wl,-current_version,${PACKAGE_VERSION}"
                ;;
        esac
        ;;
    *)
        DARWIN_LDFLAGS=
        ;;
esac
AC_SUBST([DARWIN_LDFLAGS])

# Check for --allow-multiple-definition support in linker
AC_MSG_CHECKING([whether linker supports --allow-multiple-definition flag])
MOBI_ALLOW_MULTIPLE=
saved_CFLAGS="$CFLAGS"
CFLAGS="-Wl,--allow-multiple-definition -Werror"
AC_COMPILE_IFELSE(
    [AC_LANG_PROGRAM([[]], [[]])],
    [def_allow_multiple=yes],
    [def_allow_multiple=no])
CFLAGS="$saved_CFLAGS"
AS_IF([test x$def_allow_multiple = xyes], [MOBI_ALLOW_MULTIPLE="-Wl,--allow-multiple-definition"])
AC_MSG_RESULT([$def_allow_multiple])
AC_SUBST([MOBI_ALLOW_MULTIPLE])

# Check for non-broken inline under various spellings
AC_MSG_CHECKING([for inline keyword])
def_inline=""
for inline_key in inline __inline__ __inline
do
    AC_COMPILE_IFELSE(
        [AC_LANG_PROGRAM(
            [[]],
            [[} $inline_key int foo() { return 0; } int bar() { return foo();]])],
        [def_inline=$inline_key; break])
done
AC_MSG_RESULT([$def_inline])
AC_DEFINE_UNQUOTED([MOBI_INLINE], [$def_inline], [How to obtain function inlining.])

# Check for noreturn attribute support
AC_MSG_CHECKING([whether compiler supports noreturn attribute])
AC_LINK_IFELSE(
    [AC_LANG_PROGRAM([[]], [[void foo( void ) __attribute__((noreturn));]])],
    [AC_MSG_RESULT([yes])
     AC_DEFINE([HAVE_ATTRIBUTE_NORETURN], [1], [Define to 1 if compiler supports __attribute__((noreturn))])],
    [AC_MSG_RESULT([no])]
)

# Check --enable-xmlwriter
XMLWRITER_OPT=""
AC_MSG_CHECKING([whether enable xmlwriter (for opf support)])
AC_ARG_ENABLE(
    [xmlwriter],
    [AS_HELP_STRING([--enable-xmlwriter], [enable xmlwriter (for opf support) @<:@default=yes@:>@])],
    [case "$enableval" in
         yes) xmlwriter=yes ;;
         no)  xmlwriter=no ;;
         *)   AC_MSG_ERROR([bad value $enableval for --enable-xmlwriter]) ;;
     esac],
    [xmlwriter=yes])
AC_MSG_RESULT([$xmlwriter])
AM_CONDITIONAL([USE_XMLWRITER], [test x$xmlwriter = xyes])
if test x$xmlwriter = xyes; then
    AC_DEFINE([USE_XMLWRITER], [1], [Define whether enable xmlwriter (for opf support)])

    # test for --with-libxml2
    AC_MSG_CHECKING([whether compile with libxml2])
    AC_ARG_WITH(
        [libxml2],
        [AS_HELP_STRING([--with-libxml2], [Use libxml2 instead of internal xmlwriter @<:@default=yes@:>@])],
        [if test "x$withval" = xyes; then use_libxml2=yes; else use_libxml2=no; fi],
        [use_libxml2=yes])
    AC_MSG_RESULT([$use_libxml2])

    if test x$use_libxml2 = xyes; then
        AC_ARG_VAR([XML2_CONFIG], [path to xml2-config utility])
        AC_CHECK_PROGS([XML2_CONFIG], [xml2-config])
        AC_CHECK_PROGS([PKG_CONFIG], [pkg-config])
        AC_MSG_CHECKING([for libxml2 path supplier])
        if test -n "$PKG_CONFIG" && $PKG_CONFIG --exists libxml-2.0; then
            LIBXML2_CFLAGS="`$PKG_CONFIG --cflags libxml-2.0`"
            LIBXML2_LDFLAGS="`$PKG_CONFIG --libs libxml-2.0`"
            AC_MSG_RESULT([pkg-config])
        elif test -n "$XML2_CONFIG"; then
            LIBXML2_CFLAGS="`$XML2_CONFIG --cflags`"
            LIBXML2_LDFLAGS="`$XML2_CONFIG --libs`"
            AC_MSG_RESULT([xml2-config])
        else
            LIBXML2_CFLAGS=-I/usr/include/libxml2
            LIBXML2_LDFLAGS=-lxml2
            AC_MSG_RESULT([generic])
        fi
        saved_CPPFLAGS=$CPPFLAGS
        CPPFLAGS="$CPPFLAGS $LIBXML2_CFLAGS"
        AC_CHECK_HEADER(
            [libxml/xmlwriter.h],
            [AC_DEFINE([USE_LIBXML2], [1], [Define if you want to use libxml2 library])],
            [AC_MSG_ERROR([couldn't find libxml2])])
        CPPFLAGS=$saved_CPPFLAGS
    else
        LIBXML2_LDFLAGS=
        LIBXML2_CFLAGS=
    fi
    AC_SUBST([LIBXML2_LDFLAGS])
    AC_SUBST([LIBXML2_CFLAGS])
    XMLWRITER_OPT="yes"
fi
AM_CONDITIONAL([USE_LIBXML2], [test x$use_libxml2 = xyes])
AC_SUBST([XMLWRITER_OPT])

# Check --enable-encryption
ENCRYPTION_OPT=""
AC_MSG_CHECKING([whether enable encryption])
AC_ARG_ENABLE(
    [encryption],
    [AS_HELP_STRING([--enable-encryption], [enable encryption @<:@default=yes@:>@])],
    [case "$enableval" in
         yes) encryption=yes ;;
         no)  encryption=no ;;
         *)   AC_MSG_ERROR([bad value $enableval for --enable-encryption]) ;;
     esac],
    [encryption=yes])
AC_MSG_RESULT([$encryption])
AM_CONDITIONAL([USE_ENCRYPTION], [test x$encryption = xyes])
if test x$encryption = xyes; then
    AC_DEFINE([USE_ENCRYPTION], [1], [Enable encryption])
    ENCRYPTION_OPT="yes"

    AC_CHECK_HEADERS([sys/random.h])
    AC_MSG_CHECKING([for getrandom with a standard API])
    AC_LINK_IFELSE(
        [AC_LANG_PROGRAM(
            [[#include <stdlib.h>
              #ifdef HAVE_UNISTD_H
              # include <unistd.h>
              #endif
              #ifdef HAVE_SYS_RANDOM_H
              # include <sys/random.h>
              #endif]],
            [[unsigned char buf;
              if (&getrandom != NULL) {
                (void) getrandom((void *) &buf, 1U, 0U);
              }]])],
        [AC_MSG_RESULT([yes])
         AC_CHECK_FUNCS([getrandom])],
        [AC_MSG_RESULT([no])])

fi
AC_SUBST([ENCRYPTION_OPT])

# Check --enable-debug
AC_MSG_CHECKING([whether enable debugging])
AC_ARG_ENABLE(
    [debug],
    [AS_HELP_STRING([--enable-debug], [enable debugging @<:@default=no@:>@])],
    [case "$enableval" in
         yes) debug=yes ;;
         no)  debug=no ;;
         *)   AC_MSG_ERROR([bad value $enableval for --enable-debug]) ;;
     esac],
    [debug=no])
AC_MSG_RESULT([$debug])

DEBUG_CFLAGS=
if test x$debug = xyes; then
    AC_DEFINE([MOBI_DEBUG], [1], [Enable debugging])
    if test x$SUNCC = xyes; then
        DEBUG_CFLAGS="-v -errwarn"
    else
        DEBUG_CFLAGS="-pedantic -Wall -Wextra -Werror"
    fi
fi
AC_SUBST([DEBUG_CFLAGS])

# Check --enable-debug-alloc
AC_MSG_CHECKING([whether enable alloc debugging])
AC_ARG_ENABLE(
    [debug_alloc],
    [AS_HELP_STRING([--enable-debug-alloc], [enable memory allocation debugging @<:@default=no@:>@])],
    [case "$enableval" in
         yes) debug_alloc=yes ;;
         no)  debug_alloc=no ;;
         *)   AC_MSG_ERROR([bad value $enableval for --enable-debug-alloc]) ;;
     esac],
    [debug_alloc=no])
AC_MSG_RESULT([$debug_alloc])

if test x$debug_alloc = xyes; then
    AC_DEFINE([MOBI_DEBUG_ALLOC], [1], [Enable alloc debugging])
fi

# Check --enable-tools-static
AC_MSG_CHECKING([whether link tools against static libmobi])
AC_ARG_ENABLE(
    [tools_static],
    [AS_HELP_STRING([--enable-tools-static], [link tools against static libmobi @<:@default=no@:>@])],
    [case "$enableval" in
         yes) tools_static=yes ;;
         no)  tools_static=no ;;
         *)   AC_MSG_ERROR([bad value $enableval for --enable-tools-static]) ;;
     esac],
    [tools_static=no])
AC_MSG_RESULT([$tools_static])
TOOLS_STATIC=
if test x$tools_static = xyes; then
    TOOLS_STATIC="-static"
fi
AC_SUBST([TOOLS_STATIC])

# test for --with-zlib
AC_MSG_CHECKING([whether compile with zlib])
AC_ARG_WITH(
    [zlib],
    [AS_HELP_STRING([--with-zlib], [Use zlib instead of included miniz @<:@default=yes@:>@])],
    [if test "x$withval" = xyes; then use_zlib=yes; else use_zlib=no; fi],
    [use_zlib=yes])
AC_MSG_RESULT([$use_zlib])
AM_CONDITIONAL([USE_ZLIB], [test x$use_zlib = xyes])
AM_CONDITIONAL([USE_MINIZ], [test x$use_zlib = xno])
AM_CONDITIONAL([USE_STATIC], [test x$tools_static = xyes])
if test x$use_zlib = xyes; then
    AC_CHECK_HEADER(
        [zlib.h],
        [AC_DEFINE([USE_ZLIB], [1], [Define if you want to use system zlib library])
         LIBZ_LDFLAGS=-lz
         MINIZ_CFLAGS=],
        [AC_MSG_ERROR([couldn't find zlib header])])
else
    AC_DEFINE([USE_MINIZ], [1], [Define if you want to use included miniz library])
    MINIZ_CFLAGS="-D_POSIX_C_SOURCE=200112L"
    LIBZ_LDFLAGS=
fi
AC_SUBST([LIBZ_LDFLAGS])
AC_SUBST([MINIZ_CFLAGS])

# Check for md5 or md5sum program, needed for tests
AC_ARG_VAR([MD5PROG], [md5 hashing program executable])
AS_IF([test -z "$MD5PROG"], [AC_CHECK_PROG([MD5PROG], [md5sum], [md5sum -t])], [])
AS_IF([test -z "$MD5PROG"], [AC_CHECK_PROG([MD5PROG], [md5], [md5 -r])], [])
AS_IF([test -z "$MD5PROG"], [AC_MSG_WARN([md5 hashing program not found, some tests will be skipped])], [])
AC_PATH_PROG([BASH_PATH], [bash])
AS_IF(
    [test -z "$BASH_PATH"],
    [AC_MSG_WARN([bash not found, tests will be skipped])
     RUN_TESTS="no"],
    [RUN_TESTS="yes"
     AC_SUBST([BASH_PATH])])
AC_SUBST([RUN_TESTS])

if test x$RUN_TESTS = xyes; then
    # List test files
    cur_dir=`pwd`
    cd "$srcdir"/tests

    for sample_path in samples/*.mobi
    do
        TESTLIST="${TESTLIST} ${sample_path} \\
"
    done
    for sample_path in samples/*.fail
    do
        TESTLIST="${TESTLIST} ${sample_path} \\
"
        FAILLIST="${FAILLIST} ${sample_path} \\
"
    done
    cd $cur_dir
fi

AC_SUBST([TESTLIST])
AC_SUBST([FAILLIST])

AC_CONFIG_FILES([Makefile])
AC_CONFIG_FILES([libmobi.pc])
AC_CONFIG_FILES([src/Makefile])
AC_CONFIG_FILES([tools/Makefile])
AC_CONFIG_FILES([tools/mobitool.1])
AC_CONFIG_FILES([tools/mobimeta.1])
AC_CONFIG_FILES([tools/mobidrm.1])
AC_CONFIG_FILES([tests/Makefile])
AC_CONFIG_FILES([tests/test.sh], [chmod +x tests/test.sh])

AC_OUTPUT
