# Copyright (c) 2011 Sergey Kolotsey.
# This file if part of expat-dom library.
# See the file COPYING for copying permission.

# This file is a M4 script file that defines extra functions
# required by configure script


dnl EQ_SET_MATHLIBS()
dnl Checks for required math libraries and headers. This function sets MATHLIBS 
dnl variable to a name of the library that needs to be prepended to LIBS and 
dnl adds HAVE_MATH_H to config.h header file if math.h is found
AC_DEFUN([EQ_SET_MATHLIBS], [
	ac_save_libs=$LIBS
	AC_SEARCH_LIBS(sqrt, m, [ac_lib_found=true], [ac_lib_found=false])
	AC_CHECK_HEADERS([math.h])
	LIBS=$ac_save_libs
	if test "X$ac_lib_found" = "Xtrue"; then
		if test "X$ac_cv_search_sqrt" != "Xnone required"; then
			MATHLIBS="$MATHLIBS $ac_cv_search_sqrt"
		fi
	else
		AC_MSG_ERROR([
Required math library was not found in a set of standard library paths. If you
have this library (libm or other) installed in a nonstandard lib directory,
or the name of the library is other than libm, provide path to that directory
and name of the library in LDFLAGS, e.g. LDFLAGS=-L<lib dir> -l<lib name>])
	fi
	AC_SUBST(MATHLIBS)
])


dnl EQ_SET_DLLIBS()
dnl Checks for required dl libraries and headers. This function sets DLLIBS 
dnl variable to a name of the library that needs to be prepended to LIBS and 
dnl adds HAVE_DLFCN_H to config.h header file if dlfcn.h is found
AC_DEFUN([EQ_SET_DLLIBS], [
	ac_save_libs=$LIBS
	AC_SEARCH_LIBS(dlopen, [dl dld], [ac_lib_found=true], [ac_lib_found=false])
	AC_CHECK_HEADERS([dlfcn.h])
	LIBS=$ac_save_libs
	if test "X$ac_lib_found" = "Xtrue"; then
		if test "X$ac_cv_search_dlopen" != "Xnone required"; then
			DLLIBS="$DLLIBS $ac_cv_search_dlopen"
		fi
	else
		AC_MSG_ERROR([
Required dl library was not found in a set of standard library paths. If you
have this library (libdl or other) installed in a nonstandard lib directory,
or the name of the library is other than libdl, provide path to that directory
and name of the library in LDFLAGS, e.g. LDFLAGS=-L<lib dir> -l<lib name>])
	fi
	AC_SUBST(DLLIBS)
])


dnl EQ_SET_SOCKETLIBS()
dnl Checks for required socket libraries and headers. This function sets 
dnl SOCKETLIBS variable to a name of the library(es) that needs to be prepended 
dnl to LIBS and adds HAVE_SOCKET_H to config.h header file if socket.h is found
AC_DEFUN([EQ_SET_SOCKETLIBS], [
	ac_save_libs=$LIBS
	AC_SEARCH_LIBS(getservbyname, nsl socket, [ac_nsl_lib_found=true], [ac_nsl_lib_found=false])
	AC_SEARCH_LIBS(connect, socket, [ac_socket_lib_found=true], [ac_socket_lib_found=false])
	AC_CHECK_HEADERS([socket.h sys/socket.h])
	LIBS=$ac_save_libs
	if test "X$ac_nsl_lib_found" = "Xtrue" && test "X$ac_socket_lib_found" = "Xtrue"; then
		if test "X$ac_cv_search_getservbyname" != "Xnone required"; then
			SOCKETLIBS="$SOCKETLIBS $ac_cv_search_getservbyname"
		fi
		if test "X$ac_cv_search_connect" != "Xnone required"; then
			SOCKETLIBS="$SOCKETLIBS $ac_cv_search_connect"
		fi
		dnl remove dublicates
		ac_socket_libs_ret=
		for ac_socketlibs_i in $SOCKETLIBS; do
			ac_socketlibs_found=false
			for ac_socketlibs_j in $ac_socketlibs_ret; do
				if test "X$ac_socketlibs_i" = "X$ac_socketlibs_j"; then
					ac_socketlibs_found=true
					break
				fi
			done
			if test "X$ac_socketlibs_found" = "Xfalse"; then
				ac_socketlibs_ret="$ac_socketlibs_ret $ac_socketlibs_i"
			fi
		done
		SOCKETLIBS=$ac_socketlibs_ret
	else
		AC_MSG_ERROR([
Required libraries were not found in a set of standard library paths. Calls to
connect() and/or getservbyname() functons are impossible to make. If you
have these libraries (libsocket, libnsl) installed in a nonstandard lib 
directory, provide path to that directory and name of the library in LDFLAGS,
e.g. LDFLAGS=-L<lib dir> -l<lib name>])
	fi
	AC_SUBST(SOCKETLIBS)
])


