AC_INIT([xbmc-peripheral-addons], 1:0:0)

AC_CANONICAL_HOST
AC_CANONICAL_TARGET

AM_INIT_AUTOMAKE(AC_PACKAGE_NAME, AC_PACKAGE_VERSION, [http://kodi.tv])
AM_INIT_AUTOMAKE([subdir-objects])

m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([no])])

PREFIX_DEFAULT="/usr/local"
AC_PREFIX_DEFAULT($PREFIX_DEFAULT)

AC_PROG_CXX
AC_PROG_CPP
AC_PROG_LIBTOOL
LT_INIT

AC_SEARCH_LIBS([pthread_create],[pthread],, AC_MSG_ERROR([required library 'pthread' is missing]))

AC_ARG_ENABLE([release],
  [AS_HELP_STRING([--enable-release],
  [build release binaries (default is no)])],
  [use_release=$enableval],
  [use_release=no])

### External libraries options

BUILD_TYPE="debug"
if test "$use_release" = "yes"; then
  BUILD_TYPE="release"
fi

## search for pthread, required by all targets
AC_SEARCH_LIBS([pthread_create],[pthread],,AC_MSG_ERROR($msg_pthread_missing))
AC_CHECK_FUNCS([pthread_mutexattr_init pthread_cond_init pthread_cond_destroy pthread_cond_signal pthread_cond_broadcast pthread_cond_wait pthread_cond_timedwait])

## search for dlopen, required by all targets
AC_SEARCH_LIBS([dlopen], [dl],
  [test "$ac_cv_search_dlopen" = "none required" || LIBS_DL=$ac_cv_search_dlopen],
  AC_MSG_ERROR($msg_dl_missing))
AC_CHECK_FUNCS([dlopen dlclose dlsym])
AC_SUBST(LIBS_DL)

AC_CHECK_PROG(HAVE_PKG_CONFIG, pkg-config, yes)
if test "x$HAVE_PKG_CONFIG" != "xyes" ; then
  AC_MSG_WARN([pkg-config was not found on your system])
fi

### External libraries checks

OS="unknown"
ARCHITECTURE="unknown"
HOST_INCLUDES=""
HOST_CXXFLAGS=""
ARCH_DEFINES="-DTARGET_POSIX -DTARGET_LINUX -D_LINUX"
DYN_LIB_EXT="so"
TARGET_LDFLAGS=""
BIN_EXT=".so"
BINPREFIX=""
case "${host}" in
  arm-*-linux-androideabi)
    TARGET_LDFLAGS="-avoid-version -no-undefined"
    OS="android"
    ARCHITECTURE="arm"
    BINPREFIX="lib"
    ;;
  arm*-*-linux*)
    OS="linux"
    ARCHITECTURE="arm"
    ;;
  powerpc-*-linux*)
    OS="linux"
    ARCHITECTURE="powerpc"
    ;;
  powerpc64-*-linux*)
    OS="linux"
    ARCHITECTURE="powerpc64"
    ;;
  mipsel-*-linux*)
    OS="linux"
    ARCHITECTURE="mipsel"
    ;;
  x86_64-*-linux*)
    OS="linux"
    ARCHITECTURE="x86_64"
    ;;
  *-*-linux*)
    OS="linux"
    ARCHITECTURE="i486"
    ;;
  arm-apple-darwin*)
    OS="ios"
    HOST_CXXFLAGS="-dynamiclib -single_module -undefined dynamic_lookup"
    host_os_osx="yes"
    ARCH_DEFINES="-DTARGET_POSIX -DTARGET_DARWIN -D_LINUX"
    DYN_LIB_EXT="dylib"
    ;;
  x86_64-apple-darwin*)
    OS="osx64"
    HOST_CXXFLAGS="-dynamiclib -single_module -undefined dynamic_lookup"
    host_os_osx="yes"
    ARCH_DEFINES="-DTARGET_POSIX -DTARGET_DARWIN -D_LINUX"
    DYN_LIB_EXT="dylib"
    ;;
  *-apple-darwin*)
    OS="osx32"
    HOST_CXXFLAGS="-dynamiclib -single_module -undefined dynamic_lookup"
    host_os_osx="yes"
    ARCH_DEFINES="-DTARGET_POSIX -DTARGET_DARWIN -D_LINUX"
    DYN_LIB_EXT="dylib"
    ;;
  *-freebsd*)
    OS="freebsd"
    ARCH_DEFINES="-DTARGET_POSIX -DTARGET_FREEBSD -D_LINUX"
    ;;
esac

### External libraries checks

