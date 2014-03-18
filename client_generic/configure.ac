gdnl Process this file with autoconf to produce a configure script.
AC_PREREQ(2.57)

ifdef([m4_pattern_allow],
      [m4_pattern_allow([^AC_.*])])

AC_INIT(electricsheep,2.7b33-svn,http://code.google.com/p/electricsheep/issues/list)
AC_CANONICAL_HOST
AC_CANONICAL_TARGET


AC_CONFIG_SRCDIR([Makefile.am])
AM_INIT_AUTOMAKE([1.9 tar-pax dist-bzip2])

AM_MAINTAINER_MODE

dnl Uncomment next line to have it build in /usr by default rather than /usr/local
dnl AC_PREFIX_DEFAULT(/usr)

AC_GNU_SOURCE
AC_ISC_POSIX
AC_PROG_INSTALL
AC_PROG_CC
AC_PROG_CXX

AC_CONFIG_MACRO_DIR([m4])

AC_LIBTOOL_DLOPEN
AC_PROG_LIBTOOL
AM_PROG_CC_STDC
AM_PROG_CC_C_O
AC_HEADER_STDC
AC_C_BIGENDIAN
AC_C_CONST
AC_C_INLINE
AC_SYS_LARGEFILE

AM_PROG_AS

ACX_PTHREAD(PTHREAD_OK=true,PTHREAD_OK=false)

IS_LINUX_GNU="false"

case "{$target_os}" in
 *linux-gnu*)
   IS_LINUX_GNU="true"
   echo "O/S is GNU/Linux"
  ;;
 *)
  ;;
esac

AC_PATH_PROG(PKG_CONFIG, pkg-config, no)

if test x$PKG_CONFIG = xno; then
   AC_MSG_ERROR([Cannot find pkg-config, make sure it is installed and in your PATH])
fi


AM_CONDITIONAL(IS_LINUX_GNU,$IS_LINUX_GNU)

dnl Use -Wall if we have gcc.
changequote(,)dnl
if test "x$GCC" = "xyes"; then
  case " $CFLAGS " in
  *[\ \	]-Wall[\ \	]*) ;;
  *) CFLAGS="$CFLAGS -Wall" ;;
  esac
fi
changequote([,])dnl



dnl Check for WXWidgets
WX_VER_291=""

AC_PATH_PROGS(WX_CONFIG, [wx-config-2.9 wx-config], no)