dnl EQ_SET_THREADLIBS()
dnl Test for pthreads support
dnl Taken from ICU FreeBSD Port configure script
AC_DEFUN([EQ_PTHREAD_FREEBSD],[
	dnl FreeBSD: Try ports/linuxthreads first - Mammad Zadeh <mammad@yahoo-inc.com>
	dnl FreeBSD -pthread check - Jonathan McDowell <noodles@earth.li>
	AC_CHECK_LIB(lthread, pthread_create,
		[CFLAGS="-D_THREAD_SAFE -I/usr/local/include/pthread/linuxthreads -I/usr/include $CFLAGS"
		THREADLIBS="-L/usr/local/lib -llthread -llgcc_r"],
		[AC_MSG_CHECKING([if we need -pthread for threads])
			AC_CACHE_VAL(ac_cv_ldflag_pthread,[
				ac_save_LDFLAGS="$LDFLAGS"
				LDFLAGS="-pthread $LDFLAGS"
				AC_TRY_LINK([char pthread_create();],
					pthread_create();,
					eval "ac_cv_ldflag_pthread=yes",
					eval "ac_cv_ldflag_pthread=no"
				),
				THREADLIBS="$ac_save_LDFLAGS"
			])
		if eval "test \"`echo $ac_cv_ldflag_pthread`\" = yes"; then
			AC_MSG_RESULT(yes)
		else
			AC_MSG_RESULT(no)
		fi
		],-L/usr/local/lib)
])
AC_DEFUN([EQ_SET_THREADLIBS], [
	have_pthreads="no"
	dnl Test for HPUX cma threads first
	AC_CHECK_LIB(cma,pthread_create,THREADLIBS="$THREADLIBS -lpthread")
	if test $ac_cv_lib_cma_pthread_create = yes; then
		have_pthreads="yes"
	fi
	dnl special pthread handling
	dnl AIX uses pthreads instead of pthread, and HP/UX uses cma
	dnl FreeBSD users -pthread
	AC_CHECK_LIB(pthread,pthread_create,THREADLIBS="$THREADLIBS -lpthread")
	if test $ac_cv_lib_pthread_pthread_create = yes; then
		have_pthreads="yes"
	else
		dnl For HP 11
		AC_CHECK_LIB(pthread,pthread_mutex_init,THREADLIBS="$THREADLIBS -lpthread")
		if test $ac_cv_lib_pthread_pthread_mutex_init = yes; then
			have_pthreads="yes"
		fi
	fi
	dnl AIX uses pthreads instead of pthread
	if test $have_pthreads = "no"; then
		AC_CHECK_LIB(pthreads,pthread_create,THREADLIBS="$THREADLIBS -lpthreads")
		if test $ac_cv_lib_pthreads_pthread_create = yes; then
			have_pthreads="yes"
		fi
	fi
	dnl all other thread tests fail, try BSD's -pthread
	if test $have_pthreads = "no"; then
		EQ_PTHREAD_FREEBSD
	fi
	AC_SUBST(THREADLIBS)
])


