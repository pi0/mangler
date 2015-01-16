#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.68])
AC_INIT([mangler],[1.2.5],[eric@ypass.net])
AM_INIT_AUTOMAKE
AM_MAINTAINER_MODE([disable])

AC_CONFIG_SRCDIR([config.h.in])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_MACRO_DIR([m4])

# Checks for programs.
AC_PROG_CXX
AC_PROG_AWK
AC_PROG_CC
AC_PROG_CPP
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET
AC_PROG_MKDIR_P
LT_INIT

AX_PTHREAD([
LIBS="$PTHREAD_LIBS $LIBS"
CFLAGS="$CFLAGS $PTHREAD_CFLAGS"
CC="$PTHREAD_CC"
], [echo "error: cannot determine how to build threaded applications"; exit 1])
AM_PATH_GLIB_2_0([], [], [], [gthread])
AC_CHECK_HEADERS([arpa/inet.h fcntl.h netdb.h netinet/in.h stdint.h stdlib.h string.h sys/socket.h sys/time.h unistd.h])

AC_CHECK_LIB([m], [tan], [:], [
	echo "error: cannot find a working math library"
	exit -1
	])
#AC_CHECK_HEADER([mpd/client.h],
#	[AC_CHECK_LIB([mpdclient], [mpd_run_status])]
#)

AC_ARG_ENABLE(gsm,    [  --disable-gsm           disable GSM codec support], [disable_gsm=yes])
AC_ARG_ENABLE(speex,  [  --disable-speex         disable Speex codec support], [disable_speex=yes])
AC_ARG_ENABLE(opus,   [  --disable-opus          disable Opus codec support], [disable_opus=yes])
AC_ARG_ENABLE(xosd,   [  --disable-xosd          disable XOSD overlay support], [disable_xosd=yes])
AC_ARG_ENABLE(g15,    [  --disable-g15           disable Logitech G15 keyboard support], [disable_g15=yes])
AC_ARG_ENABLE(espeak, [  --disable-espeak        disable eSpeak text to speech support], [disable_espeak=yes])
AC_ARG_ENABLE(dbus,   [  --disable-dbus          disable dbus support for rhythmbox/amarok], [disable_dbus=yes])
AC_ARG_ENABLE(ui,     [  --disable-ui            disable building of the UI (libventrilo3 only)], [disable_ui=yes])

if test "$enable_gsm" != "no" ; then
	AC_CHECK_LIB([gsm], [gsm_create], [], [
		echo "error: cannot find libgsm: Is it installed? Also make sure you have the"
		echo "       libgsm development packages installed."
		exit -1
		])
	AC_CHECK_HEADER([gsm.h], [AC_DEFINE(HAVE_GSM_H, 1, [location of gsm.h])], [
		AC_CHECK_HEADER([gsm/gsm.h], AC_DEFINE(HAVE_GSM_GSM_H, 1, [location of gsm.h]),[
			echo "error: found gsm libraries, but headers are missing or corrupt. are you"
			echo "       missing a libgsm development package?"
			exit -1
			])
		])
	AC_DEFINE(HAVE_GSM, 1, [have gsm])
else
	AC_DEFINE(HAVE_GSM, 0, [have gsm])
fi


# Checks for header files.
if test "$enable_speex" != "no" ; then
	PKG_CHECK_MODULES(speex, speex >= 1.2)
	AC_SUBST(speex_CFLAGS)
	AC_SUBST(speex_LIBS)
	AC_DEFINE(HAVE_SPEEX, 1, [have speex])
else
	AC_DEFINE(HAVE_SPEEX, 0, [have speex])
fi

if test "$enable_opus" != "no" ; then
	PKG_CHECK_MODULES(opus, opus >= 0.9.11, [
		AC_DEFINE(HAVE_OPUS, 1, [have opus])
		AC_SUBST(opus_CFLAGS)
		AC_SUBST(opus_LIBS)
		echo "Enabled optional Opus codec support."
	], [
		AC_DEFINE(HAVE_OPUS, 0, [have opus])
		echo "Optional Opus codec support is not enabled."
	])
else
	echo "Disabled optional Opus codec support."
fi

