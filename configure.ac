# -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.68])

AC_INIT([ndn], [1.0-rc])

AC_CONFIG_SRCDIR([daemon/ndnd.cc])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_AUX_DIR([build-aux])
AC_CONFIG_MACRO_DIR([m4])
AC_LANG([C++])

AM_INIT_AUTOMAKE([1.11 foreign -Wall silent-rules subdir-objects tar-pax no-dist-gzip dist-xz])
AM_SILENT_RULES([yes])

# Checks for programs.
AC_PROG_CXX
AC_PROG_CXXCPP
AM_PROG_AR
AC_PROG_LIBTOOL

# Checks for boost.
BOOST_REQUIRE([1.48])
BOOST_DATE_TIME
BOOST_FOREACH
BOOST_LAMBDA
BOOST_STRING_ALGO
BOOST_THREADS
BOOST_TOKENIZER
BOOST_TUPLE
BOOST_FIND_HEADER([boost/any.hpp])
BOOST_FIND_HEADER([boost/interprocess/ipc/message_queue.hpp])
BOOST_FIND_HEADER([boost/interprocess/smart_ptr/unique_ptr.hpp])
BOOST_FIND_HEADER([boost/multi_index_container.hpp])
BOOST_FIND_HEADER([boost/property_tree/json_parser.hpp])
BOOST_FIND_HEADER([boost/property_tree/ptree.hpp])

# Checks for other libraries.
PKG_CHECK_MODULES(LIBEVENT, [libevent])
PKG_CHECK_MODULES(YAJL, [yajl >= 2.0])

# Checks for header files.
AC_CHECK_HEADERS([arpa/inet.h fcntl.h limits.h netdb.h netinet/in.h stddef.h stdint.h stdlib.h string.h sys/ioctl.h sys/socket.h sys/time.h syslog.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_CHECK_HEADER_STDBOOL
AC_C_INLINE
AC_TYPE_INT32_T
AC_TYPE_INT8_T
AC_TYPE_PID_T
AC_TYPE_SIZE_T
AC_TYPE_UINT16_T
AC_TYPE_UINT32_T
AC_TYPE_UINT64_T
AC_TYPE_UINT8_T

# Checks for library functions.
AC_FUNC_ERROR_AT_LINE
AC_FUNC_FORK
AC_CHECK_FUNCS([floor gethostname gettimeofday inet_ntoa memset pow select socket sqrt strerror strstr])

AC_CONFIG_FILES([Makefile])
AC_OUTPUT
