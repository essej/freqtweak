# Copyright (C) 2002 Jesse Chappell <jesse@essej.net>
#  
# This file is free software; as a special exception the author gives
# unlimited permission to copy and/or distribute it, with or without 
# modifications, as long as this notice is preserved.
# 
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY, to the extent permitted by law; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

AC_INIT(src/FTapp.cpp)


BETA=

FREQTWEAK_MAJOR_VERSION=0
FREQTWEAK_MINOR_VERSION=7
FREQTWEAK_MICRO_VERSION=1

FREQTWEAK_VERSION=$FREQTWEAK_MAJOR_VERSION.$FREQTWEAK_MINOR_VERSION.${FREQTWEAK_MICRO_VERSION}${BETA}

dnl AC_SUBST(FREQTWEAK_VERSION)

AM_INIT_AUTOMAKE(freqtweak, ${FREQTWEAK_VERSION})
AM_MAINTAINER_MODE


AC_PROG_CC
AC_PROG_CPP
AC_PROG_CXX
AC_PROG_INSTALL
AC_PROG_LN_S

AC_HEADER_STDC

dnl ======================
dnl PKGCONFIG preparations
dnl ======================

if test -z $PKG_CONFIG_PATH; then
	PKG_CONFIG_PATH=/usr/local/lib/pkgconfig:/usr/lib/pkgconfig
fi

export PKG_CONFIG_PATH


dnl ==========
dnl JACK stuff
dnl ==========
PKG_CHECK_MODULES(JACK, jack >= 0.80.0, have_jack=yes, have_jack=no)

if test "x$have_jack" != "xno"
then
AC_CHECK_LIB(jack, jack_client_open, 
		   [ AC_DEFINE([HAVE_JACK_CLIENT_OPEN], 1, [Have newer JACK connect call])], 
		   [],
		   [$JACK_LIBS])
fi

AC_SUBST(JACK_LIBS)
AC_SUBST(JACK_CFLAGS)


dnl sigc++
PKG_CHECK_MODULES(SIGCPP, sigc++-1.2 >= 0.14, have_sigc12=yes, have_sigc12=no)

    

AC_CHECK_LIB(m,pow)

AC_CHECK_FUNCS([memmove])
AC_CHECK_FUNCS([memset])
AC_CHECK_FUNCS([mkdir])
AC_CHECK_FUNCS([pow])
AC_CHECK_FUNCS([sqrt])

AC_CHECK_HEADERS([fcntl.h])
AC_CHECK_HEADERS([values.h])


dnl ============
dnl Machine type
dnl ============

AC_MSG_CHECKING(machine architecture)
AC_SUBST(ARCH_TYPE)
ARCH_TYPE=`uname -m`
AC_MSG_RESULT($ARCH_TYPE)

AM_CONDITIONAL(ARCH_X86,test "$ARCH_TYPE" = "i486" || test "$ARCH_TYPE" = "i586" || test "$ARCH_TYPE" = "i686" )
AM_CONDITIONAL(ARCH_486,test "$ARCH_TYPE" = "i486")
AM_CONDITIONAL(ARCH_586,test "$ARCH_TYPE" = "i586")
AM_CONDITIONAL(ARCH_686,test "$ARCH_TYPE" = "i686")
AM_CONDITIONAL(ARCH_PPC,test "$ARCH_TYPE" = "ppc")
AM_CONDITIONAL(ARCH_ULTRA,test "$ARCH_TYPE" = "sun4u")

AC_MSG_CHECKING(for gcc optimization flags)
old_cflags=$CFLAGS
CFLAGS="$CFLAGS -O2 -fexpensive-optimizations -funroll-loops -finline-functions -ffast-math"
AC_TRY_COMPILE([#include <stdio.h>],
[ printf ("hello, world"); ],
[ EXTRA_OPT_CFLAGS="-O2 -fexpensive-optimizations -funroll-loops -finline-functions -ffast-math"],[ EXTRA_OPT_CFLAGS=""])
CFLAGS=$old_cflags
AC_MSG_RESULT([$EXTRA_OPT_CFLAGS])

ARCH_CFLAGS="$CFLAGS $EXTRA_OPT_CFLAGS"

if test "$ARCH_TYPE" = "i486" || test "$ARCH_TYPE" = "i586" || test "$ARCH_TYPE" = "i686"
then
	AC_DEFINE(ARCH_X86, 1, [Define if on x86 platform])
fi

if test "$ARCH_TYPE" = "i486"
then
        AC_DEFINE(ARCH_486, 1, [Define if on a 486 platform])
fi

if test "$ARCH_TYPE" = "i586"
then
        AC_DEFINE(ARCH_586, 1, [Define if on a 586 platform])
fi

if test "$ARCH_TYPE" = "i686"
then
        AC_DEFINE(ARCH_686, 1, [Define if on a 686 platform])
#		EXTRA_CPU_CFLAGS=$(if $CC -march=i686 -S -o /dev/null -xc /dev/null > /dev/null; then echo "-march=i686"; fi)
fi

if test "$ARCH_TYPE" = "ppc"
then
		AC_DEFINE(ARCH_PPC, 1, [Define if on a PPC platform])
fi

if test "$ARCH_TYPE" = "sun4u"
then
		AC_DEFINE(ARCH_ULTRA, 1, [Define if on a SUN ULTRA platform])
fi


dnl ================================================================
dnl Special assembly options. Note: Need to fix the above arch code!
dnl ================================================================

dnl AC_CANONICAL_HOST
AM_CONFIG_HEADER(config.h)

case "$host" in
	i?86-* | k?-* | athlon-*) dnl catch i386,i486,i586,i686,k6,k7
		AC_DEFINE(FPM_INTEL, 1, [Define if we are on Intel])
	;;	
	*-*)
		echo "host_alias = $host_alias"
		AC_DEFINE(FPM_64BIT, 1, [Use this otherwise])
	;;	
