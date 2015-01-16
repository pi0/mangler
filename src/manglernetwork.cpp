/*
 * vim: softtabstop=4 shiftwidth=4 cindent foldmethod=marker expandtab
 *
 * $LastChangedDate: 2011-06-27 23:50:10 +0430 (Mon, 27 Jun 2011) $
 * $Revision: 1139 $
 * $LastChangedBy: econnell $
 * $URL: http://svn.mangler.org/mangler/trunk/src/manglernetwork.cpp $
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

#include "mangler.h"
#include "manglernetwork.h"

#include "channeltree.h"
#include "manglersettings.h"
#include "manglercharset.h"

ManglerNetwork::ManglerNetwork(Glib::RefPtr<Gtk::Builder> builder) {
    this->builder = builder;
}

void
ManglerNetwork::connect(Glib::ustring hostname, Glib::ustring port, Glib::ustring username, Glib::ustring password, Glib::ustring phonetic) {/*{{{*/
    Glib::ustring server = hostname + ":" + port;
    gdk_threads_enter();
    builder->get_widget("connectButton", button);
    button->set_sensitive(false);
    gdk_threads_leave();
    v3_debuglevel(Mangler::config["lv3_debuglevel"].toULong());
    if (! v3_login((char *)server.c_str(), (char *)ustring_to_c(username).c_str(), (char *)password.c_str(), (char *)phonetic.c_str())) {
        gdk_threads_enter();
        button->set_label("gtk-disconnect");
        mangler->errorDialog(c_to_ustring(_v3_error(NULL)));
        mangler->wantDisconnect = true;
        mangler->onDisconnectHandler();
        builder->get_widget("statusbar", statusbar);
        statusbar->pop();
        statusbar->push("Not connected.");
        gdk_threads_leave();
        return;
    }
    gdk_threads_enter();
    //mangler->channelTree->expand_all();
    builder->get_widget("connectButton", button);
    button->set_label("gtk-disconnect");
    button->set_sensitive(true);
    //builder->get_widget("serverTabLabel", label);
    //label->set_label(server);
    builder->get_widget("serverSelectComboBox", combobox);
    combobox->set_sensitive(false);
    mangler->statusIcon->set(mangler->icons["tray_icon_red"]);
    gdk_threads_leave();
    do {
        _v3_net_message *msg;

        if ((msg = _v3_recv(V3_BLOCK)) == NULL) {
            //printf("recv() failed: %s\n", _v3_error(NULL));
            return;
        }
        switch (_v3_process_message(msg)) {
            case V3_MALFORMED:
                _v3_debug(V3_DEBUG_INFO, "received malformed packet");
                break;
            case V3_NOTIMPL:
                _v3_debug(V3_DEBUG_INFO, "packet type not implemented");
                break;
            case V3_OK:
                _v3_debug(V3_DEBUG_INFO, "packet processed");
                /*
                if (v3_queue_size() > 0) {
                    fprintf("stderr", "there's something to do\n");
                }
                */
                break;
        }
    } while (v3_is_loggedin());
}/*}}}*/

void
ManglerNetwork::disconnect(void) {
    v3_logout();
}

