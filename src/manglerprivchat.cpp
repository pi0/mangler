/*
 * vim: softtabstop=4 shiftwidth=4 cindent foldmethod=marker expandtab
 *
 * $LastChangedDate: 2012-05-24 22:31:26 +0430 (Thu, 24 May 2012) $
 * $Revision: 1159 $
 * $LastChangedBy: Haxar $
 * $URL: http://svn.mangler.org/mangler/trunk/src/manglerprivchat.cpp $
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
#include "manglerprivchat.h"
#include "manglercharset.h"

extern const char ManglerUI[];

ManglerPrivChat::ManglerPrivChat(uint16_t userid) {
    // We instantiate a new builder object here to get a completely new window (hopefully)
    builder = Gtk::Builder::create_from_string(ManglerUI, "privChatWindow");
    builder->get_widget("privChatWindow", chatWindow);
    chatWindow->signal_hide().connect(sigc::mem_fun(this, &ManglerPrivChat::chatWindowCloseChat_clicked_cb));
    chatWindow->set_title(chatWindow->get_title() + " - " + nameFromId(userid));
    this->remoteUserId = userid;

    builder->get_widget("privSendChat", sendButton);
    sendButton->signal_clicked().connect(sigc::mem_fun(this, &ManglerPrivChat::chatWindowSendChat_clicked_cb));

    builder->get_widget("privChatClose", closeButton);
    closeButton->signal_clicked().connect(sigc::mem_fun(this, &ManglerPrivChat::chatWindowCloseChat_clicked_cb));

    builder->get_widget("privChatBox", chatBox);
    builder->get_widget("privChatMessage", chatMessage);

    v3_start_privchat(remoteUserId);
    chatWindow->present();
}

void ManglerPrivChat::chatWindowSendChat_clicked_cb(void) {
    if (chatMessage->get_text_length()) {
        v3_send_privchat_message(remoteUserId, (char *)ustring_to_c(chatMessage->get_text()).c_str());
        chatMessage->set_text("");
    }
}

void ManglerPrivChat::chatWindowCloseChat_clicked_cb(void) {
    v3_end_privchat(remoteUserId);
    chatWindow->hide();
}

void ManglerPrivChat::addMessage(Glib::ustring message) {
    char timestamp[200];
    time_t t;
    struct tm *tmp;
    struct timeval tv;

    Glib::RefPtr<Gtk::TextBuffer> buffer = chatBox->get_buffer();
    if (Mangler::config["ChatTimestamps"].toBool()) {
        gettimeofday(&tv, NULL);
        t = tv.tv_sec;
        tmp = localtime(&t);
        if (strftime(timestamp, sizeof(timestamp), "%T", tmp) != 0) {
            message = "[" + Glib::ustring(timestamp) + "] " + message;
        }
    }
    buffer->insert(buffer->end(), message + "\n");
    chatBox->scroll_to(buffer->create_mark(buffer->end()), 0.0);
}

void ManglerPrivChat::remoteClosed() {
    addMessage("\n*** remote user closed connection");
    sendButton->set_sensitive(false);
}

void ManglerPrivChat::remoteAway() {
    addMessage("\n*** remote user is now away");
}

void ManglerPrivChat::remoteBack() {
    addMessage("\n*** remote user is back");
}

void ManglerPrivChat::remoteReopened() {
    addMessage("\n*** remote user has reopened chat window");
    sendButton->set_sensitive(true);
}

void ManglerPrivChat::addChatMessage(uint16_t id, Glib::ustring message) {
    addMessage("[" + nameFromId(id) + "]: " + message);
}

Glib::ustring ManglerPrivChat::nameFromId(uint16_t user_id) {
    v3_user *u = v3_get_user(user_id);
    Glib::ustring name = "";
    if (u) {
        name = c_to_ustring(u->name);
        v3_free_user(u);
    } else {
        name = "unknown";
    }
    return name;
}