esac

ARCH_CFLAGS="$ARCH_CFLAGS $EXTRA_CPU_CFLAGS"

AC_SUBST(ARCH_CFLAGS)

dnl ============================================================
dnl Check for typedefs, structures, and compiler characteristics
dnl ============================================================

AC_C_CONST
AC_TYPE_SIZE_T


# FFTW section
use_fftw=yes

AC_ARG_WITH(fftw,[  --with-fftw=PFX        Prefix where fftw is installed (optional)],
 fftw_prefix="$withval", fftw_prefix="/usr/local") 


AC_ARG_WITH(fftw3,[  --with-fftw3        Try to use fftw3 if installed (yes by default)],
 try_fftw3="$withval", try_fftw3="yes") 


if test "x$try_fftw3" != "xno"
then
   # try fftw3 first
   #PKG_CHECK_MODULES(FFTW, fftw3f >= 3.0, [AC_DEFINE(USING_FFTW3, [1], [Using FFTW3])] , [AC_DEFINE(USING_FFTW3, [0], [Using FFTW3])] )
   PKG_CHECK_MODULES(FFTW, fftw3f >= 3.0, have_fftw3=yes , have_fftw3=no )
fi

if test "x$have_fftw3" = "xyes"
then

   AC_DEFINE(USING_FFTW3, [1], [Using FFTW3])

else
  # no fftw3


if test "x$use_fftw" = "xyes"
then

LDFLAGS="$LDFLAGS -L$fftw_prefix/lib -lm"
JACK_CFLAGS="-I$fftw_prefix/include"

CFLAGS="$CFLAGS $FFTW_CFLAGS"
CPPFLAGS="$CPPFLAGS $FFTW_CFLAGS"

# check for single precision convention first
AC_CHECK_LIB(sfftw, fftw_create_plan)
AC_CHECK_LIB(srfftw, rfftw_create_plan)
AC_CHECK_LIB(srfftw, rfftw_create_plan,
[
have_fftw=yes
],
[
have_fftw=no
])

if [[ "$have_fftw" = "no" ]] ; then
    #    AC_MSG_ERROR("Could not find FFTW! Is it installed (www.fftw.org) ? ")

  AC_CHECK_LIB(fftw, fftw_create_plan)
  AC_CHECK_LIB(rfftw, rfftw_create_plan)

  AC_CHECK_LIB(rfftw, rfftw_create_plan,
  [ 
  have_fftw=yes
  ],
  [
  have_fftw=no
  ])

  if [[ "$have_fftw" = "no" ]] ; then
        AC_MSG_ERROR('Could not find FFTW! Is it installed?  www.fftw.org  ')
  fi

  cat > mytest.$ac_ext <<EOF
#include <fftw.h>
EOF


else

cat > mytest.$ac_ext <<EOF
#include <sfftw.h>
EOF

fi

AC_CHECK_HEADERS(sfftw.h srfftw.h fftw.h rfftw.h)

AC_MSG_CHECKING([if FFTW is actually float precision])
cat >> mytest.$ac_ext <<EOF

int main(int argc,char **argv)
{
   return (sizeof(float) != sizeof(fftw_real));
}

EOF

$CXX -o mytest $CFLAGS $CPPFLAGS mytest.$ac_ext >&5
if ./mytest ; then
AC_MSG_RESULT(yes)
else
AC_MSG_RESULT(no)
AC_MSG_ERROR([
==========================================================================
ERROR: FreqTweak needs an FFTW library compiled with float precision
  Try rebuilding and installing FFTW with the following configure options:
      --enable-float --enable-type-prefix 
==========================================================================
])
fi

rm -f ./mytest ./mytest.$ac_ext

fi

fi
# end fftw section



dnl LIBXML

PKG_CHECK_MODULES(XML, libxml-2.0 >= 2.0, have_xml2=yes, have_xml2=no)



