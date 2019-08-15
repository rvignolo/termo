dnl Process this file with autoconf to produce a configure script.
dnl
dnl This file is free software; as a special exception the author gives
dnl unlimited permission to copy and/or distribute it, with or without
dnl modifications, as long as this notice is preserved.
dnl
dnl This program is distributed in the hope that it will be useful, but
dnl WITHOUT ANY WARRANTY, to the extent permitted by law; without even the
dnl implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

m4_include([version.m4])
AC_INIT(pluginname,pluginversion,[authoremail])
AC_CONFIG_SRCDIR(src/pluginname.c)

AC_CONFIG_MACRO_DIR([wasora/m4])

WASORA_PLUGIN_INIT_C
WASORA_OPT_FLAGS_C

AC_CONFIG_FILES([Makefile src/Makefile])

WASORA_PLUGIN_VERSION_H
WASORA_CHECK_BASIC_HEADERS
WASORA_CHECK_BASIC_LIBS
WASORA_CHECK_GSL
WASORA_CHECK_IDA
WASORA_CHECK_READLINE

# freesteam
AC_CHECK_LIB([freesteam],[freesteam_region1_set_pT],,[AC_MSG_ERROR([freesteam library not found.])])
AC_CHECK_HEADERS([freesteam/common.h],[],[AC_MSG_ERROR([freesteam headers not found.])])

AC_OUTPUT

AS_BOX([Configuration summary for pluginname standalone executable])
WASORA_RESUME_LIBS


AS_IF([ test $readline -eq 0 ], [
  WASORA_FINAL_WARN([0])
],[
  WASORA_FINAL_WARN([1])
])
