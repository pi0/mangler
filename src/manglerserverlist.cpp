/*
 * vim: softtabstop=4 shiftwidth=4 cindent foldmethod=marker expandtab
 *
 * $LastChangedDate: 2011-06-27 23:50:10 +0430 (Mon, 27 Jun 2011) $
 * $Revision: 1139 $
 * $LastChangedBy: econnell $
 * $URL: http://svn.mangler.org/mangler/trunk/src/manglerserverlist.cpp $
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
#include "manglerserverlist.h"
#include "manglersettings.h"
#include "manglercharset.h"

ManglerServerList::ManglerServerList(Glib::RefPtr<Gtk::Builder> builder) {
    serverListTreeModel = Gtk::ListStore::create(serverListColumns);
    builder->get_widget("serverListWindow", serverListWindow);
    serverListWindow->signal_hide().connect(sigc::mem_fun(this, &ManglerServerList::serverListWindow_hide_cb));

    builder->get_widget("serverListView", serverListView);
    serverListView->set_model(serverListTreeModel);
    serverListView->append_column("Name", serverListColumns.name);
    serverListView->get_column(0)->set_sort_column(serverListColumns.name);
    serverListView->append_column("Username", serverListColumns.username);
    serverListView->get_column(1)->set_sort_column(serverListColumns.username);
    serverListView->append_column("Hostname", serverListColumns.hostname);
    serverListView->get_column(2)->set_sort_column(serverListColumns.hostname);
    serverListView->append_column("Port", serverListColumns.port);
    serverListView->get_column(3)->set_sort_column(serverListColumns.port);
    serverListSelection = serverListView->get_selection();
    serverListSelection->signal_changed().connect(sigc::mem_fun(this, &ManglerServerList::serverListSelection_changed_cb));

    builder->get_widget("serverListAddButton", serverListServerAddButton);
    serverListServerAddButton->signal_clicked().connect(sigc::mem_fun(this, &ManglerServerList::serverListAddButton_clicked_cb));

    builder->get_widget("serverListRemoveButton", serverListServerRemoveButton);
    serverListServerRemoveButton->signal_clicked().connect(sigc::mem_fun(this, &ManglerServerList::serverListRemoveButton_clicked_cb));

    builder->get_widget("serverListCloneButton", serverListServerCloneButton);
    serverListServerCloneButton->signal_clicked().connect(sigc::mem_fun(this, &ManglerServerList::serverListCloneButton_clicked_cb));

    builder->get_widget("serverListCloseButton", button);
    button->signal_clicked().connect(sigc::mem_fun(this, &ManglerServerList::serverListCloseButton_clicked_cb));

    builder->get_widget("serverListServerUpdateButton", serverListServerUpdateButton);
    serverListServerUpdateButton->signal_clicked().connect(sigc::mem_fun(this, &ManglerServerList::serverListServerUpdateButton_clicked_cb));

    builder->get_widget("serverListServerTable",            serverListServerTable);
    builder->get_widget("serverListUserTable",              serverListUserTable);
    builder->get_widget("serverListOptionsTable",           serverListOptionsTable);

    builder->get_widget("serverListServerNameEntry",        serverListServerNameEntry);
    builder->get_widget("serverListHostnameEntry",          serverListHostnameEntry);
    builder->get_widget("serverListPortEntry",              serverListPortEntry);
    builder->get_widget("serverListDefaultChannelEntry",    serverListDefaultChannelEntry);
    builder->get_widget("serverListUsernameEntry",          serverListUsernameEntry);
    builder->get_widget("serverListPasswordEntry",          serverListPasswordEntry);
    builder->get_widget("serverListPhoneticEntry",          serverListPhoneticEntry);
    builder->get_widget("serverListCommentEntry",           serverListCommentEntry);

    // CheckButton fields
    builder->get_widget("serverListPageCheckButton",        serverListPageCheckButton);
    builder->get_widget("serverListUtUCheckButton",         serverListUtUCheckButton);
    builder->get_widget("serverListPrivateChatCheckButton", serverListPrivateChatCheckButton);
    builder->get_widget("serverListRecordCheckButton",      serverListRecordCheckButton);
    builder->get_widget("serverListPersistentConnectionCheckButton", serverListPersistentConnectionCheckButton);
    builder->get_widget("serverListPersistentCommentsCheckButton", serverListPersistentCommentsCheckButton);

    // Charset combobox
    builder->get_widget("serverListCharsetComboBox",        serverListCharsetComboBox);
    charsetTreeModel = Gtk::ListStore::create(charsetColumns);
    serverListCharsetComboBox->set_model(charsetTreeModel);
    serverListCharsetComboBox->set_text_column(charsetColumns.name);
    for (int ctr = 0; charsetslist[ctr] != NULL; ctr++) {
        Gtk::TreeModel::Row charsetRow = *(charsetTreeModel->append());
        charsetRow[charsetColumns.name] = charsetslist[ctr];
    }
    serverListCharsetComboBox->get_entry()->set_text("");

    editorName = "";
}

void ManglerServerList::serverListWindow_hide_cb(void) {
    clearEntries();
}

void ManglerServerList::serverListSelection_changed_cb(void) {
    Gtk::TreeModel::iterator iter = serverListSelection->get_selected();
    if (iter) {
        Gtk::TreeModel::Row row = *iter;
        editorName = Glib::locale_from_utf8(row[serverListColumns.name]);
        editRow(editorName);
        serverListServerAddButton->set_sensitive(true);
        serverListServerRemoveButton->set_sensitive(true);
        serverListServerCloneButton->set_sensitive(true);
    } else {
        serverListServerRemoveButton->set_sensitive(false);
        serverListServerCloneButton->set_sensitive(false);
    }
}

void ManglerServerList::serverListRemoveButton_clicked_cb(void) {
    Gtk::TreeModel::iterator iter = serverListSelection->get_selected();
    if (!iter) {
        return;
    }
    Gtk::TreeModel::Row row = *iter;
    Glib::ustring name = row[serverListColumns.name];
    Mangler::config.servers.erase(name);
    Mangler::config.servers.save();
    serverListTreeModel->erase(row);
    editorName = "";
    clearEntries();
    queue_resize();
}

void ManglerServerList::serverListCloneButton_clicked_cb(void) {
    Gtk::TreeModel::iterator curIter = serverListSelection->get_selected();
    if (!curIter) {
        return; // should never happen
    }
    serverListServerAddButton->set_sensitive(false);
    Glib::ustring server_name = (*curIter)[serverListColumns.name];
    editorName = "";
    editRow(server_name);
}

void ManglerServerList::clearEntries(void) {
    serverListSelection->unselect_all();
    serverListServerNameEntry->set_text("");
    serverListHostnameEntry->set_text("");
    serverListPortEntry->set_text("");
    serverListDefaultChannelEntry->set_text("");
    serverListUsernameEntry->set_text("");
    serverListPasswordEntry->set_text("");
    serverListPhoneticEntry->set_text("");
    serverListCommentEntry->set_text("");
    serverListPageCheckButton->set_active(false);
    serverListUtUCheckButton->set_active(false);
    serverListPrivateChatCheckButton->set_active(false);
    serverListRecordCheckButton->set_active(false);
    serverListPersistentConnectionCheckButton->set_active(false);
    serverListPersistentCommentsCheckButton->set_active(false);
    serverListCharsetComboBox->get_entry()->set_text("");
    serverListServerTable->set_sensitive(false);
    serverListUserTable->set_sensitive(false);
    serverListOptionsTable->set_sensitive(false);
    serverListServerUpdateButton->set_sensitive(false);
    serverListServerAddButton->set_sensitive(true);
    serverListServerRemoveButton->set_sensitive(false);
    serverListServerCloneButton->set_sensitive(false);
}

void ManglerServerList::queue_resize(void) {
    for (int ctr = 0, cnt = serverListView->get_columns().size(); ctr < cnt; ctr++) {
        serverListView->get_column(ctr)->queue_resize();
    }
}

void ManglerServerList::serverListAddButton_clicked_cb(void) {
    serverListServerAddButton->set_sensitive(false);
    editorName = "";
    editRow("");
}

void ManglerServerList::serverListCloseButton_clicked_cb(void) {
    serverListWindow->hide();
}

void ManglerServerList::serverListServerUpdateButton_clicked_cb(void) {
    saveRow();
}

void ManglerServerList::editRow(const std::string &name) {
    if (editorName.empty()) {
        serverListSelection->unselect_all();
    }
    serverListServerTable->set_sensitive(true);
    serverListUserTable->set_sensitive(true);
    serverListOptionsTable->set_sensitive(true);
    serverListServerUpdateButton->set_sensitive(true);
    serverListServerCloneButton->set_sensitive(! editorName.empty());
    serverListServerRemoveButton->set_sensitive(! editorName.empty());
    serverListServerNameEntry->grab_focus();

    iniSection server;
    if (name.length()) {
        server = Mangler::config.servers[name];
    }
    serverListServerNameEntry->set_text(name);
    serverListHostnameEntry->set_text(server["Hostname"].toUString());
    serverListPortEntry->set_text(server["Port"].toUString());
    serverListDefaultChannelEntry->set_text(server["DefaultChannel"].toUString());
    serverListUsernameEntry->set_text(server["Username"].toUString());
    serverListPasswordEntry->set_text(server["Password"].toUString());
    serverListPhoneticEntry->set_text(server["Phonetic"].toUString());
    serverListCommentEntry->set_text(server["Comment"].toUString());
    serverListPageCheckButton->set_active(
            server["AcceptPages"].length() ?
            server["AcceptPages"].toBool() : true);
    serverListUtUCheckButton->set_active(
            server["AcceptU2U"].length() ?
            server["AcceptU2U"].toBool() : true);
    serverListPrivateChatCheckButton->set_active(
            server["AcceptPrivateChat"].length() ?
            server["AcceptPrivateChat"].toBool() : true);
    serverListRecordCheckButton->set_active(
            server["AllowRecording"].length() ?
            server["AllowRecording"].toBool() : true);
    serverListPersistentConnectionCheckButton->set_active(
            server["PersistentConnection"].length() ?
            server["PersistentConnection"].toBool() : true);
    serverListPersistentCommentsCheckButton->set_active(
            server["PersistentComments"].length() ?
            server["PersistentComments"].toBool() : true);
    std::string server_charset = server["Charset"].toString();
    if (server_charset.empty()) {
        server_charset = charsetslist[0];
    }
    serverListCharsetComboBox->get_entry()->set_text(server_charset);
}

void ManglerServerList::saveRow() {
    Glib::ustring charset;
    Gtk::TreeModel::iterator curIter = serverListSelection->get_selected();
    if (editorName.length() && !curIter) {
        return; // should never happen
    }
    Glib::ustring server_name = trim(serverListServerNameEntry->get_text());
    if (server_name.empty()) {
        mangler->errorDialog("Cannot save server without a name.");
        return;
    }
    // check for duplicate
    Gtk::TreeModel::Children::iterator ckIter = serverListTreeModel->children().begin();
    while (ckIter != serverListTreeModel->children().end()) {
        if ((editorName.empty() || ckIter != curIter) && (*ckIter)[serverListColumns.name] == server_name) {
            mangler->errorDialog("Server names must be unique.");
            return;
        }
        ckIter++;
    }
    // if name changed, remove old section first
    if (editorName.length() && server_name != editorName) {
        Mangler::config.servers.erase(editorName);
    }

    // save to config
    iniSection &server( Mangler::config.servers[server_name] );
    server["Hostname"]              = trim(serverListHostnameEntry->get_text());
    server["Port"]                  = trim(serverListPortEntry->get_text());
    server["DefaultChannel"]        = trim(serverListDefaultChannelEntry->get_text());
    server["Username"]              = trim(serverListUsernameEntry->get_text());
    server["Password"]              = trim(serverListPasswordEntry->get_text());
    server["Phonetic"]              = trim(serverListPhoneticEntry->get_text());
    server["Comment"]               = trim(serverListCommentEntry->get_text());
    server["AcceptPages"]           = serverListPageCheckButton->get_active();
    server["AcceptU2U"]             = serverListUtUCheckButton->get_active();
    server["AcceptPrivateChat"]     = serverListPrivateChatCheckButton->get_active();
    server["AllowRecording"]        = serverListRecordCheckButton->get_active();
    server["PersistentConnection"]  = serverListPersistentConnectionCheckButton->get_active();
    server["PersistentComments"]    = serverListPersistentCommentsCheckButton->get_active();
    server["Charset"]               = serverListCharsetComboBox->get_active_text();

    Gtk::TreeModel::Row row = (editorName.empty()) ? *(serverListTreeModel->append()) : *curIter;
    row[serverListColumns.name]     = server_name;
    row[serverListColumns.hostname] = server["Hostname"].toUString();
    row[serverListColumns.port]     = server["Port"].toUString();
    row[serverListColumns.username] = server["Username"].toUString();
    queue_resize();
    if (editorName.empty()) {
        serverListView->set_cursor(serverListTreeModel->get_path(row));
    }
    editorName = server_name;
    Mangler::config.servers.save();
}

Glib::ustring ManglerServerList::trim(Glib::ustring const& orig) {
    char const blankChars[] = " \t\n\r";

    Glib::ustring::size_type const first = orig.find_first_not_of(blankChars);
    return ( first==Glib::ustring::npos )
        ? Glib::ustring()
        : orig.substr(first, orig.find_last_not_of(blankChars)-first+1);
}