if [[ "$have_jack" = "no" ]] ; then
        AC_MSG_ERROR('Could not find jack! Is it installed (jackit.sf.net) ?')
fi
  

dnl LIBS="$LIBS $JACK_LIB"



#AC_MSG_CHECKING(for wxWindows)

dnl wxwindows
AC_ARG_WITH(wxconfig-path,
            [  --with-wxconfig-path=PATH    full path to wx-config to use])

if test "x$with_wxconfig_path" != "x"  ; then
        WX_CONFIG=$with_wxconfig_path
else
	WX_CONFIG=""
fi

if [[ "$WX_CONFIG" = "" ]] ; then

  AC_PATH_PROG(WX_CONFIG0, wx-config-2.6, no, $PATH:/usr/local/bin )
  if [[ "$WX_CONFIG0" = "no" ]] ; then

   AC_PATH_PROG(WX_CONFIG1, wxgtk2-2.5-config, no, $PATH:/usr/local/bin )
   if [[ "$WX_CONFIG1" = "no" ]] ; then

	AC_PATH_PROG(WX_CONFIG2, wxgtk2-2.4-config, no, $PATH:/usr/local/bin )
	if [[ "$WX_CONFIG2" = "no" ]] ; then

		AC_PATH_PROG(WX_CONFIG3, wxgtk-2.4-config, no, $PATH:/usr/local/bin )
		if [[ "$WX_CONFIG3" = "no" ]] ; then

			AC_PATH_PROG(WX_CONFIG4, wx-config, no, $PATH:/usr/local/bin )
			if [[ "$WX_CONFIG4" = "no" ]] ; then
        			AC_MSG_ERROR("Could not find any wx-config scripts: is wxWindows installed?")
			else	
	   			WX_CONFIG=$WX_CONFIG4
			fi
		else
	   		WX_CONFIG=$WX_CONFIG3
		fi
	else
		WX_CONFIG=$WX_CONFIG2
	fi
   else	
      WX_CONFIG=$WX_CONFIG1
   fi
  else
      WX_CONFIG=$WX_CONFIG0
  fi
fi

AC_SUBST(WX_CONFIG)

WX_LIBS="`$WX_CONFIG --libs`"

if $WX_CONFIG --cxxflags > /dev/null 2>&1 ; then
    WX_CFLAGS="`$WX_CONFIG --cxxflags`"
else
    WX_CFLAGS="`$WX_CONFIG --cflags`"
fi


#UNAME_MACHINE=`uname -m`
#CXXFLAGS="$JACK_CFLAGS  -g -Wall -O3 -fomit-frame-pointer -fstrength-reduce -funroll-loops -fmove-all-movables -ffast-math -mcpu=${UNAME_MACHINE} -march=${UNAME_MACHINE}"

AC_ARG_ENABLE(debug,
      [  --enable-debug    not optimized and includes debug symbols],
      [ if test "x$enable_debug" != "xno" ; then
	  CXXFLAGS="-g -Wall -D_REENTRANT $FFTW_CFLAGS $JACK_CFLAGS $WX_CFLAGS $XML_CFLAGS $SIGCPP_CFLAGS"
	  CFLAGS="$CXXFLAGS"
        else
	  CXXFLAGS="$FFTW_CFLAGS $JACK_CFLAGS $WX_CFLAGS $XML_CFLAGS $SIGCPP_CFLAGS -Wall -D_REENTRANT $ARCH_CFLAGS"
	  CFLAGS="$CXXFLAGS"
	fi
      ],
	[
	  CXXFLAGS="$FFTW_CFLAGS $JACK_CFLAGS $WX_CFLAGS $XML_CFLAGS $SIGCPP_CFLAGS -Wall -D_REENTRANT $ARCH_CFLAGS"
	  CFLAGS="$CXXFLAGS"
        ]
)

#CFLAGS="$CPPFLAGS $FFTW_CFLAGS $JACK_CFLAGS $WX_CFLAGS $XML_CFLAGS -Wall -D_REENTRANT $ARCH_CFLAGS"
#CXXFLAGS="$CFLAGS"

#CXXFLAGS="-g -Wall $FFTW_CFLAGS $JACK_CFLAGS $WX_CFLAGS $XML_CFLAGS"

LIBS="$LIBS $FFTW_LIBS $JACK_LIBS $WX_LIBS $XML_LIBS $SIGCPP_LIBS"


AC_SUBST(FREQTWEAK_MAJOR_VERSION)
AC_SUBST(FREQTWEAK_MINOR_VERSION)
AC_SUBST(FREQTWEAK_MICRO_VERSION)
AC_SUBST(FREQTWEAK_VERSION)

AC_DEFINE_UNQUOTED(HELP_HTML_PATH, "${prefix}/share/${PACKAGE}/help", [Help path])


AC_OUTPUT([
Makefile
src/Makefile
src/version.h
doc/Makefile
])

echo 

echo Now type \'make\' to build FreqTweak.

echo
