/*
 * vim: softtabstop=4 shiftwidth=4 cindent foldmethod=marker expandtab
 *
 * $LastChangedDate: 2011-06-27 23:50:10 +0430 (Mon, 27 Jun 2011) $
 * $Revision: 1139 $
 * $LastChangedBy: econnell $
 * $URL: http://svn.mangler.org/mangler/trunk/src/manglerintegration.cpp $
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
// MPRIS INTERFACE: http://wiki.xmms2.xmms.se/wiki/MPRIS

#include "manglerintegration.h"
#include "config.h"
#include <iostream>
using namespace std;


bool ManglerIntegration::update(bool forceUpdate, const gchar *data)
{
    switch (client) {
#ifdef HAVE_LIBMPDCLIENT
        case MusicClient_MPD:
            {
                // Get out of idle mode
                mpd_idle idle_status = mpd_run_noidle (mpd_connection);
                // If there was no change of song, return false, and go back in idle mode
                if (idle_status != MPD_IDLE_PLAYER && !forceUpdate) {
                    mpd_send_idle (mpd_connection);
                    return false;
                }

                // Retrieve the status object
                struct mpd_status *status = mpd_run_status (mpd_connection);

                // Playback is stopped, or unknown
                mpd_state current_state = mpd_status_get_state (status);
                if (current_state == MPD_STATE_STOP || current_state == MPD_STATE_UNKNOWN) {
                    current_status = 0;
                }
                if (current_state == MPD_STATE_PAUSE) {
                    current_status = 1;
                }
                if (current_state == MPD_STATE_PLAY) {
                    current_status = 2;
                }

                // Get the currrent song
                int current_song_pos = mpd_status_get_song_pos (status);
                if (current_song_pos == -1) {
                    return false;
                }
                struct mpd_song *current_song = mpd_run_get_queue_song_pos (mpd_connection, current_song_pos);
                if (current_song == NULL) {
                    return false;
                }

                // Fill the strings
                artist = mpd_song_get_tag (current_song, MPD_TAG_ARTIST, 0);
                title = mpd_song_get_tag (current_song, MPD_TAG_TITLE, 0);
                album= mpd_song_get_tag (current_song, MPD_TAG_ALBUM, 0);

                // Free the memory for the status and song objects, and go back in idle mode
                mpd_status_free (status);
                mpd_song_free (current_song);
                mpd_send_idle (mpd_connection);
                return true;
            }
            break;
#endif
        default:
            break;
    }
    return false;
}

ManglerIntegration::ManglerIntegration ()
{
#ifdef HAVE_LIBMPDCLIENT
    mpd_connection = NULL;
#endif
}

void ManglerIntegration::setClient (MusicClient _client)
{
    client = _client;
    switch (client) {
        case MusicClient_None:
            break;
#ifdef HAVE_LIBMPDCLIENT
        case MusicClient_MPD:
            {
                mode = 0;
                const char *hostname = "localhost";
                int port = 6600;
                int timeout = -1;
                mpd_connection = mpd_connection_new (hostname, port, timeout);
                mpd_send_idle (mpd_connection);
            }
            break;


#endif
#ifdef HAVE_DBUS
        case MusicClient_Rhythmbox:
            {
                mode = 1;
                dbus_namespace = "org.gnome.Rhythmbox";
                dbus_player = "org.gnome.Rhythmbox.Player";
                dbus_player_path = "/org/gnome/Rhythmbox/Player";
                dbus_shell = "org.gnome.Rhythmbox.Shell";
                dbus_shell_path = "/org/gnome/Rhythmbox/Shell";
                dbus_uri_changed = "playingUriChanged";

                dbus_playing_changed = "playingChanged";
                dbus_get_current_song = "getPlayingUri";
                dbus_get_song_properties = "getSongProperties";

                bus = dbus_g_bus_get(DBUS_BUS_SESSION, NULL);
                player = dbus_g_proxy_new_for_name (bus, dbus_namespace.c_str(), dbus_player_path.c_str(), dbus_player.c_str());
                shell = dbus_g_proxy_new_for_name (bus, dbus_namespace.c_str(), dbus_shell_path.c_str(), dbus_shell.c_str());

                dbus_g_proxy_add_signal (player, dbus_uri_changed.c_str(), G_TYPE_STRING, G_TYPE_INVALID);
                dbus_g_proxy_connect_signal (player, dbus_uri_changed.c_str(), G_CALLBACK (dbus_uri_signal_callback), this, NULL);

                dbus_g_proxy_add_signal (player, dbus_playing_changed.c_str(), G_TYPE_BOOLEAN, G_TYPE_INVALID);
                dbus_g_proxy_connect_signal (player, dbus_playing_changed.c_str(), G_CALLBACK(dbus_playing_changed_callback), this, NULL);

                dbus_playing_changed_callback(NULL, FALSE, this);
            }
            break;

        case MusicClient_Amarok:
            mode = 1;
            dbus_namespace = "org.kde.amarok";
            dbus_player = dbus_shell = "org.gnome.Amarok.Player";
            dbus_player_path = dbus_shell_path = "/Player";
            dbus_uri_changed = "TrackChange";
            dbus_playing_changed = "StatusChange";

            dbus_get_current_song = dbus_get_song_properties = "GetMetadata";

            bus = dbus_g_bus_get(DBUS_BUS_SESSION, NULL);
            player = dbus_g_proxy_new_for_name (bus, dbus_namespace.c_str(), dbus_player_path.c_str(), dbus_player.c_str());
            shell = dbus_g_proxy_new_for_name (bus, dbus_namespace.c_str(), dbus_shell_path.c_str(), dbus_shell.c_str());

            dbus_g_proxy_add_signal (player, dbus_uri_changed.c_str(), G_TYPE_STRING, G_TYPE_INVALID);
            dbus_g_proxy_connect_signal (player, dbus_uri_changed.c_str(), G_CALLBACK (dbus_uri_signal_callback), this, NULL);

            dbus_g_proxy_add_signal (player, dbus_playing_changed.c_str(), G_TYPE_BOOLEAN, G_TYPE_INVALID);
            dbus_g_proxy_connect_signal (player, dbus_playing_changed.c_str(), G_CALLBACK(dbus_playing_changed_callback), this, NULL);

            dbus_playing_changed_callback(NULL, FALSE, this);

            break;
#endif
        default:
            break;
    }
    first_sent = false;
}

#ifdef HAVE_DBUS
gchar * ManglerIntegration::get_current_uri_dbus ()
{
    DBusConnection *conn;
    DBusMessage *msg, *reply;
    DBusMessageIter iter;
    gchar *uri;

    msg = dbus_message_new_method_call (dbus_namespace.c_str(), dbus_player_path.c_str(), dbus_player.c_str(), dbus_get_current_song.c_str());
    if (!msg) {
        return NULL;
    }

    dbus_message_set_auto_start (msg, FALSE);
    conn = (DBusConnection *) dbus_g_connection_get_connection (bus);
    reply = dbus_connection_send_with_reply_and_block(conn, msg, 5000, NULL);
    dbus_message_unref (msg);

    if (!reply) {
        return NULL;
    }

    if (dbus_message_get_type(reply) != DBUS_MESSAGE_TYPE_METHOD_RETURN) {
        dbus_message_unref (reply);
        return NULL;
    }

    dbus_message_iter_init(reply, &iter);
    if (dbus_message_iter_get_arg_type(&iter) != DBUS_TYPE_STRING) {
        dbus_message_unref(reply);
        return NULL;
    }
    dbus_message_iter_get_basic(&iter, &uri);
    dbus_message_unref(reply);
    return uri;
}

void dbus_playing_changed_callback (DBusGProxy *proxy, gboolean playing, gpointer data)
{
    ManglerIntegration *integration = (ManglerIntegration *) data;
    gchar *uri;

    if (!playing) {
        uri = integration->get_current_uri_dbus();
        if (uri && strlen(uri) != 0) {
            integration->set_current_status (1);
            dbus_uri_signal_callback(NULL, uri, integration);
        }
    } else {
        integration->set_current_status (2);
    }
}
void dbus_uri_signal_callback (DBusGProxy *proxy, const gchar *uri, gpointer data)
{
    ManglerIntegration *integration = (ManglerIntegration *) data;
    GValue *value;
    GHashTable *table = NULL;
    g_return_if_fail (uri != NULL);
    if (!dbus_g_proxy_call(integration->get_shell(), integration->dbus_get_song_properties.c_str(), NULL, G_TYPE_STRING, uri, G_TYPE_INVALID, dbus_g_type_get_map("GHashTable", G_TYPE_STRING, G_TYPE_VALUE), &table, G_TYPE_INVALID)) {
        return;
    }

    value = (GValue *) g_hash_table_lookup(table, "artist");
    if (value != NULL && G_VALUE_HOLDS_STRING(value)) {
        integration->artist = g_value_get_string(value);
    }
    value = (GValue *) g_hash_table_lookup(table, "album");
    if (value != NULL && G_VALUE_HOLDS_STRING(value)) {
        integration->album = g_value_get_string(value);
    }
    value = (GValue *) g_hash_table_lookup(table, "title");
    if (value != NULL && G_VALUE_HOLDS_STRING(value)) {
        integration->title = g_value_get_string(value);
    }
    g_hash_table_destroy(table);
    return;
}
#endif

Glib::ustring ManglerIntegration::format () const
{
    std::ostringstream os;
    int _state = (int) current_status;
    Glib::ustring formatted_text ("");
    if (_state == 0 || _state == 1) {
        return formatted_text;
    }
    //        os <<  "Stopped" << ": ";
    //    if (_state == 1)
    //        os <<  "Paused" << ": ";
    os << artist << " - " << title;
    formatted_text = os.str();
    return formatted_text;
}

ManglerIntegration::~ManglerIntegration()
{
#ifdef HAVE_LIBMPDCLIENT
    if (mpd_connection != NULL) {
        mpd_connection_free (mpd_connection);
    }
#endif
}

bool ManglerIntegration::first ()
{
    if (!first_sent) {
        first_sent = true;
        return false;
    }
    return true;
}
