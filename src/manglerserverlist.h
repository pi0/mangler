/*
 * vim: softtabstop=4 shiftwidth=4 cindent foldmethod=marker expandtab
 *
 * $LastChangedDate: 2011-06-27 23:50:10 +0430 (Mon, 27 Jun 2011) $
 * $Revision: 1139 $
 * $LastChangedBy: econnell $
 * $URL: http://svn.mangler.org/mangler/trunk/src/manglerserverlist.h $
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

#ifndef _MANGLERSERVERLIST_H
#define _MANGLERSERVERLIST_H

class ManglerServerList {
    public:
        ManglerServerList(Glib::RefPtr<Gtk::Builder> builder);
        Gtk::Window *serverListWindow;

        class serverListModelColumns : public Gtk::TreeModel::ColumnRecord {
            public:
                serverListModelColumns() { add(id); add(name); add(hostname); add(port); add(username); }
                Gtk::TreeModelColumn<uint32_t>              id;
                Gtk::TreeModelColumn<Glib::ustring>         name;
                Gtk::TreeModelColumn<Glib::ustring>         hostname;
                Gtk::TreeModelColumn<Glib::ustring>         port;
                Gtk::TreeModelColumn<Glib::ustring>         username;
        };
        serverListModelColumns       serverListColumns;
        Glib::RefPtr<Gtk::ListStore> serverListTreeModel;
        Gtk::TreeView                *serverListView;
        Glib::RefPtr<Gtk::TreeSelection> serverListSelection;

        // Table fields
        Gtk::Table *serverListServerTable;
        Gtk::Table *serverListUserTable;
        Gtk::Table *serverListOptionsTable;

        // Entry fields
        std::string editorName;
        int32_t     editorId;
        Gtk::Entry *serverListServerNameEntry;
        Gtk::Entry *serverListHostnameEntry;
        Gtk::Entry *serverListPortEntry;
        Gtk::Entry *serverListDefaultChannelEntry;
        Gtk::Entry *serverListUsernameEntry;
        Gtk::Entry *serverListPasswordEntry;
        Gtk::Entry *serverListPhoneticEntry;
        Gtk::Entry *serverListCommentEntry;

        // CheckButton fields
        Gtk::CheckButton *serverListPageCheckButton;
        Gtk::CheckButton *serverListUtUCheckButton;
        Gtk::CheckButton *serverListPrivateChatCheckButton;
        Gtk::CheckButton *serverListRecordCheckButton;
        Gtk::CheckButton *serverListPersistentConnectionCheckButton;
        Gtk::CheckButton *serverListPersistentCommentsCheckButton;

        // Character Set Combobox
        Gtk::ComboBoxEntry *serverListCharsetComboBox;
        class charsetModelColumns : public Gtk::TreeModel::ColumnRecord
        {
            public:
                charsetModelColumns() { add(name); }
                Gtk::TreeModelColumn<Glib::ustring> name;
        };
        charsetModelColumns  charsetColumns;
        Glib::RefPtr<Gtk::ListStore> charsetTreeModel;

        // Editor Buttons
        Gtk::Button *serverListServerUpdateButton;
        Gtk::Button *serverListServerAddButton;
        Gtk::Button *serverListServerRemoveButton;
        Gtk::Button *serverListServerCloneButton;

        void serverListWindow_hide_cb(void);
        void serverListSelection_changed_cb(void);
        void serverListAddButton_clicked_cb(void);
        void serverListRemoveButton_clicked_cb(void);
        void serverListCloneButton_clicked_cb(void);
        void serverListCloseButton_clicked_cb(void);
        void serverListServerUpdateButton_clicked_cb(void);

        void editRow(const std::string &name);
        void saveRow();
        void clearEntries(void);
        void queue_resize(void);

        Glib::ustring trim(Glib::ustring const& orig);

        // generic types for builder
        Gtk::Button     *button;
};

#endif

