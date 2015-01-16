/*
 * vim: softtabstop=4 shiftwidth=4 cindent foldmethod=marker expandtab
 *
 * $LastChangedDate: 2011-06-27 23:50:10 +0430 (Mon, 27 Jun 2011) $
 * $Revision: 1139 $
 * $LastChangedBy: econnell $
 * $URL: http://svn.mangler.org/mangler/trunk/src/manglerprivchat.h $
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

#ifndef _MANGLERPRIVCHAT_H
#define _MANGLERPRIVCHAT_H

class ManglerPrivChat {
    public:
        ManglerPrivChat(uint16_t userid);
        uint16_t remoteUserId;

        Glib::RefPtr<Gtk::Builder> builder;
        Gtk::Window   *chatWindow;
        Gtk::Button   *sendButton;
        Gtk::Button   *closeButton;
        Gtk::Entry    *chatMessage;
        Gtk::TextView *chatBox;

        void chatWindow_show_cb(void);
        void chatWindow_hide_cb(void);

        void addMessage(Glib::ustring message);
        void addChatMessage(uint16_t id, Glib::ustring message);
        void remoteClosed(void);
        void remoteAway(void);
        void remoteBack(void);
        void remoteReopened(void);
        void chatWindowSendChat_clicked_cb(void);
        void chatWindowCloseChat_clicked_cb(void);
        void clear(void);
        Glib::ustring nameFromId(uint16_t user_id);
};

#endif

