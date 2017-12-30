# ===========================================================================
#    http://autoconf-archive.cryp.to/ac_cxx_verbose_terminate_handler.html
# ===========================================================================
#
# SYNOPSIS
#
#   AC_CXX_VERBOSE_TERMINATE_HANDLER
#
# DESCRIPTION
#
#   If the compiler does have the verbose terminate handler, define
#   HAVE_VERBOSE_TERMINATE_HANDLER.
#
# LAST MODIFICATION
#
#   2008-04-12
#
# COPYLEFT
#
#   Copyright (c) 2008 Lapo Luchini <lapo@lapo.it>
#
#   Copying and distribution of this file, with or without modification, are
#   permitted in any medium without royalty provided the copyright notice
#   and this notice are preserved.

AC_DEFUN([AC_CXX_VERBOSE_TERMINATE_HANDLER],
[AC_CACHE_CHECK(whether the compiler has __gnu_cxx::__verbose_terminate_handler,
	ac_cv_verbose_terminate_handler,
	[
		AC_LANG_PUSH([C++])
		AC_COMPILE_IFELSE([AC_LANG_PROGRAM(
			[[#include <exception>]],
			[[std::set_terminate(__gnu_cxx::__verbose_terminate_handler);]])],
			[ac_cv_verbose_terminate_handler=yes], [ac_cv_verbose_terminate_handler=no])
		AC_LANG_POP([])
	])
	if test "$ac_cv_verbose_terminate_handler" = yes; then
		AC_DEFINE(HAVE_VERBOSE_TERMINATE_HANDLER, 1,
			[defined to 1 if the compiler has __gnu_cxx::__verbose_terminate_handler, 0 otherwise])
	else
		AC_DEFINE(HAVE_VERBOSE_TERMINATE_HANDLER, 0,
			[defined to 1 if the compiler has __gnu_cxx::__verbose_terminate_handler, 0 otherwise])
	fi
])