dnl EQ_CHECK_PACKAGE(flags-prefix, package-name [,check-libs [,check-headers]])
dnl Check for specific library and header files for required package. Names of 
dnl the package libraries and specific functions in that libraries are provided
dnl as `check-libs' argument of the macro call. Names of the package header 
dnl files are provided as `check-headers' argument. The macro substitutes
dnl flags-prefix_LIBS and flags-prefix_CFLAGS variables
dnl for example:
dnl    EQ_CHECK_PACKAGE( ZLIB, zlib)
dnl will result in ZLIB_LIBS and ZLIB_CFLAGS variables set to libs and cflags
dnl for the package zlib. Another example:
dnl    EQ_CHECK_PACKAGE( PKG, packagename, [pkglib1:function1 pkglib2:function2], [header1.h header2.h])
AC_DEFUN([EQ_CHECK_PACKAGE],[
	ac_new_cflags=
	ac_new_libs=
	ac_cflags_given=false
	ac_libs_given=false
	ac_cflags_found=false
	ac_libs_found=false
	ac_pakage_cflags_found=false
	ac_package_libs_found=false
	
	if test "X$4" != "X"; then
		AC_ARG_WITH([$2-inc], [AS_HELP_STRING([--with-$2-inc], [location of `$2' header files @<:@directories provided in CFLAGS@:>@])], [ac_new_cflags=-I$withval; ac_cflags_given=true])
	fi
	AC_ARG_WITH([$2-lib], [AS_HELP_STRING([--with-$2-lib], [location of `$2' library files @<:@directories provided in LIBS or LDFLAGS@:>@])], [ac_new_libs=-L$withval; ac_libs_given=true])

	dnl try using pkg-config
	if test "X$ac_cflags_given" != "Xtrue" && test "X$ac_libs_given" != "Xtrue"; then
		PKG_CHECK_MODULES( [$1], [$2], [ac_package_cflags_found=true; ac_package_libs_found=true], [ac_package_cflags_found=false; ac_package_libs_found=false])
	fi
	
	dnl if pkg-config did not provide us with correct result, try other tests
	ac_save_cflags=$CFLAGS
	ac_save_cppflags=$CPPFLAGS
	ac_save_ldflags=$LDFLAGS
	ac_save_libs=$LIBS
	CPPFLAGS="$CPPFLAGS $ac_new_cflags $$1_CFLAGS"
	LIBS="$LIBS $ac_new_libs $$1_LIBS"
	
	if test "X$4" = "X"; then
		if test "X$ac_package_cflags_found" = "Xtrue"; then
			ac_header_found=true
		else
			ac_header_found=false
		fi
	else
		AC_CHECK_HEADERS([$4], [ac_header_found=true], [ac_header_found=false])
	fi
	if test "X$ac_header_found" = "Xtrue"; then
		if test "X$3" = "X"; then
			if test "X$ac_package_libs_found" = "Xtrue"; then
				ac_libs_found=true	
			else
				ac_libs_found=false
			fi
		else
			ac_libs_found=true
			for ac_ii in $3; do
				ac_library=${ac_ii%%:*}
				ac_function=${ac_ii#*:}
				AC_SEARCH_LIBS( [$ac_function], [$ac_library], [ac_libs_found=true], [ac_libs_found=false])
				if test "X$ac_libs_found" = "Xtrue"; then
					eval "ac_cv_search_function=\$ac_cv_search_$ac_function"
					if test "X$ac_cv_search_function" != "Xnone required"; then
						ac_new_libs="$ac_new_libs $ac_cv_search_function";
					fi
				else
					break
				fi
			done
		fi
	fi
	CFLAGS=$ac_save_cflags
	CPPFLAGS=$ac_save_cppflags
	LDFLAGS=$ac_save_ldflags
	LIBS=$ac_save_libs
	
	if test "X$ac_header_found" = "Xtrue" && test "X$ac_libs_found" = "Xtrue" ; then
		$1_CFLAGS="$$1_CFLAGS $ac_new_cflags"
		$1_LIBS="$$1_LIBS $ac_new_libs"
		AC_SUBST($1_CFLAGS)
		AC_SUBST($1_LIBS)
	else
		AC_MSG_ERROR([
Required library and/or headers files from required package `$2'
were not found in a set of standard library paths. If you have this package
installed in a nonstandard directory, provide path to include and lib
directories using command line arguments:
    --with-$2-inc=<include dir>
    --with-$2-lib=<lib dir>
See 
    $as_me --help
for more information])
	fi
])

dnl EQ_PACKAGE_ENABLED(flags-prefix, package-name [,check-libs [,check-headers]])
dnl Check for specific library and header files for required package. Checks if
dnl user enabled this package or if it could be enabled by default. Names of 
dnl the package libraries and specific functions in that libraries are provided
dnl as `check-libs' argument of the macro call. Names of the package header 
dnl files are provided as `check-headers' argument. The macro substitutes
dnl flags-prefix_LIBS and flags-prefix_CFLAGS and flags-prfix_ENABLED variables
dnl for example:
dnl    EQ_PACKAGE_ENABLED( ZLIB, zlib)
dnl will result in ZLIB_LIBS and ZLIB_CFLAGS variables set to libs and cflags
dnl for the package zlib and ZLIB_ENABLED set to true if package is installed.
AC_DEFUN([EQ_PACKAGE_ENABLED],[
	ac_new_cflags=
	ac_new_libs=
	ac_cflags_given=false
	ac_libs_given=false
	ac_cflags_found=false
	ac_libs_found=false

	AC_ARG_WITH([$2], [AS_HELP_STRING([--with-$2], [enable package `$2' support, @<:@default=check@:>@])], [ac_with_package=$withval], [ac_with_package=check])
	
	if test "X$4" != "X"; then
		AC_ARG_WITH([$2-inc], [AS_HELP_STRING([--with-$2-inc], [location of package `$2' header files @<:@directories provided in CFLAGS@:>@])], [ac_new_cflags=-I$withval; ac_cflags_given=true; ac_with_package=yes])
	fi
	AC_ARG_WITH([$2-lib], [AS_HELP_STRING([--with-$2-lib], [location of package `$2' library files @<:@directories provided in LIBS and LDFLAGS@:>@])], [ac_new_libs=-L$withval; ac_libs_given=true; ac_with_package=yes])

	if test "X$ac_with_package" != "Xno"; then
		dnl try using pkg-config
		if test "X$ac_cflags_given" != "Xtrue" && test "X$ac_libs_given" != "Xtrue"; then
			PKG_CHECK_MODULES( [$1], [$2], [ac_package_cflags_found=true; ac_package_libs_found=true], [ac_package_cflags_found=false; ac_package_libs_found=false])
		fi
	
		ac_save_cflags=$CFLAGS
		ac_save_cppflags=$CPPFLAGS
		ac_save_ldflags=$LDFLAGS
		ac_save_libs=$LIBS
		CPPFLAGS="$CPPFLAGS $ac_new_cflags $$1_CFLAGS"
		LIBS="$LDFLAGS $ac_new_libs $$1_LIBS"
		if test "X$4" = "X"; then
			if test "X$ac_cflags_given" = "Xtrue" || test "X$ac_package_cflags_found" = "Xtrue"; then
				ac_header_found=true
			else
				ac_header_found=false
			fi
		else
			AC_CHECK_HEADERS([$4], [ac_header_found=true], [ac_header_found=false])
		fi
		if test "X$ac_header_found" = "Xtrue"; then
			if test "X$3" = "X"; then
				if test "X$ac_libs_given" = "Xtrue" || test "X$ac_package_libs_found" = "Xtrue"; then
					ac_libs_found=true	
				else
					ac_libs_found=false
				fi
			else
				ac_libs_found=true
				for ac_ii in $3; do
					ac_library=${ac_ii%%:*}
					ac_function=${ac_ii#*:}
					AC_SEARCH_LIBS( [$ac_function], [$ac_library], [ac_libs_found=true], [ac_libs_found=false])
					if test "X$ac_libs_found" = "Xtrue"; then
						eval "ac_cv_search_function=\$ac_cv_search_$ac_function"
						if test "X$ac_cv_search_function" != "Xnone required"; then
							ac_new_libs="$ac_new_libs $ac_cv_search_function";
						fi
					else
						break
					fi
				done
			fi
		fi
		CFLAGS=$ac_save_cflags
		CPPFLAGS=$ac_save_cppflags
		LDFLAGS=$ac_save_ldflags
		LIBS=$ac_save_libs
		if test "X$ac_header_found" = "Xtrue" && test "X$ac_libs_found" = "Xtrue" ; then
			$1_ENABLED=true
			$1_CFLAGS="$$1_CFLAGS $ac_new_cflags"
			$1_LIBS="$$1_LIBS $ac_new_libs"
		elif test "X$ac_with_package" = "Xyes"; then
			AC_MSG_ERROR([
Could not find library and/or headers files for package `$2'
This package was set as required in command line arguments. If you want to 
enable this package, install it. If you have this package installed in a 
nonstandard directory, provide path to include and lib directories using command
line arguments:
	--with-$2-inc=<include dir>
	--with-$2-lib=<lib dir>
See 
    $as_me --help
for more information])
		else
			$1_ENABLED=false
			$1_CFLAGS=
			$1_LIBS=
		fi
	else
		$1_ENABLED=false
		$1_CFLAGS=
		$1_LIBS=
	fi
	AC_SUBST($1_ENABLED)
	AC_SUBST($1_CFLAGS)
	AC_SUBST($1_LIBS)
])