if test "$disable_ui" != "yes" ; then
	if test "$enable_xosd" != "no" ; then
		AC_CHECK_LIB([xosd], [xosd_create], [
			AC_DEFINE(HAVE_XOSD, 1, [have xosd])
			LIBS="-lxosd $LIBS"
			echo "Enabled optional XOSD support."
		], [
			echo "Optional XOSD support is not enabled."
		])
	else
		echo "Disabled optional XOSD support."
	fi

	if test "$enable_g15" != "no" ; then
		AC_CHECK_LIB([g15daemon_client], [new_g15_screen], [
			AC_CHECK_LIB([g15render], [g15r_clearScreen], [
				AC_DEFINE(HAVE_G15, 1, [have g15 kb support])
				LIBS="-lg15daemon_client -lg15render $LIBS"
				echo "Enabled optional Logitech G15 support."
			])
		], [
			echo "Optional Logitech G15 support is not enabled."
		])
	else
		echo "Disabled optional Logitech G15 support."
	fi

	if test "$enable_espeak" != "no" ; then
		AC_CHECK_LIB([espeak], [espeak_Initialize], [
			AC_DEFINE(HAVE_ESPEAK, 1, [have espeak])
			LIBS="-lespeak $LIBS"
			echo "Enabled optional eSpeak support."
		], [
			echo "Optional eSpeak support is not enabled."
		])
	else
		echo "Disabled optional eSpeak support."
	fi

	PKG_CHECK_MODULES(x11, x11) 
	AC_SUBST(x11_CFLAGS) 
	AC_SUBST(x11_LIBS) 

	PKG_CHECK_MODULES(xi, xi) 
	AC_SUBST(xi_CFLAGS) 
	AC_SUBST(xi_LIBS) 

	PKG_CHECK_MODULES(gtk, gtk+-2.0 >= 2.16.0)

	if test "$enable_dbus" != "no" ; then
		PKG_CHECK_MODULES(dbus, dbus-glib-1 >= 0.80, [AC_DEFINE(HAVE_DBUS, 1, [have dbus])], [:])
		AC_SUBST(dbus_CFLAGS)
		AC_SUBST(dbus_LIBS)
	fi

	PKG_CHECK_MODULES(gtkmm, gtkmm-2.4 >= 1.3.3)
	AC_SUBST(gtkmm_CFLAGS)
	AC_SUBST(gtkmm_LIBS)

	AC_ARG_WITH([pulseaudio],
		[AS_HELP_STRING([--without-pulseaudio], [disable support for PulseAudio])],
		[:],
		[with_pulseaudio=yes])

	AC_ARG_WITH([alsa],
		[AS_HELP_STRING([--without-alsa], [disable support for ALSA])],
		[:],
		[with_alsa=yes])

	AC_ARG_WITH([oss],
		[AS_HELP_STRING([--without-oss], [disable support for OSS])],
		[:],
		[with_oss=yes])

	AS_IF(
		[test "x$with_pulseaudio" != xno],
		[
			PKG_CHECK_MODULES(pulse, libpulse-simple >= 0.9.14, [
				AC_DEFINE(HAVE_PULSE, 1, [have pulse])
				AC_SUBST(pulse_CFLAGS)
				AC_SUBST(pulse_LIBS)
			], [
				echo "warning: PulseAudio development libraries or headers were not found."
				echo "         PulseAudio support is disabled."
			])
		],[:]
	)
	AS_IF(
		[test "x$with_alsa" != xno],
		[
			PKG_CHECK_MODULES(alsa, alsa >= 1.0.0, [
				AC_DEFINE(HAVE_ALSA, 1, [have alsa])
				AC_SUBST(alsa_CFLAGS)
				AC_SUBST(alsa_LIBS)
			], [
				echo "warning: ALSA development libraries or headers were not found."
				echo "         ALSA support is disabled."
			])
			
		],[:]
	)
	AS_IF(
		[test "x$with_oss" != xno],
		[
			AS_IF(
				[test -r /usr/lib/oss/include/sys/soundcard.h],
				[
					echo "Found OSSv4 include file in /usr/lib/oss/include/sys/soundcard.h";
					oss_CFLAGS="-I/usr/lib/oss/include"
					AC_SUBST(oss_CFLAGS)
					# We can't perform any real checks on OSSv4 because their include file is in a retarded location.
					AC_DEFINE(HAVE_OSS, 1, [have oss])
				], [
					AC_CHECK_HEADER([sys/soundcard.h], [
						AC_DEFINE(HAVE_OSS, 1, [have oss])
					], [
						echo "warning: OSS required headers were not found."
						echo "         OSS support is disabled."
					])
				])
			
		],[:]
	)

	PKG_CHECK_MODULES(gthread, gthread-2.0 >= 2.20.1)
	AC_SUBST(gthread_CFLAGS)
	AC_SUBST(gthread_LIBS)

	PKG_CHECK_MODULES(gtkmm, gtkmm-2.4 >= 1.3.3)
	SUBDIR_LIST="libventrilo3 icons sounds src"
else
	SUBDIR_LIST="libventrilo3"
fi
AC_SUBST([SUBDIR_LIST])


# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_TYPE_INT16_T
AC_TYPE_INT64_T
AC_TYPE_SIZE_T
AC_TYPE_SSIZE_T
AC_TYPE_UINT16_T
AC_TYPE_UINT32_T
AC_TYPE_UINT8_T

# Checks for library functions.
AC_FUNC_MALLOC
AC_FUNC_REALLOC
AC_CHECK_FUNCS([dup2 gettimeofday inet_ntoa memset select socket strchr strdup strerror gethostbyname_r])

AC_CONFIG_FILES([src/Makefile
                 Makefile
                 libventrilo3/Makefile])

if test "x$with_pulseaudio" = "xno" -a "x$with_alsa" = "xno" -a "x$with_oss" = "xno" ; then
	echo "**************************************************************************"
	echo "*** WARNING: You have disabled support for PulseAudio, ALSA and OSS.   ***"
	echo "***          You can compile and use Mangler without an audio          ***"
	echo "***          subsystem but you will not have any sound, which more     ***"
	echo "***          or less makes the program useless.                        ***"
	echo "**************************************************************************"
fi

AC_OUTPUT