### End external Libraries 

### Check for Intree building
if test "x${cross_compiling}" = "xyes" || test "x${cross_compiling}" = "xmaybe"; then
  # don't call AC_CHECK_FILE when (maybe) cross-compiling
  AM_CONDITIONAL(IS_INTREE_BUILD, false)
  intree=false
else
  checkpath=".."
  AC_CHECK_FILE([$checkpath/xbmc/xbmc.h], [AM_CONDITIONAL(IS_INTREE_BUILD, true) intree=true], [AM_CONDITIONAL(IS_INTREE_BUILD, false) intree=false])
  echo "Intree build: $intree"
fi

# Check for intrinsics
AC_MSG_CHECKING([for __sync_add_and_fetch(temp, 1)])
AC_TRY_COMPILE([],[long* temp=0; __sync_add_and_fetch(temp, 1);],
                [have_builtin_sync_add_and_fetch=yes],
                [have_builtin_sync_add_and_fetch=no])
AC_MSG_RESULT($have_builtin_sync_add_and_fetch)
if test "x$have_builtin_sync_add_and_fetch" = "xyes"; then
    AC_DEFINE(HAS_BUILTIN_SYNC_ADD_AND_FETCH, 1,
        [Define to 1 if your compiler supports the __sync_add_and_fetch() intrinsic.])
fi

AC_MSG_CHECKING([for __sync_sub_and_fetch(temp, 1)])
AC_TRY_COMPILE([],[long* temp=0; __sync_sub_and_fetch(temp, 1);],
                [have_builtin_sync_sub_and_fetch=yes],
                [have_builtin_sync_sub_and_fetch=no])
AC_MSG_RESULT($have_builtin_sync_sub_and_fetch)
if test "x$have_builtin_sync_sub_and_fetch" = "xyes"; then
    AC_DEFINE(HAS_BUILTIN_SYNC_SUB_AND_FETCH, 1,
        [Define to 1 if your compiler supports the __sync_sub_and_fetch() intrinsic.])
fi

AC_MSG_CHECKING([for __sync_val_compare_and_swap(temp, 1, 1)])
AC_TRY_COMPILE([],[long *temp = 0; __sync_val_compare_and_swap(temp, 1, 1);],
                [have_builtin_sync_val_compare_and_swap=yes],
                [have_builtin_sync_val_compare_and_swap=no])
AC_MSG_RESULT($have_builtin_sync_val_compare_and_swap)
if test "x$have_builtin_sync_val_compare_and_swap" = "xyes"; then
    AC_DEFINE(HAS_BUILTIN_SYNC_VAL_COMPARE_AND_SWAP, 1,
        [Define to 1 if your compiler supports the __sync_val_compare_and_swap() intrinsic.])
fi



HOST_CXXFLAGS="-Wall -Wextra -Wno-missing-field-initializers -Woverloaded-virtual -Wno-parentheses -fPIC $HOST_CXXFLAGS"

AC_SUBST(ARCHITECTURE)
AC_SUBST(BUILD_TYPE)
AC_SUBST(HOST_CXXFLAGS)
AC_SUBST(HOST_INCLUDES)
AC_SUBST(OS)
AC_SUBST(ARCH_DEFINES)
AC_SUBST(DYN_LIB_EXT)
AC_SUBST(TARGET_LDFLAGS)
AC_SUBST(BINPREFIX)
AC_SUBST(BIN_EXT)

if test "x$host_os_osx" = "xyes"; then
  AM_CONDITIONAL(HOST_IS_OSX, true)
else
  AM_CONDITIONAL(HOST_IS_OSX, false)
fi

if test "x${libdir}" != 'x${exec_prefix}/lib'; then
  LIBDIR=${libdir}
elif test "$prefix" = "NONE"; then
  LIBDIR=$PREFIX_DEFAULT/lib/kodi/addons
else
  LIBDIR=$prefix/lib/kodi/addons
fi
AC_SUBST(LIBDIR)

if test "x${datadir}" != 'x${datarootdir}' && test "x${datarootdir}" = 'x${prefix}/share'; then
  DATADIR=${datadir}
elif test "$prefix" = "NONE"; then
  DATADIR=$PREFIX_DEFAULT/share/kodi/addons
else
  DATADIR=$prefix/share/kodi/addons
fi
AC_SUBST(DATADIR)

AC_CONFIG_FILES([Makefile \
                 lib/Makefile \
                 addons/Makefile \
                 addons/peripheral.joystick/Makefile \
                 addons/peripheral.joystick/addon/addon.xml]) 
AC_OUTPUT
