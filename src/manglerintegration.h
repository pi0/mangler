/*
 * vim: softtabstop=4 shiftwidth=4 cindent foldmethod=marker expandtab
 *
 * $LastChangedDate: 2011-06-27 23:50:10 +0430 (Mon, 27 Jun 2011) $
 * $Revision: 1139 $
 * $LastChangedBy: econnell $
 * $URL: http://svn.mangler.org/mangler/trunk/src/manglerintegration.h $
 *
 * Copyright 2009-2011 Eric Connell
 *
 * This file is part of Mangler.
 *
 * Mangler is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Mangler is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Mangler.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _MANGLERINTEGRATION_H
#define _MANGLERINTEGRATION_H

#include "config.h"
#ifdef HAVE_LIBMPDCLIENT
# include "mpd/client.h"
#endif

#include <glib.h>
#include <gtkmm.h>
#ifdef HAVE_DBUS
# include <dbus/dbus.h>
# include <dbus/dbus-glib.h>
# include <dbus/dbus-glib-lowlevel.h>
#endif
#include <string.h>
#include <string>

enum MusicClient
{
    MusicClient_None = 0,
#ifdef HAVE_LIBMPDCLIENT
    MusicClient_MPD,
#endif
#ifdef HAVE_DBUS
    MusicClient_Rhythmbox,
    MusicClient_Amarok,
#endif
};


class ManglerIntegration{
    public:
        ~ManglerIntegration();
        ManglerIntegration();
        bool update (bool forceUpdate, const gchar *data = NULL);
        bool first();
        void setClient (MusicClient);
        Glib::ustring format() const;
        bool first_sent;
        Glib::ustring artist;
        Glib::ustring album;
        Glib::ustring title;
        int get_mode() const { return mode; };
#ifdef HAVE_DBUS
        DBusGProxy * get_player() { return player; };
        DBusGProxy * get_shell() { return shell; };
#endif
#ifdef HAVE_LIBMPDCLIENT
        struct mpd_connection *mpd_connection;
#endif
        gchar * get_current_uri_dbus();

        void set_current_status (int _status) { current_status = _status; };

        Glib::ustring dbus_namespace;

        Glib::ustring dbus_player;
        Glib::ustring dbus_player_path;
        Glib::ustring dbus_shell;
        Glib::ustring dbus_shell_path;

        Glib::ustring dbus_uri_changed;
        Glib::ustring dbus_playing_changed;

        Glib::ustring dbus_get_current_song;
        Glib::ustring dbus_get_song_properties;

        MusicClient client;

    private:
        // 0 = polling, 1 = listen
        int mode;

        int current_status;

        // DBus stuff
#ifdef HAVE_DBUS
        DBusGConnection *bus;
        DBusGProxy *player;
        DBusGProxy *shell;
#endif



};

#ifdef HAVE_DBUS
void dbus_uri_signal_callback (DBusGProxy *, const gchar *, gpointer);
void dbus_playing_changed_callback (DBusGProxy *, gboolean, gpointer);
#endif

#endif

