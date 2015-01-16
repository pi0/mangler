/*
 * vim: softtabstop=4 shiftwidth=4 cindent foldmethod=marker expandtab
 *
 * $LastChangedDate: 2011-06-27 23:50:10 +0430 (Mon, 27 Jun 2011) $
 * $Revision: 1139 $
 * $LastChangedBy: econnell $
 * $URL: http://svn.mangler.org/mangler/trunk/src/manglerchat.h $
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

#ifndef _MANGLERCHAT_H
#define _MANGLERCHAT_H

class ManglerChat {
    public:
        ManglerChat(Glib::RefPtr<Gtk::Builder> builder);
        Glib::RefPtr<Gtk::Builder>          builder;

        class chatUserModelColumns : public Gtk::TreeModel::ColumnRecord {
            public:
                chatUserModelColumns() { add(id); add(name); add(channel);}
                Gtk::TreeModelColumn<uint32_t>              id;
                Gtk::TreeModelColumn<Glib::ustring>         name;
                Gtk::TreeModelColumn<uint32_t>              channel;
        };
        chatUserModelColumns                chatUserColumns;
        Glib::RefPtr<Gtk::ListStore>        chatUserTreeModel;
        Glib::RefPtr<Gtk::TreeModelFilter>  chatUserTreeModelFilter;
        Gtk::TreeView                       *chatUserListView;
        Gtk::TreeModel::iterator            chatUserIter;
        Gtk::TreeModel::Row                 chatUserRow;

        //Glib::RefPtr<Gtk::TreeSelection> chatUserSelection; // probably not needed for this

        Gtk::Window   *chatWindow;
        Gtk::Button   *button;
        Gtk::Entry    *chatMessage;
        Gtk::CheckButton   *checkbutton;
        Gtk::TextView *chatBox;

        std::map<uint32_t, Glib::ustring> histEntry;
        std::map<uint32_t, Glib::ustring>::iterator histIter;
        uint32_t      histCount;
        uint32_t      histPos;

        bool          isOpen;
        bool          isJoined;
        bool          isPerChannel;

        void chatTimestampCheckButton_toggled_cb(void);
        void chatWindow_show_cb(void);
        void chatWindow_hide_cb(void);
        void chatMessage_activate_cb(void);
        bool chatMessage_key_press_event_cb(GdkEventKey* event);
        void chatClear_clicked_cb(void);
        void chatClose_clicked_cb(void);
        void chatHide_clicked_cb(void);

        void addChatMessage(uint16_t user_id, Glib::ustring message);
        void addRconMessage(Glib::ustring message);
        void addMessage(Glib::ustring message);
        void updateUser(uint16_t user_id);
        void addUser(uint16_t user_id);
        void clear(void);
        void removeUser(uint16_t user_id);
        bool isUserInChat(uint16_t user_id);
        Glib::ustring nameFromId(uint16_t user_id);
        bool filterVisible(const Gtk::TreeIter& iter);

};

#endif

