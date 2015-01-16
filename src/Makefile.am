
ACLOCAL_AMFLAGS = -I m4

icondir = $(datadir)/pixmaps
desktopdir = $(datadir)/applications

manglerui:
	echo -n "char ManglerUI[] = " > manglerui.h
	cat mangler.ui  | sed 's/"/\\"/g' | sed 's/^/"/' | sed 's/$$/\\n"/' >> manglerui.h
	echo ";" >> manglerui.h

bin_PROGRAMS = mangler
icon_DATA = ../icons/mangler_logo.svg 
desktop_DATA = ../mangler.desktop
man_MANS = mangler.1

mangler_SOURCES = \
	manglerui.h \
	mangler.cpp mangler.h \
	channeltree.cpp channeltree.h \
	mangleraudio.cpp mangleraudio.h \
	manglerbackend.cpp manglerbackend.h \
	manglerpulse.cpp manglerpulse.h \
	mangleralsa.cpp mangleralsa.h \
	mangleross.cpp mangleross.h \
	manglernetwork.cpp manglernetwork.h \
	manglerserverlist.cpp manglerserverlist.h \
	manglersettings.cpp manglersettings.h \
	manglerconfig.cpp manglerconfig.h \
	manglerchat.cpp manglerchat.h \
	manglerprivchat.cpp manglerprivchat.h \
	manglercharset.cpp manglercharset.h \
	manglerintegration.cpp manglerintegration.h \
	mangleradmin.cpp mangleradmin.h \
	manglerrecorder.cpp manglerrecorder.h \
	manglerosd.cpp manglerosd.h \
	manglerg15.cpp manglerg15.h \
	inilib.cpp inilib.h

mangler_LDADD = @top_srcdir@/libventrilo3/libventrilo3.la $(gtkmm_LIBS) $(gthread_LIBS) $(pulse_LIBS) $(alsa_LIBS) $(dbus_LIBS) $(xi_LIBS) $(x11_LIBS)

INCLUDES = -I@top_srcdir@/libventrilo3/ -I@top_srcdir@/icons/ -I@top_srcdir@/sounds/ $(gtkmm_CFLAGS) $(gthread_CFLAGS) $(pulse_CFLAGS) $(alsa_CFLAGS) $(oss_CFLAGS) $(dbus_CFLAGS) $(xi_CFLAGS) $(x11_CFLAGS)

dev:
	cd ../libventrilo3 && make dev
	g++ -g -Wall -Wextra -o mangler -I../ -I../libventrilo3/ -I../icons/ -I../sounds/ -pthread \
		mangler.cpp  channeltree.cpp  mangleraudio.cpp \
		manglerbackend.cpp  manglerpulse.cpp  mangleralsa.cpp  mangleross.cpp \
		manglernetwork.cpp  manglerserverlist.cpp  manglersettings.cpp  manglerconfig.cpp \
		manglerchat.cpp  manglerprivchat.cpp  manglercharset.cpp  manglerintegration.cpp \
		mangleradmin.cpp  manglerrecorder.cpp  manglerosd.cpp  manglerg15.cpp \
		inilib.cpp ../libventrilo3/libventrilo3.a \
		$(pulse_LIBS) $(alsa_LIBS) $(speex_LIBS) $(opus_LIBS) $(LIBS) \
		`pkg-config --cflags --libs dbus-glib-1` \
		`pkg-config --cflags --libs gthread-2.0` \
		`pkg-config --cflags --libs gtkmm-2.4`