if test x$WX_CONFIG != xno; then
   WX_VERSION=`$WX_CONFIG --version`

   WX_MAJOR=${WX_VERSION%%.*};
   WX_MINOR=${WX_VERSION#*.};
   WX_MINOR=${WX_MINOR%%.*};
   WX_MICRO=${WX_VERSION##*.};

   if test $WX_MAJOR -gt 2; then
      WX_VER_291=yes
   else
     if test $WX_MAJOR -eq 2; then
        if test $WX_MINOR -gt 9; then
           WX_VER_291=yes
        else
           if test $WX_MINOR -eq 9; then
              if test $WX_MICRO -ge 1; then
                 WX_VER_291=yes
              fi
           fi
        fi
     fi
  fi
fi

if test "$WX_VER_291" != "yes"; then

    dnl Use static version
    echo "installed version of wx is too old,"

    AC_MSG_ERROR([Please install development libraries for wxWidgets 2.9.1 or higher, and then run this configure script again.])
fi

WX_CXXFLAGS="`$WX_CONFIG --cxxflags`"
WX_LIBS="-static `$WX_CONFIG --libs` `$WX_CONFIG --libs richtext`"

echo "WX_CXXFLAGS is $WX_CXXFLAGS"
echo "WX_LIBS is $WX_LIBS"

WX_LDADD="$WX_LIBS"

AC_SUBST(WX_CXXFLAGS)
AC_SUBST(WX_LDADD)



dnl Check for avcodec


PKG_CHECK_MODULES([AVCODEC],[libavcodec])

dnl Check for avcodec_decode_video2
AC_CHECK_LIB([avcodec],[avcodec_decode_video2],HAVE_AVC_VID2=true,HAVE_AVC_VID2=false)

AM_CONDITIONAL(HAVE_AVC_VID2,$HAVE_AVC_VID2)

AC_SUBST(AVCODEC_CFLAGS)
AC_SUBST(AVCODEC_LIBS)



dnl Check for avformat

PKG_CHECK_MODULES(AVFORMAT,libavformat)

AC_SUBST(AVFORMAT_CFLAGS)
AC_SUBST(AVFORMAT_LIBS)

AC_CHECK_LIB([avformat], [av_open_input_file], AC_DEFINE([USE_OLD_FFMPEG_API]), AC_DEFINE([USE_NEW_FFMPEG_API]))

dnl Check for swscale

PKG_CHECK_MODULES(SWSCALE,libswscale)

AC_SUBST(SWSCALE_CFLAGS)
AC_SUBST(SWSCALE_LIBS)


dnl Check for avutil

PKG_CHECK_MODULES(AVUTIL,libavutil)

AC_SUBST(AVUTIL_CFLAGS)
AC_SUBST(AVUTIL_LIBS)


dnl Check for lua

PKG_CHECK_MODULES([LUA], [lua5.1],, 
	[PKG_CHECK_MODULES([LUA], [lua >= 5.1])
])

AC_SUBST(LUA_CFLAGS)
AC_SUBST(LUA_LIBS)



dnl Check for glee

PKG_CHECK_MODULES([GLEE],[glee], [HAVE_GLEE=true] , [HAVE_GLEE=false] )
if test "$HAVE_GLEE" = "false"; then

   PKG_CHECK_MODULES([GLEE],[GLee], [HAVE_GLEE=true] , [HAVE_GLEE=false] )
   if test "$HAVE_GLEE" = "false"; then
      AC_MSG_CHECKING(for GLee.h)
      AC_CHECK_HEADERS(GLee.h GL/GLee.h)

      if test "x$ac_cv_header_GLee_h" = "xno"; then
      	 if test "xac_cv_header_GL_GLee_h" = "xno"; then
      	    AC_MSG_ERROR([neither GLee.h nor GL/GLee.h were found. Please make sure you have GLee.h installed in your include path.])
      	 fi
      	 dnl found GLee.h in GL subdir
      	 GLEE_CFLAGS="-I $includedir/GL -I/usr/include/GL"
      fi

      GLEE_LIBS="-lGLee"
      fi

fi
AC_SUBST(GLEE_CFLAGS)
AC_SUBST(GLEE_LIBS)


dnl Check for curl

PKG_CHECK_MODULES(CURL,libcurl)

AC_SUBST(CURL_CFLAGS)
AC_SUBST(CURL_LIBS)


dnl Check for libpng

PKG_CHECK_MODULES(PNG,libpng)

AC_SUBST(PNG_CFLAGS)
AC_SUBST(PNG_LIBS)



dnl Check for Xrender

PKG_CHECK_MODULES(XRENDER,xrender)

AC_SUBST(XRENDER_CFLAGS)
AC_SUBST(XRENDER_LIBS)


# Check for flam3-animate
AC_PATH_PROG(FLAM3_ANIMATE, flam3-animate, no)
if test x$FLAM3_ANIMATE = xno; then
   AC_MSG_ERROR([Cannot find flam3-animate, make sure it is installed and in your PATH])
fi


dnl Check for libgtop-2.0

PKG_CHECK_MODULES(LIBGTOP,libgtop-2.0)

AC_SUBST(LIBGTOP_CFLAGS)
AC_SUBST(LIBGTOP_LIBS)


dnl Checks for boost
AC_LANG_PUSH([C++])

dnl Check boost version

REQ_BOOST_VERSION=103900
AC_MSG_CHECKING(for libboost >= 1.39)

AC_RUN_IFELSE([
  AC_LANG_PROGRAM(
  [
   #include <boost/version.hpp>
   #include <fstream>
  ],
  [dnl
#if BOOST_VERSION >= $REQ_BOOST_VERSION
  std::ofstream ofs("conftest.out");
  ofs << BOOST_VERSION / 100000 << '.' << BOOST_VERSION / 100 % 1000;
#else
#  error Boost version is too old
#endif
  ])],
  [
    BOOST_VERSION=`cat conftest.out`
    AC_MSG_RESULT(yes)
    succeeded=yes
    found_system=yes
  ],
  [])

AC_LANG_POP([C++])


dnl Check result of boost version
if test "x$succeeded" != "xyes"; then
   AC_MSG_ERROR([libboost version 1.41 or higher is required to compile electricsheep.])
fi


AC_LANG_PUSH([C++])

dnl Check for boost/thread.hpp
AC_CHECK_HEADER([boost/thread.hpp],,
	[AC_MSG_ERROR([you must install libboost-thread to compile electricsheep.])
])

AC_CHECK_LIB([boost_thread],[main],,
	[AC_MSG_ERROR([you must install libboost-thread to compile electricsheep.])
])


dnl Check for boost/filesystem.hpp
AC_CHECK_HEADER([boost/filesystem.hpp],,
	[AC_MSG_ERROR([you must install libboost-filesystem to compile electricsheep.])
])

AC_CHECK_LIB([boost_filesystem],[main],,
	[AC_MSG_ERROR([you must install libboost-filesystem to compile electricsheep.])
])

dnl Check for boost/system/config.hpp
AC_CHECK_HEADER([boost/system/config.hpp],,
	[AC_MSG_ERROR([you must install libboost-system to compile electricsheep.])
])

AC_CHECK_LIB([boost_system],[main],,
	[AC_MSG_ERROR([you must install libboost-system to compile electricsheep.])
])

AC_LANG_POP([C++])



BOOST_LDADD="-lboost_system"

AC_SUBST(BOOST_LDADD)


dnl Check for libtinyxml
AC_CHECK_LIB([tinyxml],[main],,
	[AC_MSG_ERROR([you must install libtinyxml dev to compile electricsheep.])
])

dnl Check for libglut
AC_CHECK_LIB([glut],[glutMainLoop],,
	[AC_MSG_ERROR([you must install libglut dev to compile electricsheep.])
])


AC_CHECK_LIB([GL],[glXCreateNewContext],,
	[AC_MSG_ERROR([you must install libGL to compile electricsheep.])
])

AC_CHECK_LIB([rt],[clock_gettime],,
	[AC_MSG_ERROR([you must install librt to compile electricsheep.])
])



AC_OUTPUT([ 
Makefile
Client/Makefile
MSVC/SettingsGUI/Makefile
])
