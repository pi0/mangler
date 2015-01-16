/*
 * vim: softtabstop=4 shiftwidth=4 cindent foldmethod=marker expandtab
 *
 * $LastChangedDate: 2011-06-27 23:50:10 +0430 (Mon, 27 Jun 2011) $
 * $Revision: 1139 $
 * $LastChangedBy: econnell $
 * $URL: http://svn.mangler.org/mangler/trunk/src/mangleradmin.cpp $
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
#include "mangleradmin.h"
#include "manglercharset.h"
#include "inilib.h"

#include <unistd.h>
#include <sys/stat.h>
#include <dirent.h>

int natsort(const char *l, const char *r);

Glib::ustring
ManglerAdmin::trimString(Glib::ustring s) {/*{{{*/
    if (s.empty()) {
        return s;
    }
    while (s.length() && (s[0] == ' ' || s[0] == '\t')) {
        s.erase(0, 1);
    }
    if (s.empty()) {
        return s;
    }
    for (int i = s.length() - 1; i >= 0; --i) {
        if (s[i] == '\n') {
            continue;
        }
        if (s[i] != ' ' && s[i] != '\t') {
            break;
        }
        s.erase(i, 1);
    }
    return s;
}/*}}}*/

ManglerAdmin::ManglerAdmin(Glib::RefPtr<Gtk::Builder> builder) {/*{{{*/
    /* set up the basic window variables */
    this->builder = builder;
    Gtk::TreeModel::Row row;
    Gtk::TreeView::Column *pColumn;
    Gtk::CellRendererText *renderer;

    builder->get_widget("adminWindow", adminWindow);
    adminWindow->signal_show().connect(sigc::mem_fun(this, &ManglerAdmin::adminWindow_show_cb));
    adminWindow->signal_hide().connect(sigc::mem_fun(this, &ManglerAdmin::adminWindow_hide_cb));
    builder->get_widget("adminNotebook", adminNotebook);
    builder->get_widget("CloseButton", button);
    button->signal_clicked().connect(sigc::mem_fun(this, &ManglerAdmin::CloseButton_clicked_cb));

    builder->get_widget("ServerTab", ServerTab);
    ServerTab->hide();
    builder->get_widget("ChannelsTab", ChannelsTab);
    builder->get_widget("UsersTab", UsersTab);
    builder->get_widget("RanksTab", RanksTab);
    builder->get_widget("BansTab", BansTab);
    builder->get_widget("AdminStatusbar", AdminStatusbar);
    AdminStatusbar->set_has_resize_grip(false);

    StatusbarTime = ::time(NULL);
    StatusbarCount = 0;

    Glib::signal_timeout().connect_seconds(sigc::mem_fun(this, &ManglerAdmin::statusbarPop), 1);

    /* set up the server settings editor stuff */
    builder->get_widget("ServerChatFilter", combobox);
    SrvChatFilterModel = Gtk::TreeStore::create(adminRecord);
    combobox->set_model(SrvChatFilterModel);
    combobox->pack_start(adminRecord.name);
    row = *(SrvChatFilterModel->append());
    row[adminRecord.id] = 0; row[adminRecord.name] = "Global to Server";
    row = *(SrvChatFilterModel->append());
    row[adminRecord.id] = 1; row[adminRecord.name] = "Per Channel";

    builder->get_widget("ServerChannelOrdering", combobox);
    SrvChanOrderModel = Gtk::TreeStore::create(adminRecord);
    combobox->set_model(SrvChanOrderModel);
    combobox->pack_start(adminRecord.name);
    row = *(SrvChanOrderModel->append());
    row[adminRecord.id] = 0; row[adminRecord.name] = "Sort Alphabetically";
    row = *(SrvChanOrderModel->append());
    row[adminRecord.id] = 1; row[adminRecord.name] = "Manual";

    builder->get_widget("ServerAction", combobox);
    SrvInactActionModel = Gtk::TreeStore::create(adminRecord);
    combobox->set_model(SrvInactActionModel);
    combobox->pack_start(adminRecord.name);
    row = *(SrvInactActionModel->append());
    row[adminRecord.id] = 0; row[adminRecord.name] = "Kick User";
    row = *(SrvInactActionModel->append());
    row[adminRecord.id] = 1; row[adminRecord.name] = "Move to Channel";

    builder->get_widget("ServerChannel", combobox);
    SrvInactChannelModel = Gtk::TreeStore::create(adminRecord);
    combobox->set_model(SrvInactChannelModel);
    combobox->pack_start(adminRecord.name);
    renderer = (Gtk::CellRendererText*)(*(combobox->get_cells().begin()));
    renderer->property_ellipsize() = Pango::ELLIPSIZE_MIDDLE;

    builder->get_widget("ServerSpamFilterChannelAction", combobox);
    SrvSpamFilterChannelModel = Gtk::TreeStore::create(adminRecord);
    combobox->set_model(SrvSpamFilterChannelModel);
    combobox->pack_start(adminRecord.name);
    row = *(SrvSpamFilterChannelModel->append());
    row[adminRecord.id] = 0; row[adminRecord.name] = "Do Nothing";
    row = *(SrvSpamFilterChannelModel->append());
    row[adminRecord.id] = 1; row[adminRecord.name] = "Kick User";
    row = *(SrvSpamFilterChannelModel->append());
    row[adminRecord.id] = 2; row[adminRecord.name] = "Ban User";

    builder->get_widget("ServerSpamFilterChatAction", combobox);
    SrvSpamFilterChatModel = Gtk::TreeStore::create(adminRecord);
    combobox->set_model(SrvSpamFilterChatModel);
    combobox->pack_start(adminRecord.name);
    row = *(SrvSpamFilterChatModel->append());
    row[adminRecord.id] = 0; row[adminRecord.name] = "Do Nothing";
    row = *(SrvSpamFilterChatModel->append());
    row[adminRecord.id] = 1; row[adminRecord.name] = "Kick User";
    row = *(SrvSpamFilterChatModel->append());
    row[adminRecord.id] = 2; row[adminRecord.name] = "Ban User";

    builder->get_widget("ServerSpamFilterCommentAction", combobox);
    SrvSpamFilterCommentModel = Gtk::TreeStore::create(adminRecord);
    combobox->set_model(SrvSpamFilterCommentModel);
    combobox->pack_start(adminRecord.name);
    row = *(SrvSpamFilterCommentModel->append());
    row[adminRecord.id] = 0; row[adminRecord.name] = "Do Nothing";
    row = *(SrvSpamFilterCommentModel->append());
    row[adminRecord.id] = 1; row[adminRecord.name] = "Kick User";
    row = *(SrvSpamFilterCommentModel->append());
    row[adminRecord.id] = 2; row[adminRecord.name] = "Ban User";

    builder->get_widget("ServerSpamFilterTTSAction", combobox);
    SrvSpamFilterTTSModel = Gtk::TreeStore::create(adminRecord);
    combobox->set_model(SrvSpamFilterTTSModel);
    combobox->pack_start(adminRecord.name);
    row = *(SrvSpamFilterTTSModel->append());
    row[adminRecord.id] = 0; row[adminRecord.name] = "Do Nothing";
    row = *(SrvSpamFilterTTSModel->append());
    row[adminRecord.id] = 1; row[adminRecord.name] = "Kick User";
    row = *(SrvSpamFilterTTSModel->append());
    row[adminRecord.id] = 2; row[adminRecord.name] = "Ban User";

    builder->get_widget("ServerSpamFilterWaveAction", combobox);
    SrvSpamFilterWaveModel = Gtk::TreeStore::create(adminRecord);
    combobox->set_model(SrvSpamFilterWaveModel);
    combobox->pack_start(adminRecord.name);
    row = *(SrvSpamFilterWaveModel->append());
    row[adminRecord.id] = 0; row[adminRecord.name] = "Do Nothing";
    row = *(SrvSpamFilterWaveModel->append());
    row[adminRecord.id] = 1; row[adminRecord.name] = "Kick User";
    row = *(SrvSpamFilterWaveModel->append());
    row[adminRecord.id] = 2; row[adminRecord.name] = "Ban User";

    builder->get_widget("ServerUpdate", button);
    button->signal_clicked().connect(sigc::mem_fun(this, &ManglerAdmin::ServerUpdate_clicked_cb));

    /* set up the channel editor stuff */
    builder->get_widget("ChannelEditorTree", ChannelEditorTree);
    ChannelEditorTreeModel = adminChannelStore::create();
    ChannelEditorTree->set_model(ChannelEditorTreeModel);
    pColumn = Gtk::manage( new Gtk::TreeView::Column("Channels") );
    pColumn->pack_start(adminRecord.name);
    pColumn->set_expand(true);
    ChannelEditorTree->append_column(*pColumn);
    ChannelEditorTree->signal_cursor_changed().connect(sigc::mem_fun(this, &ManglerAdmin::ChannelTree_cursor_changed_cb));
    ChannelEditorTree->enable_model_drag_source();
    ChannelEditorTree->enable_model_drag_dest();

    builder->get_widget("ChannelEditor", ChannelEditor);
    ChannelEditor->set_sensitive(false);

    builder->get_widget("ChannelAdd", ChannelAdd);
    ChannelAdd->set_sensitive(false);
    ChannelAdd->signal_clicked().connect(sigc::mem_fun(this, &ManglerAdmin::ChannelAdd_clicked_cb));

    builder->get_widget("ChannelRemove", ChannelRemove);
    ChannelRemove->set_sensitive(false);
    ChannelRemove->signal_clicked().connect(sigc::mem_fun(this, &ManglerAdmin::ChannelRemove_clicked_cb));

    builder->get_widget("ChannelUpdate", ChannelUpdate);
    ChannelUpdate->signal_clicked().connect(sigc::mem_fun(this, &ManglerAdmin::ChannelUpdate_clicked_cb));

    currentChannelID = 0;
    currentChannelParent = 0;

    builder->get_widget("ChannelProtMode", combobox);
    ChannelProtModel = Gtk::TreeStore::create(ChannelProtColumns);
    combobox->set_model(ChannelProtModel);
    combobox->pack_start(adminRecord.name);
    row = *(ChannelProtModel->append());
    row[adminRecord.id] = 0; row[adminRecord.name] = "Open to Public";
    row = *(ChannelProtModel->append());
    row[adminRecord.id] = 1; row[adminRecord.name] = "Channel Password";
    row = *(ChannelProtModel->append());
    row[adminRecord.id] = 2; row[adminRecord.name] = "User Authorization";
    combobox->signal_changed().connect(sigc::mem_fun(this, &ManglerAdmin::ChannelProtMode_changed_cb));

    builder->get_widget("ChannelVoiceMode", combobox);
    ChannelVoiceModel = Gtk::TreeStore::create(ChannelVoiceColumns);
    combobox->set_model(ChannelVoiceModel);
    combobox->pack_start(adminRecord.name);
    row = *(ChannelVoiceModel->append());
    row[adminRecord.id] = 0; row[adminRecord.name] = "Normal";
    row = *(ChannelVoiceModel->append());
    row[adminRecord.id] = 1; row[adminRecord.name] = "Queued";
    row = *(ChannelVoiceModel->append());
    row[adminRecord.id] = 2; row[adminRecord.name] = "Muted";
    combobox->signal_changed().connect(sigc::mem_fun(this, &ManglerAdmin::ChannelVoiceMode_changed_cb));

    builder->get_widget("ChannelCodec", combobox);
    ChannelCodecModel = Gtk::TreeStore::create(ChannelCodecColumns);
    combobox->set_model(ChannelCodecModel);
    combobox->pack_start(adminRecord.name);
    row = *(ChannelCodecModel->append());
    row[adminRecord.id] = 0; row[adminRecord.name] = "GSM";
    row = *(ChannelCodecModel->append());
    row[adminRecord.id] = 1; row[adminRecord.name] = "Codec 1";
    row = *(ChannelCodecModel->append());
    row[adminRecord.id] = 2; row[adminRecord.name] = "Codec 2";
    row = *(ChannelCodecModel->append());
    row[adminRecord.id] = 3; row[adminRecord.name] = "Speex";
    row = *(ChannelCodecModel->append());
    row[adminRecord.id] = 4; row[adminRecord.name] = "Server Default";
    combobox->signal_changed().connect(sigc::mem_fun(this, &ManglerAdmin::LoadCodecFormats));

    builder->get_widget("ChannelFormat", combobox);
    ChannelFormatModel = Gtk::TreeStore::create(ChannelCodecColumns);
    combobox->set_model(ChannelFormatModel);
    combobox->pack_start(adminRecord.name);

    /* set up the user editor stuff */
    UserEditorTreeModel = Gtk::TreeStore::create(UserEditorColumns);
    builder->get_widget("UserEditorTree", UserEditorTree);
    UserEditorTree->set_model(UserEditorTreeModel);
    pColumn = Gtk::manage( new Gtk::TreeView::Column("Users") );
    pColumn->pack_start(adminRecord.name);
    pColumn->set_expand(true);
    UserEditorTree->append_column(*pColumn);
    UserEditorTree->signal_cursor_changed().connect(sigc::mem_fun(this, &ManglerAdmin::UserTree_cursor_changed_cb));

    UserChanAdminModel = Gtk::TreeStore::create(UserChanAdminColumns);
    builder->get_widget("UserChanAdminTree", UserChanAdminTree);
    UserChanAdminTree->set_model(UserChanAdminModel);
    pColumn = Gtk::manage( new Gtk::TreeView::Column("Channels") );
    pColumn->pack_start(adminCheckRecord.name);
    pColumn->set_expand(true);
    UserChanAdminTree->append_column_editable("Select", adminCheckRecord.on);
    UserChanAdminTree->append_column(*pColumn);

    UserChanAuthModel = Gtk::TreeStore::create(UserChanAuthColumns);
    builder->get_widget("UserChanAuthTree", UserChanAuthTree);
    UserChanAuthTree->set_model(UserChanAuthModel);
    pColumn = Gtk::manage( new Gtk::TreeView::Column("Channels") );
    pColumn->pack_start(adminCheckRecord.name);
    pColumn->set_expand(true);
    UserChanAuthTree->append_column_editable("Select", adminCheckRecord.on);
    UserChanAuthTree->append_column(*pColumn);

    builder->get_widget("UserInfoSection", UserInfoSection);
    builder->get_widget("UserNetworkSection", UserNetworkSection);
    builder->get_widget("UserTransmitSection", UserTransmitSection);
    builder->get_widget("UserDisplaySection", UserDisplaySection);
    builder->get_widget("UserAdminSection", UserAdminSection);

    builder->get_widget("UserEditor", UserEditor);

    builder->get_widget("UserOwner", combobox);
    UserOwnerModel = Gtk::TreeStore::create(UserEditorColumns);
    combobox->set_model(UserOwnerModel);
    pColumn = Gtk::manage( new Gtk::TreeView::Column("Owners") );
    pColumn->pack_start(adminCheckRecord.name);
    pColumn->set_expand(true);
    combobox->pack_start(adminRecord.name);

    builder->get_widget("UserRank", combobox);
    UserRankModel = Gtk::TreeStore::create(UserRankColumns);
    combobox->set_model(UserRankModel);
    combobox->pack_start(adminRecord.name);

    builder->get_widget("UserDuplicateIPs", combobox);
    UserDuplicateIPsModel = Gtk::TreeStore::create(UserDuplicateIPsColumns);
    combobox->set_model(UserDuplicateIPsModel);
    combobox->pack_start(adminRecord.name);
    row = *(UserDuplicateIPsModel->append());
    row[adminRecord.id] = 0; row[adminRecord.name] = "No Limit";
    row = *(UserDuplicateIPsModel->append());
    row[adminRecord.id] = 1; row[adminRecord.name] = "Do Not Allow Duplicates";
    row = *(UserDuplicateIPsModel->append());
    row[adminRecord.id] = 2; row[adminRecord.name] = "2";
    row = *(UserDuplicateIPsModel->append());
    row[adminRecord.id] = 3; row[adminRecord.name] = "3";
    row = *(UserDuplicateIPsModel->append());
    row[adminRecord.id] = 4; row[adminRecord.name] = "4";
    row = *(UserDuplicateIPsModel->append());
    row[adminRecord.id] = 5; row[adminRecord.name] = "5";
    row = *(UserDuplicateIPsModel->append());
    row[adminRecord.id] = 6; row[adminRecord.name] = "6";
    row = *(UserDuplicateIPsModel->append());
    row[adminRecord.id] = 7; row[adminRecord.name] = "7";

    builder->get_widget("UserDefaultChannel", combobox);
    UserDefaultChannelModel = Gtk::TreeStore::create(UserDefaultChannelColumns);
    combobox->set_model(UserDefaultChannelModel);
    combobox->pack_start(adminRecord.name);
    renderer = (Gtk::CellRendererText*)(*(combobox->get_cells().begin()));
    renderer->property_ellipsize() = Pango::ELLIPSIZE_MIDDLE;

    builder->get_widget("UserAdd", UserAdd);
    UserAdd->signal_clicked().connect(sigc::mem_fun(this, &ManglerAdmin::UserAdd_clicked_cb));

    builder->get_widget("UserRemove", UserRemove);
    UserRemove->signal_clicked().connect(sigc::mem_fun(this, &ManglerAdmin::UserRemove_clicked_cb));

    builder->get_widget("UserUpdate", UserUpdate);
    UserUpdate->signal_clicked().connect(sigc::mem_fun(this, &ManglerAdmin::UserUpdate_clicked_cb));

    currentUserID = 0;

    builder->get_widget("UserInfoButton", togglebutton);
    togglebutton->signal_toggled().connect(sigc::mem_fun(this, &ManglerAdmin::UserInfoButton_toggled_cb));

    builder->get_widget("UserNetworkButton", togglebutton);
    togglebutton->signal_toggled().connect(sigc::mem_fun(this, &ManglerAdmin::UserNetworkButton_toggled_cb));

    builder->get_widget("UserTransmitButton", togglebutton);
    togglebutton->signal_toggled().connect(sigc::mem_fun(this, &ManglerAdmin::UserTransmitButton_toggled_cb));

    builder->get_widget("UserDisplayButton", togglebutton);
    togglebutton->signal_toggled().connect(sigc::mem_fun(this, &ManglerAdmin::UserDisplayButton_toggled_cb));

    builder->get_widget("UserAdminButton", togglebutton);
    togglebutton->signal_toggled().connect(sigc::mem_fun(this, &ManglerAdmin::UserAdminButton_toggled_cb));

    builder->get_widget("UserChanAdminButton", togglebutton);
    togglebutton->signal_toggled().connect(sigc::mem_fun(this, &ManglerAdmin::UserChanAdminButton_toggled_cb));

    builder->get_widget("UserChanAuthButton", togglebutton);
    togglebutton->signal_toggled().connect(sigc::mem_fun(this, &ManglerAdmin::UserChanAuthButton_toggled_cb));

    builder->get_widget("UserTemplate", UserTemplate);
    UserTemplateModel = Gtk::TreeStore::create(UserTemplateColumns);
    UserTemplate->set_model(UserTemplateModel);
    UserTemplate->pack_start(adminRecord.name);
    renderer = (Gtk::CellRendererText*)(*(UserTemplate->get_cells().begin()));
    renderer->property_ellipsize() = Pango::ELLIPSIZE_MIDDLE;
    UserTemplate->signal_changed().connect(sigc::mem_fun(this, &ManglerAdmin::UserTemplate_changed_cb));

    builder->get_widget("UserTemplateLoad", button);
    button->signal_clicked().connect(sigc::mem_fun(this, &ManglerAdmin::UserTemplateLoad_clicked_cb));

    builder->get_widget("UserTemplateDelete", button);
    button->signal_clicked().connect(sigc::mem_fun(this, &ManglerAdmin::UserTemplateDelete_clicked_cb));

    builder->get_widget("UserTemplateSave", button);
    button->signal_clicked().connect(sigc::mem_fun(this, &ManglerAdmin::UserTemplateSave_clicked_cb));

    /* set up the rank editor stuff */
    RankEditorModel = Gtk::TreeStore::create(RankEditorColumns);
    builder->get_widget("RankTree", RankEditorTree);
    RankEditorTree->set_model(RankEditorModel);
    RankEditorTree->append_column("Name", rankRecord.name);
    RankEditorTree->append_column("Level", rankRecord.level);
    RankEditorTree->append_column("Description", rankRecord.description);
    RankEditorTree->signal_cursor_changed().connect(sigc::mem_fun(this, &ManglerAdmin::RankEditorTree_cursor_changed_cb));

    builder->get_widget("RankEditor", RankEditor);

    builder->get_widget("RankAdd", button);
    button->signal_clicked().connect(sigc::mem_fun(this, &ManglerAdmin::RankAdd_clicked_cb));

    builder->get_widget("RankRemove", button);
    button->signal_clicked().connect(sigc::mem_fun(this, &ManglerAdmin::RankRemove_clicked_cb));

    builder->get_widget("RankUpdate", button);
    button->signal_clicked().connect(sigc::mem_fun(this, &ManglerAdmin::RankUpdate_clicked_cb));

    /* set up the ban editor stuff */
    BanEditorModel = Gtk::TreeStore::create(BanEditorColumns);
    builder->get_widget("BanTree", BanEditorTree);
    BanEditorTree->set_model(BanEditorModel);
    BanEditorTree->append_column("IP Address", banRecord.ip);
    BanEditorTree->append_column("Netmask", banRecord.netmask);
    BanEditorTree->append_column("User", banRecord.user);
    BanEditorTree->append_column("Admin", banRecord.by);
    BanEditorTree->append_column("Reason", banRecord.reason);
    BanEditorTree->signal_cursor_changed().connect(sigc::mem_fun(this, &ManglerAdmin::BanEditorTree_cursor_changed_cb));

    builder->get_widget("BanEditor", BanEditor);

    builder->get_widget("BanNetmask", combobox);
    BanNetmaskModel = Gtk::TreeStore::create(adminRecord);
    combobox->set_model(BanNetmaskModel);
    combobox->pack_start(adminRecord.name);
    for (uint32_t ctr = 8; _v3_bitmasks[ctr] != NULL; ctr++) {
        Gtk::TreeModel::iterator iter = BanNetmaskModel->append();
        (*iter)[adminRecord.id] = ctr;
        (*iter)[adminRecord.name] = c_to_ustring(_v3_bitmasks[ctr]);
        combobox->set_active(iter);
        BanNetmaskDefault = ctr;
    }

    builder->get_widget("BanAdd", button);
    button->signal_clicked().connect(sigc::mem_fun(this, &ManglerAdmin::BanAdd_clicked_cb));

    builder->get_widget("BanRemove", button);
    button->signal_clicked().connect(sigc::mem_fun(this, &ManglerAdmin::BanRemove_clicked_cb));

    builder->get_widget("BanUpdate", button);
    button->signal_clicked().connect(sigc::mem_fun(this, &ManglerAdmin::BanUpdate_clicked_cb));

    /* set up the editor tabs */
    clear();

    tmpldialog = NULL;
    isOpen = false;
}/*}}}*/
ManglerAdmin::~ManglerAdmin() {/*{{{*/
    if (tmpldialog) {
        delete tmpldialog;
    }
}/*}}}*/

void
ManglerAdmin::show(void) {/*{{{*/
    adminWindow->set_icon(mangler->icons["tray_icon"]);
    adminWindow->present();
    if (isOpen) {
        permsUpdated(true);
    }
}/*}}}*/
void
ManglerAdmin::hide(void) {/*{{{*/
    adminWindow->hide();
}/*}}}*/
void
ManglerAdmin::permsUpdated(bool refresh) {/*{{{*/
    const v3_permissions *perms = v3_get_permissions();
    clearRankEditor();
    setWidgetSensitive("RankAdd", RankEditorOpen && perms->edit_rank);
    if (isOpen && !refresh) {
        return;
    }
    if (mangler->isAdmin || perms->add_user || perms->del_user) {
        UserAdd->set_sensitive(mangler->isAdmin || perms->add_user);
        UsersTab->show();
    } else {
        UsersTab->hide();
    }
    if (perms->ban_user) {
        BansTab->show();
    } else {
        BansTab->hide();
    }
}/*}}}*/
void
ManglerAdmin::adminWindow_show_cb(void) {/*{{{*/
    const v3_permissions *perms = v3_get_permissions();
    permsUpdated();
    if (perms->ban_user) {
        v3_admin_ban_list();
    }
    if (mangler->isAdmin || perms->add_user || perms->del_user) {
        UserEditorOpen = true;
        v3_userlist_open();
    }
    if (perms->edit_rank) {
        RankEditorOpen = true;
        v3_ranklist_open();
    }
    if (mangler->isAdmin && SrvIsNotUpdating) {
        SrvEditorOpen = true;
        v3_serverprop_open();
    }
    loadUserTemplates();
    ChannelEditorTree->expand_all();
    UserChanAdminTree->expand_all();
    UserChanAuthTree->expand_all();
    Gtk::TreeModel::iterator iter = ChannelEditorTreeModel->children().begin();
    if (iter) {
        ChannelEditorTree->set_cursor(ChannelEditorTreeModel->get_path(iter));
    }
    isOpen = true;
}/*}}}*/
void
ManglerAdmin::adminWindow_hide_cb(void) {/*{{{*/
    if (SrvEditorOpen && SrvIsNotUpdating) {
        SrvEditorOpen = false;
        v3_serverprop_close();
    }
    if (RankEditorOpen) {
        RankEditorOpen = false;
        v3_ranklist_close();
    }
    if (UserEditorOpen) {
        UserEditorOpen = false;
        v3_userlist_close();
    }
    ServerTab->hide();
    clearUsers();
    clearRankEditor();
    clearBans();
    isOpen = false;
}/*}}}*/
void
ManglerAdmin::queue_resize(Gtk::TreeView *treeview) {/*{{{*/
    for (int ctr = 0, cnt = treeview->get_columns().size(); ctr < cnt; ctr++) {
        treeview->get_column(ctr)->queue_resize();
    }
}/*}}}*/
void
ManglerAdmin::statusbarPush(Glib::ustring msg) {/*{{{*/
    AdminStatusbar->push(msg);
    StatusbarCount++;
    StatusbarTime = time(NULL);
}/*}}}*/
bool
ManglerAdmin::statusbarPop(void) {/*{{{*/
    if (StatusbarTime + 3 > ::time(NULL)) return true;
    while (StatusbarCount) {
        AdminStatusbar->pop();
        StatusbarCount--;
    }
    return true;
}/*}}}*/
void
ManglerAdmin::CloseButton_clicked_cb(void) {/*{{{*/
    adminWindow->hide();
}/*}}}*/
void
ManglerAdmin::copyToEntry(const char *widgetName, Glib::ustring src) {/*{{{*/
    builder->get_widget(widgetName, entry);
    //if (src) entry->set_text(src);
    //else entry->set_text("");
    entry->set_text(src);
}/*}}}*/
void
ManglerAdmin::copyToSpinbutton(const char *widgetName, uint32_t src) {/*{{{*/
    builder->get_widget(widgetName, spinbutton);
    spinbutton->set_value(src);
}/*}}}*/
void
ManglerAdmin::copyToCheckbutton(const char *widgetName, bool src) {/*{{{*/
    builder->get_widget(widgetName, checkbutton);
    checkbutton->set_active(src);
}/*}}}*/
void
ManglerAdmin::copyToCombobox(const char *widgetName, uint32_t src, uint32_t deflt) {/*{{{*/
    builder->get_widget(widgetName, combobox);
    Glib::RefPtr<const Gtk::TreeModel> mdl = combobox->get_model();
    Gtk::TreeModel::Children children = mdl->children();
    if (! children || ! children.size()) {
        combobox->set_sensitive(false);
        return;
    } else combobox->set_sensitive(true);
    Gtk::TreeModel::iterator iter = children.begin();
    Gtk::TreeModel::iterator dIter = iter;
    while (iter != children.end()) {
        if ((*iter)[adminRecord.id] == deflt) dIter = iter;
        if ((*iter)[adminRecord.id] == src) break;
        iter++;
    }
    if (iter == children.end()) combobox->set_active(dIter);
    else combobox->set_active(iter);
}/*}}}*/
Glib::ustring
ManglerAdmin::getFromEntry(const char *widgetName) {/*{{{*/
    builder->get_widget(widgetName, entry);
    return entry->get_text();
}/*}}}*/
uint32_t
ManglerAdmin::getFromSpinbutton(const char *widgetName) {/*{{{*/
    builder->get_widget(widgetName, spinbutton);
    return uint32_t( spinbutton->get_value() );
}/*}}}*/
bool
ManglerAdmin::getFromCheckbutton(const char *widgetName) {/*{{{*/
    builder->get_widget(widgetName, checkbutton);
    return checkbutton->get_active();
}/*}}}*/
uint32_t
ManglerAdmin::getFromCombobox(const char *widgetName, uint32_t deflt) {/*{{{*/
    builder->get_widget(widgetName, combobox);
    Gtk::TreeModel::iterator iter = combobox->get_active();
    if (iter) return (*iter)[adminRecord.id];
    else return deflt;
}/*}}}*/
void
ManglerAdmin::setWidgetSensitive(const char *widgetName, bool widgetSens) {/*{{{*/
    Gtk::Widget *w;
    builder->get_widget(widgetName, w);
    w->set_sensitive(widgetSens);
}/*}}}*/

/* ----------  Server Settings Related Methods  ---------- */
void
ManglerAdmin::ServerUpdate_clicked_cb(void) {/*{{{*/
    v3_server_prop prop;

    memset(&prop, 0, sizeof(v3_server_prop));
    strncpy(prop.server_comment, ustring_to_c(getFromEntry("ServerComment")).c_str(), sizeof(prop.server_comment));
    prop.chat_filter = getFromCombobox("ServerChatFilter");
    prop.channel_order = getFromCombobox("ServerChannelOrdering");
    prop.motd_always = getFromCheckbutton("ServerAlwaysDisplayMOTD");
    // guest accounts
    prop.max_guest = getFromSpinbutton("ServerMaxGuests");
    prop.autokick_time = getFromSpinbutton("ServerKickGuests");
    prop.autoban_time = getFromSpinbutton("ServerBanGuests");
    // inactivity
    prop.inactivity_timeout = getFromSpinbutton("ServerTimeout");
    prop.inactivity_action = getFromCombobox("ServerAction");
    prop.inactivity_channel = getFromCombobox("ServerChannel", 0);
    // spam filters
    prop.channel_spam_filter.action = getFromCombobox("ServerSpamFilterChannelAction");
    prop.channel_spam_filter.interval = getFromSpinbutton("ServerSpamFilterChannelInterval");
    prop.channel_spam_filter.times = getFromSpinbutton("ServerSpamFilterChannelTimes");
    prop.chat_spam_filter.action = getFromCombobox("ServerSpamFilterChatAction");
    prop.chat_spam_filter.interval = getFromSpinbutton("ServerSpamFilterChatInterval");
    prop.chat_spam_filter.times = getFromSpinbutton("ServerSpamFilterChatTimes");
    prop.comment_spam_filter.action = getFromCombobox("ServerSpamFilterCommentAction");
    prop.comment_spam_filter.interval = getFromSpinbutton("ServerSpamFilterCommentInterval");
    prop.comment_spam_filter.times = getFromSpinbutton("ServerSpamFilterCommentTimes");
    prop.tts_spam_filter.action = getFromCombobox("ServerSpamFilterTTSAction");
    prop.tts_spam_filter.interval = getFromSpinbutton("ServerSpamFilterTTSInterval");
    prop.tts_spam_filter.times = getFromSpinbutton("ServerSpamFilterTTSTimes");
    prop.wave_spam_filter.action = getFromCombobox("ServerSpamFilterWaveAction");
    prop.wave_spam_filter.interval = getFromSpinbutton("ServerSpamFilterWaveInterval");
    prop.wave_spam_filter.times = getFromSpinbutton("ServerSpamFilterWaveTimes");
    // bind filters
    prop.tts_bind_filter = getFromCheckbutton("ServerBindFilterTTS");
    prop.wave_bind_filter = getFromCheckbutton("ServerBindFilterWave");
    // remote status
    prop.rem_srv_comment = getFromCheckbutton("ServerRemoteStatusServerComment");
    prop.rem_chan_names = getFromCheckbutton("ServerRemoteStatusChannelNames");
    prop.rem_chan_comments = getFromCheckbutton("ServerRemoteStatusChannelComments");
    prop.rem_user_names = getFromCheckbutton("ServerRemoteStatusUserNames");
    prop.rem_user_comments = getFromCheckbutton("ServerRemoteStatusUserComments");
    prop.rem_show_login_names = getFromCheckbutton("ServerRemoteStatusUseless");

    setWidgetSensitive("ServerVBox", false);
    SrvIsNotUpdating = false;
    v3_serverprop_update(&prop);
    statusbarPush("Sending server properties...");
}/*}}}*/
void
ManglerAdmin::serverSettingsUpdated(v3_server_prop &prop) {/*{{{*/
    copyToEntry("ServerComment", c_to_ustring(prop.server_comment));
    copyToCombobox("ServerChatFilter", prop.chat_filter, 0);
    copyToCombobox("ServerChannelOrdering", prop.channel_order, 0);
    copyToCheckbutton("ServerAlwaysDisplayMOTD", prop.motd_always);
    // guest accounts
    copyToSpinbutton("ServerMaxGuests", prop.max_guest);
    copyToSpinbutton("ServerKickGuests", prop.autokick_time);
    copyToSpinbutton("ServerBanGuests", prop.autoban_time);
    // inactivity
    copyToSpinbutton("ServerTimeout", prop.inactivity_timeout);
    copyToCombobox("ServerAction", prop.inactivity_action, 0);
    copyToCombobox("ServerChannel", prop.inactivity_channel, 0);
    // spam filters
    copyToCombobox("ServerSpamFilterChannelAction", prop.channel_spam_filter.action, 0);
    copyToSpinbutton("ServerSpamFilterChannelInterval", prop.channel_spam_filter.interval);
    copyToSpinbutton("ServerSpamFilterChannelTimes", prop.channel_spam_filter.times);
    copyToCombobox("ServerSpamFilterChatAction", prop.chat_spam_filter.action, 0);
    copyToSpinbutton("ServerSpamFilterChatInterval", prop.chat_spam_filter.interval);
    copyToSpinbutton("ServerSpamFilterChatTimes", prop.chat_spam_filter.times);
    copyToCombobox("ServerSpamFilterCommentAction", prop.comment_spam_filter.action, 0);
    copyToSpinbutton("ServerSpamFilterCommentInterval", prop.comment_spam_filter.interval);
    copyToSpinbutton("ServerSpamFilterCommentTimes", prop.comment_spam_filter.times);
    copyToCombobox("ServerSpamFilterTTSAction", prop.tts_spam_filter.action, 0);
    copyToSpinbutton("ServerSpamFilterTTSInterval", prop.tts_spam_filter.interval);
    copyToSpinbutton("ServerSpamFilterTTSTimes", prop.tts_spam_filter.times);
    copyToCombobox("ServerSpamFilterWaveAction", prop.wave_spam_filter.action, 0);
    copyToSpinbutton("ServerSpamFilterWaveInterval", prop.wave_spam_filter.interval);
    copyToSpinbutton("ServerSpamFilterWaveTimes", prop.wave_spam_filter.times);
    // bind filters
    copyToCheckbutton("ServerBindFilterTTS", prop.tts_bind_filter);
    copyToCheckbutton("ServerBindFilterWave", prop.wave_bind_filter);
    // remote status
    copyToCheckbutton("ServerRemoteStatusServerComment", prop.rem_srv_comment);
    copyToCheckbutton("ServerRemoteStatusChannelNames", prop.rem_chan_names);
    copyToCheckbutton("ServerRemoteStatusChannelComments", prop.rem_chan_comments);
    copyToCheckbutton("ServerRemoteStatusUserNames", prop.rem_user_names);
    copyToCheckbutton("ServerRemoteStatusUserComments", prop.rem_user_comments);
    copyToCheckbutton("ServerRemoteStatusUseless", prop.rem_show_login_names);

    setWidgetSensitive("ServerVBox", true);
    SrvIsNotUpdating = true;
    ServerTab->show();
}/*}}}*/
void
ManglerAdmin::serverSettingsSendDone(void) {/*{{{*/
    statusbarPush("Sending server properties... done.");
    v3_serverprop_close();
    SrvIsNotUpdating = true;
    if ((SrvEditorOpen = isOpen)) {
        v3_serverprop_open();
    } else {
        setWidgetSensitive("ServerVBox", true);
    }
}/*}}}*/

/* ----------  Channel Editor Related Methods  ---------- */
Glib::RefPtr<adminChannelStore>
adminChannelStore::create() {/*{{{*/
    return Glib::RefPtr<adminChannelStore>( new adminChannelStore() );
}/*}}}*/
bool
adminChannelStore::row_draggable_vfunc(const Gtk::TreeModel::Path& path) const {/*{{{*/
    adminChannelStore* _this = const_cast<adminChannelStore*>(this);
    const_iterator iter = _this->get_iter(path);
    if (!iter) {
        return Gtk::TreeStore::row_draggable_vfunc(path);
    }
    return (mangler->isAdmin && mangler->admin->channelSortManual && (*iter)[c.id] != 0);
}/*}}}*/
bool
adminChannelStore::row_drop_possible_vfunc(const Gtk::TreeModel::Path& dest, const Gtk::SelectionData& selection_data) const {/*{{{*/
    Gtk::TreeModel::Path sel;
    Gtk::TreeModel::Path::get_from_selection_data(selection_data, sel);
    return (sel.get_depth() == dest.get_depth());
}/*}}}*/
bool
adminChannelStore::drag_data_received_vfunc(const Gtk::TreeModel::Path& dest, const Gtk::SelectionData& selection_data) {/*{{{*/
    Gtk::TreeModel::Path sel;
    Gtk::TreeModel::Path::get_from_selection_data(selection_data, sel);
    if (sel.get_depth() != dest.get_depth()) {
        return false;
    }
    adminChannelStore* _this = const_cast<adminChannelStore*>(this);
    const_iterator iter;
    uint16_t src_id, dest_id;
    if (!(iter = _this->get_iter(sel)) || !(src_id = (*iter)[c.id]) ||
        !(iter = _this->get_iter(dest)) || !(dest_id = (*iter)[c.id]) || src_id == dest_id) {
        return false;
    }
    v3_force_channel_move(src_id, dest_id);
    return false;
}/*}}}*/
void
ManglerAdmin::ChannelTree_cursor_changed_cb() {/*{{{*/
    Gtk::TreeModel::iterator iter = ChannelEditorTree->get_selection()->get_selected();
    if (!iter) {
        return;
    }
    Gtk::TreeModel::Row row = *iter;
    currentChannelID = row[adminRecord.id];

    if (currentChannelID) {
        // load channel data into editor
        v3_channel *channel;
        if ((channel = v3_get_channel(currentChannelID))) {
            populateChannelEditor(channel);
            v3_free_channel(channel);
        } else {
            fprintf(stderr, "failed to retrieve channel information for channel id %d\n", currentChannelID);
            populateChannelEditor(NULL);
            currentChannelID = 0;
            currentChannelParent = 0;
        }
    } else {
        populateChannelEditor(NULL);
    }
    // get user permissions
    //const v3_permissions *perms = v3_get_permissions();
    // enable or disable editor and necessary buttons
    bool editAccess( mangler->isAdmin || v3_is_channel_admin(currentChannelID) );
    ChannelUpdate->set_sensitive(editAccess && currentChannelID);
    ChannelEditor->set_sensitive(currentChannelID);
    ChannelRemove->set_sensitive(editAccess && currentChannelID);
    ChannelAdd->set_sensitive(editAccess);
}/*}}}*/
Gtk::TreeModel::Row
ManglerAdmin::getChannel(uint32_t id, Gtk::TreeModel::Children children, bool hasCheckbox) {/*{{{*/
    Gtk::TreeModel::iterator iter = children.begin();
    while (iter != children.end()) {
        Gtk::TreeModel::Row row = *iter;
        uint32_t rowId = hasCheckbox ? row[adminCheckRecord.id] : row[adminRecord.id];
        if (rowId == id) {
            return row;
        }
        if (row.children().size()) {
            if (row = getChannel(id, row->children())) {
                return row;
            }
        }
        iter++;
    }
    return *iter;
}/*}}}*/
Glib::ustring
ManglerAdmin::getChannelPathString(uint32_t id, Gtk::TreeModel::Children children) {/*{{{*/
    Gtk::TreeModel::iterator iter = children.begin();
    while (iter != children.end()) {
        Gtk::TreeModel::Row row = *iter;
        uint32_t rowId = row[adminRecord.id];
        if (rowId == id) {
            return row[adminRecord.name];
        }
        if (row.children().size()) {
            Glib::ustring retstr = getChannelPathString(id, row->children());
            if (! retstr.empty()) {
                if (rowId == 0) return retstr;
                else return Glib::ustring::compose("%1 : %2", row[adminRecord.name], retstr);
            }
        }
        iter++;
    }
    return "";
}/*}}}*/
void
ManglerAdmin::channelUpdated(v3_channel *channel) {/*{{{*/
    /* channel editor tree */
    Gtk::TreeModel::Row chanrow;
    Gtk::TreeModel::iterator iter;

    chanrow = getChannel(channel->id, ChannelEditorTreeModel->children());
    if (chanrow) {
        chanrow[adminRecord.id] = channel->id;
        chanrow[adminRecord.name] = c_to_ustring(channel->name);
        if (currentChannelID == channel->id) populateChannelEditor(channel);
    }
    queue_resize(ChannelEditorTree);
    /* server inactive "move to" channel combo box */
    Gtk::TreeModel::Children siChildren = SrvInactChannelModel->children();
    bool found( false );
    if (siChildren.size()) {
        iter = siChildren.begin();
        while (iter != siChildren.end()) {
            if ((*iter)[adminRecord.id] == channel->id) {
                found = true;
                break;
            }
            iter++;
        }
    }
    if (! found) iter = SrvInactChannelModel->append();
    (*iter)[adminRecord.id] = channel->id;
    (*iter)[adminRecord.name] = getChannelPathString(channel->id, ChannelEditorTreeModel->children());
    /* user default channel combo box */
    Gtk::TreeModel::Children udChildren = UserDefaultChannelModel->children();
    found = false;
    if (udChildren.size()) {
        iter = udChildren.begin();
        while (iter != udChildren.end()) {
            if ((*iter)[adminRecord.id] == channel->id) {
                found = true;
                break;
            }
            iter++;
        }
    }
    if (! found) iter = UserDefaultChannelModel->append();
    (*iter)[adminRecord.id] = channel->id;
    (*iter)[adminRecord.name] = getChannelPathString(channel->id, ChannelEditorTreeModel->children());
    /* user channel admin tree */
    chanrow = getChannel(channel->id, UserChanAdminModel->children(), true);
    if (chanrow) {
        chanrow[adminCheckRecord.id] = channel->id;
        chanrow[adminCheckRecord.name] = c_to_ustring(channel->name);
    }
    queue_resize(UserChanAdminTree);
    /* user channel auth tree */
    chanrow = getChannel(channel->id, UserChanAuthModel->children(), true);
    if (chanrow) {
        if (channel->protect_mode == 2) {
            chanrow[adminCheckRecord.id] = channel->id;
            chanrow[adminCheckRecord.name] = c_to_ustring(channel->name);
        } else UserChanAuthModel->erase(chanrow);
    } else if (channel->protect_mode == 2) {
        chanrow = *(UserChanAuthModel->append());
        if (chanrow) {
            chanrow[adminCheckRecord.id] = channel->id;
            chanrow[adminCheckRecord.name] = c_to_ustring(channel->name);
        }
    }
    queue_resize(UserChanAuthTree);
    /* update status bar */
    statusbarPush(Glib::ustring::compose("Channel %1 (%2) updated.", Glib::ustring::format(channel->id), c_to_ustring(channel->name)));
}/*}}}*/
void
ManglerAdmin::channelRemoved(uint32_t chanid) {/*{{{*/
    /* channel editor tree */
    Gtk::TreeModel::Row chanrow;
    Gtk::TreeModel::iterator iter;

    chanrow = getChannel(chanid, ChannelEditorTreeModel->children());
    if (chanrow) {
        iter = ChannelEditorTree->get_selection()->get_selected();
        if (iter && (*iter)[adminRecord.id] == chanrow[adminRecord.id]) {
            populateChannelEditor(NULL);
            currentChannelID = 0;
            currentChannelParent = 0;
        }
        ChannelEditorTreeModel->erase(chanrow);
    }
    queue_resize(ChannelEditorTree);
    /* server inactive "move to" channel combo box */
    Gtk::TreeModel::Children siChildren = SrvInactChannelModel->children();
    if (siChildren.size()) {
        iter = siChildren.begin();
        while (iter != siChildren.end()) {
            if ((*iter)[adminRecord.id] == chanid) {
                SrvInactChannelModel->erase(*iter);
                break;
            }
            iter++;
        }
    }
    /* user default channel combo box */
    Gtk::TreeModel::Children udChildren = UserDefaultChannelModel->children();
    if (udChildren.size()) {
        iter = udChildren.begin();
        while (iter != udChildren.end()) {
            if ((*iter)[adminRecord.id] == chanid) {
                UserDefaultChannelModel->erase(*iter);
                break;
            }
            iter++;
        }
    }
    /* user channel admin tree */
    chanrow = getChannel(chanid, UserChanAdminModel->children());
    if (chanrow) UserChanAdminModel->erase(chanrow);
    queue_resize(UserChanAdminTree);
    /* user channel auth tree */
    chanrow = getChannel(chanid, UserChanAuthModel->children());
    if (chanrow) UserChanAuthModel->erase(chanrow);
    queue_resize(UserChanAuthTree);
    /* update status bar */
    statusbarPush(Glib::ustring::compose("Channel %1 removed.", Glib::ustring::format(chanid)));
}/*}}}*/
void
ManglerAdmin::channelRemoved(v3_channel *channel) {/*{{{*/
    channelRemoved(channel->id);
}/*}}}*/
void
ManglerAdmin::channelAdded(v3_channel *channel) {/*{{{*/
    /* channel editor tree */
    Gtk::TreeModel::Row parent;
    Gtk::TreeModel::iterator channelIter;
    Gtk::TreeModel::Row channelRow;
    parent = getChannel(channel->parent, ChannelEditorTreeModel->children());
    if (parent) {
        channelIter = ChannelEditorTreeModel->append(parent.children());
    } else {
        channelIter = ChannelEditorTreeModel->append();
    }
    channelRow = *channelIter;
    channelRow[adminRecord.id] = channel->id;
    channelRow[adminRecord.name] = c_to_ustring(channel->name);
    if (ChannelAdded) {
        ChannelAdded = false;
        if (currentChannelID == 0xffff && c_to_ustring(channel->name) == trimString(getFromEntry("ChannelName"))) {
            if (parent) {
                ChannelEditorTree->expand_row(ChannelEditorTreeModel->get_path(parent), false);
            }
            ChannelEditorTree->set_cursor(ChannelEditorTreeModel->get_path(channelIter));
        }
    }
    queue_resize(ChannelEditorTree);
    /* server inactive "move to" channel combo box */
    channelIter = SrvInactChannelModel->append();
    (*channelIter)[adminRecord.id] = channel->id;
    (*channelIter)[adminRecord.name] = getChannelPathString(channel->id, ChannelEditorTreeModel->children());
    /* user default channel combo box */
    channelIter = UserDefaultChannelModel->append();
    (*channelIter)[adminRecord.id] = channel->id;
    (*channelIter)[adminRecord.name] = getChannelPathString(channel->id, ChannelEditorTreeModel->children());
    /* user channel admin tree */
    parent = getChannel(channel->parent, UserChanAdminModel->children(), true);
    if (parent) {
        channelIter = UserChanAdminModel->append(parent.children());
    } else {
        channelIter = UserChanAdminModel->append();
    }
    channelRow = *channelIter;
    channelRow[adminCheckRecord.id] = channel->id;
    channelRow[adminCheckRecord.name] = c_to_ustring(channel->name);
    queue_resize(UserChanAdminTree);
    /* user channel auth tree */
    if (channel->protect_mode == 2) {
        parent = getChannel(channel->parent, UserChanAuthModel->children(), true);
        if (parent) {
            channelIter = UserChanAuthModel->append(parent.children());
        } else {
            channelIter = UserChanAuthModel->append();
        }
        channelRow = *channelIter;
        channelRow[adminCheckRecord.id] = channel->id;
        channelRow[adminCheckRecord.name] = c_to_ustring(channel->name);
    }
    queue_resize(UserChanAuthTree);
    /* update status bar */
    statusbarPush(Glib::ustring::compose("Channel %1 (%2) added.", Glib::ustring::format(channel->id), c_to_ustring(channel->name)));
}/*}}}*/
void
ManglerAdmin::populateChannelEditor(const v3_channel *channel) {/*{{{*/
    v3_channel c;
    ::memset(&c, 0, sizeof(v3_channel));
    c.channel_codec = 0xffff;
    if (channel) {
        ::memcpy(&c, channel, sizeof(v3_channel));
        currentChannelID = c.id;
        currentChannelParent = c.parent;
    } else {
        ChannelUpdate->set_sensitive(false);
        ChannelEditor->set_sensitive(false);
        ChannelRemove->set_sensitive(false);
    }
    //fprintf(stderr, "Populate: channel %lu, parent %lu\n", currentChannelID, currentChannelParent);
    copyToEntry("ChannelName", c_to_ustring(c.name));
    copyToEntry("ChannelPhonetic", c_to_ustring(c.phonetic));
    copyToEntry("ChannelComment", c_to_ustring(c.comment));
    copyToEntry("ChannelPassword", (c.password_protected) ? "        " : "");
    copyToCombobox("ChannelProtMode", c.protect_mode, 0);
    copyToCombobox("ChannelVoiceMode", c.voice_mode, 0);
    copyToCheckbutton("AllowRecording", c.allow_recording);
    copyToCheckbutton("AllowCCxmit", c.allow_cross_channel_transmit);
    copyToCheckbutton("AllowPaging", c.allow_paging);
    copyToCheckbutton("AllowWaveBinds", c.allow_wave_file_binds);
    copyToCheckbutton("AllowTTSBinds", c.allow_tts_binds);
    copyToCheckbutton("AllowU2Uxmit", c.allow_u2u_transmit);
    copyToCheckbutton("AllowPhantoms", c.allow_phantoms);
    copyToCheckbutton("AllowGuests", c.allow_guests);
    copyToCheckbutton("AllowVoiceTargets", c.allow_voice_target);
    copyToCheckbutton("AllowCommandTargets", c.allow_command_target);
    copyToCheckbutton("TimerExempt", c.inactive_exempt);
    copyToCheckbutton("MuteGuests", c.disable_guest_transmit);
    copyToCheckbutton("DisableSoundEvents", c.disable_sound_events);
    if (v3_is_licensed()) {
        copyToCombobox("ChannelCodec" , c.channel_codec, 4);
        if (c.channel_codec != 0xffff) {
            copyToCombobox("ChannelFormat", c.channel_format);
        }
        setWidgetSensitive("ChannelCodecLabel", true);
        setWidgetSensitive("ChannelCodec", true);
        setWidgetSensitive("ChannelFormatLabel", true);
        setWidgetSensitive("ChannelFormat", true);
        setWidgetSensitive("AllowVoiceTargets", true);
        setWidgetSensitive("AllowCommandTargets", true);
    } else {
        copyToCombobox("ChannelCodec" , 4);
        setWidgetSensitive("ChannelCodecLabel", false);
        setWidgetSensitive("ChannelCodec", false);
        setWidgetSensitive("ChannelFormatLabel", false);
        setWidgetSensitive("ChannelFormat", false);
        setWidgetSensitive("AllowVoiceTargets", false);
        setWidgetSensitive("AllowCommandTargets", false);
    }
    builder->get_widget("ChannelEditorLabel", label);
    label->set_text("Editing: " + ((channel) ? c_to_ustring(c.name) : "NONE"));
    //LoadCodecFormats();
}/*}}}*/
void
ManglerAdmin::ChannelAdd_clicked_cb(void) {/*{{{*/
    ChannelEditorTree->get_selection()->unselect_all();
    populateChannelEditor(NULL);
    copyToCheckbutton("AllowRecording", true);
    copyToCheckbutton("AllowCCxmit", true);
    copyToCheckbutton("AllowPaging", true);
    copyToCheckbutton("AllowWaveBinds", true);
    copyToCheckbutton("AllowTTSBinds", true);
    copyToCheckbutton("AllowU2Uxmit", true);
    copyToCheckbutton("AllowPhantoms", true);
    copyToCheckbutton("AllowGuests", true);
    bool isLicensed( v3_is_licensed() );
    copyToCheckbutton("AllowVoiceTargets", isLicensed);
    copyToCheckbutton("AllowCommandTargets", isLicensed);
    builder->get_widget("ChannelEditorLabel", label);
    label->set_text("Editing: NEW CHANNEL");
    currentChannelParent = currentChannelID;
    currentChannelID = 0xffff;
    //fprintf(stderr, "Add: channel %lu, parent %lu\n", currentChannelID, currentChannelParent);
    ChannelAdd->set_sensitive(false);
    ChannelRemove->set_sensitive(false);
    ChannelEditor->set_sensitive(true);
    ChannelUpdate->set_sensitive(true);
}/*}}}*/
void
ManglerAdmin::ChannelRemove_clicked_cb(void) {/*{{{*/
    v3_channel *channel = v3_get_channel(currentChannelID);
    if (!channel) return;
    Gtk::MessageDialog confirmDlg( Glib::ustring::compose("Are you sure you want to remove channel %1 (%2)?", Glib::ustring::format(channel->id), c_to_ustring(channel->name)), false, Gtk::MESSAGE_QUESTION, Gtk::BUTTONS_YES_NO, true );
    v3_free_channel(channel);
    if (confirmDlg.run() == Gtk::RESPONSE_YES) {
        v3_channel_remove(currentChannelID);
    }
}/*}}}*/
void
ManglerAdmin::ChannelUpdate_clicked_cb(void) {/*{{{*/
    Gtk::TreeModel::iterator iter;
    v3_channel channel;
    Glib::ustring password;
    ::memset(&channel, 0, sizeof(v3_channel));
    if (currentChannelID == 0xffff) {
        // New Record
        channel.id = 0;
        ChannelAdded = true;
    } else {
        channel.id = currentChannelID;
    }
    channel.parent = currentChannelParent;
    //fprintf(stderr, "Update: channel %lu, parent %lu\n", channel.id, channel.parent);
    channel.name = ::strdup(ustring_to_c(getFromEntry("ChannelName")).c_str());
    channel.phonetic = ::strdup(ustring_to_c(getFromEntry("ChannelPhonetic")).c_str());
    channel.comment = ::strdup(ustring_to_c(getFromEntry("ChannelComment")).c_str());
    password = trimString(getFromEntry("ChannelPassword"));
    if (password.length()) channel.password_protected = 1;
    channel.protect_mode = getFromCombobox("ChannelProtMode", 0);
    channel.voice_mode = getFromCombobox("ChannelVoiceMode", 0);
    channel.allow_recording = getFromCheckbutton("AllowRecording") ? 1 : 0;
    channel.allow_cross_channel_transmit = getFromCheckbutton("AllowCCxmit") ? 1 : 0;
    channel.allow_paging = getFromCheckbutton("AllowPaging") ? 1 : 0;
    channel.allow_wave_file_binds = getFromCheckbutton("AllowWaveBinds") ? 1 : 0;
    channel.allow_tts_binds = getFromCheckbutton("AllowTTSBinds") ? 1 : 0;
    channel.allow_u2u_transmit = getFromCheckbutton("AllowU2Uxmit") ? 1 : 0;
    channel.allow_phantoms = getFromCheckbutton("AllowPhantoms") ? 1 : 0;
    channel.allow_guests = getFromCheckbutton("AllowGuests") ? 1 : 0;
    channel.allow_voice_target = getFromCheckbutton("AllowVoiceTargets") ? 1 : 0;
    channel.allow_command_target = getFromCheckbutton("AllowCommandTargets") ? 1 : 0;
    channel.inactive_exempt = getFromCheckbutton("TimerExempt") ? 1 : 0;
    channel.disable_guest_transmit = getFromCheckbutton("MuteGuests") ? 1 : 0;
    channel.disable_sound_events = getFromCheckbutton("DisableSoundEvents") ? 1 : 0;
    if (v3_is_licensed()) {
        channel.channel_codec = getFromCombobox("ChannelCodec", 4);
        if (channel.channel_codec < 4) {
            channel.channel_format = getFromCombobox("ChannelFormat", 0);
        } else {
            channel.channel_codec = 0xffff;
            channel.channel_format = 0xffff;
        }
    } else {
        channel.channel_codec = 0xffff;
        channel.channel_format = 0xffff;
    }
    //fprintf(stderr, "Updating:\nname: %s\nphonetic: %s\ncomment: %s\npassword: %s\n",
    //    channel.name, channel.phonetic, channel.comment, password.c_str());
    v3_channel_update(&channel, ustring_to_c(password).c_str());
    ::free(channel.name);
    ::free(channel.phonetic);
    ::free(channel.comment);
}/*}}}*/
void
ManglerAdmin::channelSort(bool manual) {/*{{{*/
    channelSortManual = manual;
    if (!manual) {
        ChannelEditorTreeModel->set_sort_func(0, sigc::mem_fun(*this, &ManglerAdmin::channelSortFunction));
        ChannelEditorTreeModel->set_sort_column(adminRecord.name, Gtk::SORT_ASCENDING);
    } else {
        ChannelEditorTreeModel->set_sort_func(adminRecord.id, sigc::mem_fun(*this, &ManglerAdmin::channelSortFunction));
        ChannelEditorTreeModel->set_sort_column(adminRecord.id, Gtk::SORT_ASCENDING);
    }
}/*}}}*/
void
ManglerAdmin::channelResort(void) {/*{{{*/
    channelSort(channelSortManual);
}/*}}}*/
int
ManglerAdmin::channelSortFunction(const Gtk::TreeModel::iterator &left, const Gtk::TreeModel::iterator &right) {/*{{{*/
    if (!channelSortManual) {
        Glib::ustring leftstr = (*left)[adminRecord.name];
        Glib::ustring rightstr = (*right)[adminRecord.name];
        return natsort(leftstr.c_str(), rightstr.c_str());
    } else {
        return v3_get_channel_sort((*left)[adminRecord.id], (*right)[adminRecord.id]);
    }
}/*}}}*/
void
ManglerAdmin::clearChannels(void) {/*{{{*/
    ChannelAdded = false;
    ChannelEditorTreeModel->clear();
    queue_resize(ChannelEditorTree);
    Gtk::TreeModel::Row lobby = *(ChannelEditorTreeModel->append());
    lobby[adminRecord.id] = 0;
    lobby[adminRecord.name] = "(Lobby)";
    currentChannelID = 0;
    currentChannelParent = 0;
    ChannelUpdate->set_sensitive(false);
    ChannelEditor->set_sensitive(false);
    ChannelRemove->set_sensitive(false);
    ChannelAdd->set_sensitive(false);
    SrvInactChannelModel->clear();
    lobby = *(SrvInactChannelModel->append());
    lobby[adminRecord.id] = 0;
    lobby[adminRecord.name] = "(Lobby)";
    UserDefaultChannelModel->clear();
    lobby = *(UserDefaultChannelModel->append());
    lobby[adminRecord.id] = 0;
    lobby[adminRecord.name] = "(Lobby)";
    UserChanAdminModel->clear();
    queue_resize(UserChanAdminTree);
    UserChanAuthModel->clear();
    queue_resize(UserChanAuthTree);
}/*}}}*/
void
ManglerAdmin::LoadCodecFormats(void) {/*{{{*/
    builder->get_widget("ChannelCodec", combobox);
    uint16_t c = 4;
    Gtk::TreeModel::iterator iter = combobox->get_active();
    if (iter) c = (*iter)[adminRecord.id];
    builder->get_widget("ChannelFormat", combobox);
    combobox->set_sensitive(c < 4);
    uint16_t f = 0;
    const v3_codec *codec;
    Gtk::TreeModel::Row row;
    ChannelFormatModel->clear();
    while ((codec = v3_get_codec(c, f)) && codec->codec != (uint8_t)-1) {
        row = *(ChannelFormatModel->append());
        row[adminRecord.id] = f;
        row[adminRecord.name] = codec->name;
        f++;
    }
    if (c < 4 && ! combobox->get_active()) {
        combobox->set_active(0);
    } else {
        combobox->set_active(-1);
    }
}/*}}}*/
void
ManglerAdmin::ChannelProtMode_changed_cb(void) {/*{{{*/
    Gtk::TreeModel::iterator iter;
    builder->get_widget("ChannelProtMode", combobox);
    iter = combobox->get_active();
    bool isPassword( iter && (*iter)[adminRecord.id] == 1);
    builder->get_widget("ChannelPassword", entry);
    entry->set_sensitive(isPassword);
}/*}}}*/
void
ManglerAdmin::ChannelVoiceMode_changed_cb(void) {/*{{{*/
    Gtk::TreeModel::iterator iter;
    builder->get_widget("ChannelVoiceMode", combobox);
    iter = combobox->get_active();
    bool notNormal( iter && (*iter)[adminRecord.id] );
    builder->get_widget("TransmitRank", spinbutton);
    spinbutton->set_sensitive(notNormal);
}/*}}}*/

/* ----------  User Editor Related Methods  ---------- */
Gtk::TreeModel::Row
ManglerAdmin::getAccount(uint32_t id, Gtk::TreeModel::Children children) {/*{{{*/
    Gtk::TreeModel::iterator iter = children.begin();
    while (iter != children.end()) {
        if ((*iter)[adminRecord.id] == id) break;
        iter++;
    }
    return *iter;
}/*}}}*/
void
ManglerAdmin::accountUpdated(v3_account *account) {/*{{{*/
    /* main user list */
    Gtk::TreeModel::Row acct;
    acct = getAccount(account->perms.account_id, UserEditorTreeModel->children());
    if (! acct) return;
    acct[adminRecord.name] = c_to_ustring(account->username);
    /* User Owner combo box */
    acct = getAccount(account->perms.account_id, UserOwnerModel->children());
    if (acct) {
        if (account->perms.srv_admin || account->perms.add_user) {
            /* update name in owner list */
            acct[adminRecord.name] = c_to_ustring(account->username);
        } else {
            /* needs to be removed, no longer an admin */
            UserOwnerModel->erase(acct);
        }
    } else if (account->perms.srv_admin || account->perms.add_user) {
        /* needs to be added to owner list */
        acct = *(UserOwnerModel->append());
        acct[adminRecord.id] = account->perms.account_id;
        acct[adminRecord.name] = c_to_ustring(account->username);
    }
    if (account->perms.account_id == currentUserID) populateUserEditor(account);
    queue_resize(UserEditorTree);
    /* update status bar */
    statusbarPush(Glib::ustring::compose("User %1 (%2) updated.", Glib::ustring::format(account->perms.account_id), c_to_ustring(account->username)));
}/*}}}*/
void
ManglerAdmin::accountAdded(v3_account *account) {/*{{{*/
    /* main user list */
    Gtk::TreeModel::iterator iter;
    Gtk::TreeModel::Row acct;
    iter = UserEditorTreeModel->append();
    acct = *iter;
    acct[adminRecord.id] = account->perms.account_id;
    acct[adminRecord.name] = c_to_ustring(account->username);
    /* User Owner combo box */
    if (account->perms.srv_admin || account->perms.add_user) {
        /* needs to be added to owner list */
        acct = *(UserOwnerModel->append());
        acct[adminRecord.id] = account->perms.account_id;
        acct[adminRecord.name] = c_to_ustring(account->username);
    }
    if (currentUserID == 0xffff && c_to_ustring(account->username) == trimString(getFromEntry("UserLogin"))) {
        UserEditorTree->set_cursor(UserEditorTreeModel->get_path(iter));
    }
    queue_resize(UserEditorTree);
    /* update status bar */
    statusbarPush(Glib::ustring::compose("User %1 (%2) added.", Glib::ustring::format(account->perms.account_id), c_to_ustring(account->username)));
}/*}}}*/
void
ManglerAdmin::accountRemoved(uint32_t acctid) {/*{{{*/
    /* main user list */
    Gtk::TreeModel::Row acct;
    Gtk::TreeModel::iterator iter;

    acct = getAccount(acctid, UserEditorTreeModel->children());
    if (acct) {
        iter = UserEditorTree->get_selection()->get_selected();
        if (iter && (*iter)[adminRecord.id] == acct[adminRecord.id]) {
            populateUserEditor(NULL);
            currentUserID = 0;
        }
        UserEditorTreeModel->erase(acct);
    }
    queue_resize(UserEditorTree);
    /* User Owner combo box */
    acct = getAccount(acctid, UserOwnerModel->children());
    if (acct) UserOwnerModel->erase(acct);
    /* update status bar */
    statusbarPush(Glib::ustring::compose("User %1 removed.", Glib::ustring::format(acctid)));
}/*}}}*/
void
ManglerAdmin::accountRemoved(v3_account *account) {/*{{{*/
    if (account) accountRemoved(account->perms.account_id);
}/*}}}*/
void
ManglerAdmin::UserTree_cursor_changed_cb(void) {/*{{{*/
    Gtk::TreeModel::iterator iter = UserEditorTree->get_selection()->get_selected();
    if (!iter) {
        return;
    }
    Gtk::TreeModel::Row row = *iter;
    currentUserID = row[adminRecord.id];

    if (currentUserID) {
        // load user data into editor
        v3_account *account;
        if ((account = v3_get_account(currentUserID))) {
            populateUserEditor(account);
            v3_free_account(account);
        } else {
            fprintf(stderr, "failed to retrieve user information for account id %d\n", currentUserID);
            populateUserEditor(NULL);
            currentUserID = 0;
        }
    } else {
        populateUserEditor(NULL);
    }
    // get user permissions
    const v3_permissions *perms = v3_get_permissions();
    // enable or disable editor and necessary buttons
    bool editAccess( mangler->isAdmin );
    UserUpdate->set_sensitive((editAccess || perms->add_user) && currentUserID);
    UserEditor->set_sensitive(currentUserID);
    UserRemove->set_sensitive((editAccess || perms->del_user) && currentUserID);
    UserAdd->set_sensitive(editAccess || perms->add_user);
}/*}}}*/
void
ManglerAdmin::populateUserEditor(const v3_account *account, bool isTemplate) {/*{{{*/
    v3_account a;
    ::memset(&a, 0, sizeof(v3_account));
    bool isLicensed( v3_is_licensed() );
    if (!isTemplate) {
        if (account) {
            ::memcpy(&a, account, sizeof(v3_account));
            setWidgetSensitive("UserLogin", false);
        } else {
            setWidgetSensitive("UserLogin", true);
            UserUpdate->set_sensitive(false);
            UserEditor->set_sensitive(false);
            UserRemove->set_sensitive(false);
        }
        copyToEntry("UserLogin", c_to_ustring(a.username));
        copyToEntry("UserPassword", (a.perms.account_id > 1) ? "        " : "");
        setWidgetSensitive("UserPassword", a.perms.account_id != 1);
        copyToCombobox("UserRank", a.perms.rank_id, 0);
        copyToCombobox("UserOwner", 0);
        Glib::ustring ownerName = c_to_ustring(a.owner);
        if (ownerName.length()) {
            Gtk::TreeModel::iterator iter = UserOwnerModel->children().begin();
            while (iter != UserOwnerModel->children().end()) {
                if ((*iter)[adminRecord.name] == ownerName) {
                    copyToCombobox("UserOwner", (*iter)[adminRecord.id], 0);
                    break;
                }
                iter++;
            }
        }
        Gtk::TextView *textview;
        builder->get_widget("UserNotes", textview);
        textview->get_buffer()->set_text(c_to_ustring(a.notes));
    } else if (account) {
        ::memcpy(&a, account, sizeof(v3_account));
    }
    copyToCheckbutton("UserLocked", a.perms.lock_acct);
    copyToEntry("UserLockedReason", c_to_ustring(a.lock_reason));
    copyToCheckbutton("UserInReservedList", a.perms.in_reserve_list);
    copyToCheckbutton("UserReceiveBroadcasts", a.perms.recv_bcast);
    copyToCheckbutton("UserAddPhantoms", a.perms.add_phantom);
    copyToCheckbutton("UserAllowRecord", a.perms.record);
    copyToCheckbutton("UserIgnoreTimers", a.perms.inactive_exempt);
    copyToCheckbutton("UserSendComplaints", a.perms.send_complaint);
    copyToCheckbutton("UserReceiveComplaints", a.perms.recv_complaint);
    copyToCombobox("UserDuplicateIPs", a.perms.dupe_ip, 0);
    copyToCheckbutton("UserSwitchChannels", a.perms.switch_chan);
    copyToCombobox("UserDefaultChannel", a.perms.dfl_chan);
    copyToCheckbutton("UserBroadcast", a.perms.bcast);
    copyToCheckbutton("UserBroadcastLobby", a.perms.bcast_lobby);
    copyToCheckbutton("UserBroadcastU2U", a.perms.bcast_user);
    copyToCheckbutton("UserBroadcastxChan", a.perms.bcast_x_chan);
    copyToCheckbutton("UserSendTTSBinds", a.perms.send_tts_bind);
    copyToCheckbutton("UserSendWaveBinds", a.perms.send_wav_bind);
    copyToCheckbutton("UserSendPages", a.perms.send_page);
    copyToCheckbutton("UserSetPhonetic", a.perms.set_phon_name);
    copyToCheckbutton("UserSendComment", a.perms.send_comment);
    copyToCheckbutton("UserGenCommentSounds", a.perms.gen_comment_snds);
    copyToCheckbutton("UserEventSounds", a.perms.event_snds);
    copyToCheckbutton("UserMuteGlobally", a.perms.mute_glbl);
    copyToCheckbutton("UserMuteOthers", a.perms.mute_other);
    copyToCheckbutton("UserGlobalChat", a.perms.glbl_chat);
    copyToCheckbutton("UserPrivateChat", a.perms.start_priv_chat);
    setWidgetSensitive("UserPrivateChat", isLicensed);
    copyToCheckbutton("UserEqOut", a.perms.eq_out);
    copyToCheckbutton("UserSeeGuests", a.perms.see_guest);
    copyToCheckbutton("UserSeeNonGuests", a.perms.see_nonguest);
    copyToCheckbutton("UserSeeMOTD", a.perms.see_motd);
    copyToCheckbutton("UserSeeServerComment", a.perms.see_srv_comment);
    copyToCheckbutton("UserSeeChannelList", a.perms.see_chan_list);
    copyToCheckbutton("UserSeeChannelComments", a.perms.see_chan_comment);
    copyToCheckbutton("UserSeeUserComments", a.perms.see_user_comment);
    copyToCheckbutton("UserServerAdmin", a.perms.srv_admin);
    copyToCheckbutton("UserRemoveUsers", a.perms.del_user);
    copyToCheckbutton("UserAddUsers", a.perms.add_user);
    copyToCheckbutton("UserBanUsers", a.perms.ban_user);
    copyToCheckbutton("UserKickUsers", a.perms.kick_user);
    copyToCheckbutton("UserMoveUsers", a.perms.move_user);
    copyToCheckbutton("UserAssignChanAdmin", a.perms.assign_chan_admin);
    copyToCheckbutton("UserAssignRank", a.perms.assign_rank);
    copyToCheckbutton("UserEditRanks", a.perms.edit_rank);
    copyToCheckbutton("UserEditMOTD", a.perms.edit_motd);
    copyToCheckbutton("UserEditGuestMOTD", a.perms.edit_guest_motd);
    copyToCheckbutton("UserIssueRcon", a.perms.issue_rcon_cmd);
    copyToCheckbutton("UserEditVoiceTargets", a.perms.edit_voice_target);
    setWidgetSensitive("UserEditVoiceTargets", isLicensed);
    copyToCheckbutton("UserEditCommandTargets", a.perms.edit_command_target);
    setWidgetSensitive("UserEditCommandTargets", isLicensed);
    copyToCheckbutton("UserAssignReserved", a.perms.assign_reserved);
    if (!isTemplate) {
        setAdminCheckTree(UserChanAdminModel->children(), a.chan_admin, a.chan_admin_count);
        setAdminCheckTree(UserChanAuthModel->children(), a.chan_auth, a.chan_auth_count);
        builder->get_widget("UserEditorLabel", label);
        label->set_text("Editing: " + ((account) ? c_to_ustring(a.username) : "NONE"));
    }
}/*}}}*/
void
ManglerAdmin::setAdminCheckTree(Gtk::TreeModel::Children children, uint16_t *chanids, int chan_count) {/*{{{*/
    Gtk::TreeModel::iterator iter = children.begin();
    Gtk::TreeModel::Row row;
    uint32_t rowId;
    int i;
    bool found;
    while (iter != children.end()) {
        row = *iter;
        rowId = row[adminCheckRecord.id];
        found = false;
        for (i = 0; i < chan_count; ++i) {
            if (chanids[i] == rowId) {
                found = true;
                break;
            }
        }
        row[adminCheckRecord.on] = found;
        if (row.children().size()) setAdminCheckTree(row->children(), chanids, chan_count);
        iter++;
    }
}/*}}}*/
void
ManglerAdmin::getAdminCheckTree(Gtk::TreeModel::Children children, std::vector<uint16_t> &chanids) {/*{{{*/
    Gtk::TreeModel::iterator iter = children.begin();
    Gtk::TreeModel::Row row;
    uint32_t rowId;
    bool rowOn;
    while (iter != children.end()) {
        row = *iter;
        rowId = row[adminCheckRecord.id];
        rowOn = row[adminCheckRecord.on];
        if (rowOn) chanids.push_back(rowId);
        if (row.children().size()) getAdminCheckTree(row->children(), chanids);
        iter++;
    }
}/*}}}*/
void
ManglerAdmin::getAdminCheckTree(Gtk::TreeModel::Children children, uint16_t *&chanids, int &chan_count) {/*{{{*/
    std::vector<uint16_t> chanvec;
    getAdminCheckTree(children, chanvec);
    chan_count = chanvec.size();
    if (chan_count) {
        chanids = (uint16_t*)::malloc(sizeof(uint16_t) * chanvec.size());
        if (chanids) {
            for (int i = 0; i < chan_count; ++i) chanids[i] = chanvec[i];
        } else chan_count = 0; // failed malloc
    } else chanids = NULL;
}/*}}}*/
void
ManglerAdmin::UserAdd_clicked_cb(void) {/*{{{*/
    UserEditorTree->get_selection()->unselect_all();
    v3_account a;
    memset(&a, 0, sizeof(v3_account));
    a.perms.recv_bcast       = true;
    a.perms.add_phantom      = true;
    a.perms.record           = true;
    a.perms.send_complaint   = true;
    a.perms.switch_chan      = true;
    a.perms.bcast            = true;
    a.perms.bcast_lobby      = true;
    a.perms.bcast_user       = true;
    a.perms.bcast_x_chan     = true;
    a.perms.send_tts_bind    = true;
    a.perms.send_wav_bind    = true;
    a.perms.send_page        = true;
    a.perms.set_phon_name    = true;
    a.perms.send_comment     = true;
    a.perms.gen_comment_snds = true;
    a.perms.event_snds       = true;
    a.perms.glbl_chat        = true;
    a.perms.start_priv_chat  = true;
    a.perms.see_guest        = true;
    a.perms.see_nonguest     = true;
    a.perms.see_motd         = true;
    a.perms.see_srv_comment  = true;
    a.perms.see_chan_list    = true;
    a.perms.see_chan_comment = true;
    a.perms.see_user_comment = true;
    populateUserEditor(NULL);
    populateUserEditor(&a, true);
    builder->get_widget("UserEditorLabel", label);
    label->set_text("Editing: NEW USER");
    currentUserID = 0xffff;
    // enable or disable editor and necessary buttons
    UserUpdate->set_sensitive(true);
    UserEditor->set_sensitive(true);
    UserRemove->set_sensitive(false);
    UserAdd->set_sensitive(false);
}/*}}}*/
void
ManglerAdmin::UserRemove_clicked_cb(void) {/*{{{*/
    v3_account *account = v3_get_account(currentUserID);
    if (!account) return;
    Gtk::MessageDialog confirmDlg( Glib::ustring::compose("Are you sure you want to remove user %1 (%2)?", Glib::ustring::format(account->perms.account_id), c_to_ustring(account->username)), false, Gtk::MESSAGE_QUESTION, Gtk::BUTTONS_YES_NO, true );
    v3_free_account(account);
    if (confirmDlg.run() == Gtk::RESPONSE_YES) {
        v3_userlist_remove(currentUserID);
    }
}/*}}}*/
void
ManglerAdmin::UserUpdate_clicked_cb(void) {/*{{{*/
    v3_account account;
    ::memset(&account, 0, sizeof(v3_account));
    account.perms.account_id = (currentUserID == 0xffff) ? 0 : currentUserID;
    account.username = ::strdup(ustring_to_c(getFromEntry("UserLogin")).c_str());
    Glib::ustring password = trimString(getFromEntry("UserPassword"));
    if (password.length()) {
        _v3_hash_password((uint8_t *)ustring_to_c(password).c_str(), (uint8_t *)account.perms.hash_password);
    }
    account.perms.rank_id = getFromCombobox("UserRank", 0);
    uint16_t ownerID = getFromCombobox("UserOwner", 0);
    if (ownerID) {
        Gtk::TreeModel::Row ownerRow = getAccount(ownerID, UserEditorTreeModel->children());
        Glib::ustring ownerName = ownerRow[adminRecord.name];
        account.owner = ::strdup(ustring_to_c(ownerName).c_str());
    } else {
        account.owner = ::strdup("");
    }
    Gtk::TextView *textview;
    builder->get_widget("UserNotes", textview);
    account.notes = ::strdup(ustring_to_c(textview->get_buffer()->get_text()).c_str());
    account.perms.lock_acct = getFromCheckbutton("UserLocked");
    account.lock_reason = ::strdup(ustring_to_c(getFromEntry("UserLockedReason")).c_str());
    account.perms.in_reserve_list = getFromCheckbutton("UserInReservedList");
    account.perms.recv_bcast = getFromCheckbutton("UserReceiveBroadcasts");
    account.perms.add_phantom = getFromCheckbutton("UserAddPhantoms");
    account.perms.record = getFromCheckbutton("UserAllowRecord");
    account.perms.inactive_exempt = getFromCheckbutton("UserIgnoreTimers");
    account.perms.send_complaint = getFromCheckbutton("UserSendComplaints");
    account.perms.recv_complaint = getFromCheckbutton("UserReceiveComplaints");
    account.perms.dupe_ip = getFromCombobox("UserDuplicateIPs", 0);
    account.perms.switch_chan = getFromCheckbutton("UserSwitchChannels");
    account.perms.dfl_chan = getFromCombobox("UserDefaultChannel");
    account.perms.bcast = getFromCheckbutton("UserBroadcast");
    account.perms.bcast_lobby = getFromCheckbutton("UserBroadcastLobby");
    account.perms.bcast_user = getFromCheckbutton("UserBroadcastU2U");
    account.perms.bcast_x_chan = getFromCheckbutton("UserBroadcastxChan");
    account.perms.send_tts_bind = getFromCheckbutton("UserSendTTSBinds");
    account.perms.send_wav_bind = getFromCheckbutton("UserSendWaveBinds");
    account.perms.send_page = getFromCheckbutton("UserSendPages");
    account.perms.set_phon_name = getFromCheckbutton("UserSetPhonetic");
    account.perms.send_comment = getFromCheckbutton("UserSendComment");
    account.perms.gen_comment_snds = getFromCheckbutton("UserGenCommentSounds");
    account.perms.event_snds = getFromCheckbutton("UserEventSounds");
    account.perms.mute_glbl = getFromCheckbutton("UserMuteGlobally");
    account.perms.mute_other = getFromCheckbutton("UserMuteOthers");
    account.perms.glbl_chat = getFromCheckbutton("UserGlobalChat");
    account.perms.start_priv_chat = getFromCheckbutton("UserPrivateChat");
    account.perms.eq_out = getFromCheckbutton("UserEqOut");
    account.perms.see_guest = getFromCheckbutton("UserSeeGuests");
    account.perms.see_nonguest = getFromCheckbutton("UserSeeNonGuests");
    account.perms.see_motd = getFromCheckbutton("UserSeeMOTD");
    account.perms.see_srv_comment = getFromCheckbutton("UserSeeServerComment");
    account.perms.see_chan_list = getFromCheckbutton("UserSeeChannelList");
    account.perms.see_chan_comment = getFromCheckbutton("UserSeeChannelComments");
    account.perms.see_user_comment = getFromCheckbutton("UserSeeUserComments");
    account.perms.srv_admin = getFromCheckbutton("UserServerAdmin");
    account.perms.del_user = getFromCheckbutton("UserRemoveUsers");
    account.perms.add_user = getFromCheckbutton("UserAddUsers");
    account.perms.ban_user = getFromCheckbutton("UserBanUsers");
    account.perms.kick_user = getFromCheckbutton("UserKickUsers");
    account.perms.move_user = getFromCheckbutton("UserMoveUsers");
    account.perms.assign_chan_admin = getFromCheckbutton("UserAssignChanAdmin");
    account.perms.assign_rank = getFromCheckbutton("UserAssignRank");
    account.perms.edit_rank = getFromCheckbutton("UserEditRanks");
    account.perms.edit_motd = getFromCheckbutton("UserEditMOTD");
    account.perms.edit_guest_motd = getFromCheckbutton("UserEditGuestMOTD");
    account.perms.issue_rcon_cmd = getFromCheckbutton("UserIssueRcon");
    account.perms.edit_voice_target = getFromCheckbutton("UserEditVoiceTargets");
    account.perms.edit_command_target = getFromCheckbutton("UserEditCommandTargets");
    account.perms.assign_reserved = getFromCheckbutton("UserAssignReserved");
    getAdminCheckTree(UserChanAdminModel->children(), account.chan_admin, account.chan_admin_count);
    getAdminCheckTree(UserChanAuthModel->children(), account.chan_auth, account.chan_auth_count);
    v3_userlist_update(&account);
    ::free(account.username);
    ::free(account.owner);
    ::free(account.notes);
    ::free(account.lock_reason);
    if (account.chan_admin_count) ::free(account.chan_admin);
    if (account.chan_auth_count) ::free(account.chan_auth);
}/*}}}*/
void
ManglerAdmin::UserInfoButton_toggled_cb(void) {/*{{{*/
    builder->get_widget("UserInfoButton", togglebutton);
    builder->get_widget("UserInfoArrow", arrow);
    if (togglebutton->get_active()) {
        UserInfoSection->show();
        arrow->set(Gtk::ARROW_DOWN, Gtk::SHADOW_NONE);
    } else {
        UserInfoSection->hide();
        arrow->set(Gtk::ARROW_RIGHT, Gtk::SHADOW_NONE);
    }
}/*}}}*/
void
ManglerAdmin::UserNetworkButton_toggled_cb(void) {/*{{{*/
    builder->get_widget("UserNetworkButton", togglebutton);
    builder->get_widget("UserNetworkArrow", arrow);
    if (togglebutton->get_active()) {
        UserNetworkSection->show();
        arrow->set(Gtk::ARROW_DOWN, Gtk::SHADOW_NONE);
    } else {
        UserNetworkSection->hide();
        arrow->set(Gtk::ARROW_RIGHT, Gtk::SHADOW_NONE);
    }
}/*}}}*/
void
ManglerAdmin::UserTransmitButton_toggled_cb(void) {/*{{{*/
    builder->get_widget("UserTransmitButton", togglebutton);
    builder->get_widget("UserTransmitArrow", arrow);
    if (togglebutton->get_active()) {
        UserTransmitSection->show();
        arrow->set(Gtk::ARROW_DOWN, Gtk::SHADOW_NONE);
    } else {
        UserTransmitSection->hide();
        arrow->set(Gtk::ARROW_RIGHT, Gtk::SHADOW_NONE);
    }
}/*}}}*/
void
ManglerAdmin::UserDisplayButton_toggled_cb(void) {/*{{{*/
    builder->get_widget("UserDisplayButton", togglebutton);
    builder->get_widget("UserDisplayArrow", arrow);
    if (togglebutton->get_active()) {
        UserDisplaySection->show();
        arrow->set(Gtk::ARROW_DOWN, Gtk::SHADOW_NONE);
    } else {
        UserDisplaySection->hide();
        arrow->set(Gtk::ARROW_RIGHT, Gtk::SHADOW_NONE);
    }
}/*}}}*/
void
ManglerAdmin::UserAdminButton_toggled_cb(void) {/*{{{*/
    builder->get_widget("UserAdminButton", togglebutton);
    builder->get_widget("UserAdminArrow", arrow);
    if (togglebutton->get_active()) {
        UserAdminSection->show();
        arrow->set(Gtk::ARROW_DOWN, Gtk::SHADOW_NONE);
    } else {
        UserAdminSection->hide();
        arrow->set(Gtk::ARROW_RIGHT, Gtk::SHADOW_NONE);
    }
}/*}}}*/
void
ManglerAdmin::UserChanAdminButton_toggled_cb(void) {/*{{{*/
    builder->get_widget("UserChanAdminButton", togglebutton);
    builder->get_widget("UserChanAdminArrow", arrow);
    if (togglebutton->get_active()) {
        UserChanAdminTree->show();
        arrow->set(Gtk::ARROW_DOWN, Gtk::SHADOW_NONE);
    } else {
        UserChanAdminTree->hide();
        arrow->set(Gtk::ARROW_RIGHT, Gtk::SHADOW_NONE);
    }
}/*}}}*/
void
ManglerAdmin::UserChanAuthButton_toggled_cb(void) {/*{{{*/
    builder->get_widget("UserChanAuthButton", togglebutton);
    builder->get_widget("UserChanAuthArrow", arrow);
    if (togglebutton->get_active()) {
        UserChanAuthTree->show();
        arrow->set(Gtk::ARROW_DOWN, Gtk::SHADOW_NONE);
    } else {
        UserChanAuthTree->hide();
        arrow->set(Gtk::ARROW_RIGHT, Gtk::SHADOW_NONE);
    }
}/*}}}*/
void
ManglerAdmin::clearUsers(void) {/*{{{*/
    UserEditorTreeModel->clear();
    queue_resize(UserEditorTree);
    populateUserEditor(NULL);
    currentUserID = 0;
    UserUpdate->set_sensitive(false);
    UserEditor->set_sensitive(false);
    UserRemove->set_sensitive(false);
    UserAdd->set_sensitive(false);
    UserOwnerModel->clear();
    Gtk::TreeModel::Row row = *(UserOwnerModel->append());
    row[adminRecord.id] = 0;
    row[adminRecord.name] = "(None)";
}/*}}}*/
/* user editor 'profile' methods */
void
ManglerAdmin::loadUserTemplates(void) {/*{{{*/
    tmpldir = ManglerConfig::confdir() + "/templates";
    UserTemplateModel->clear();
    Gtk::TreeStore::Row row = *(UserTemplateModel->append());
    row[adminRecord.id] = 0;
    row[adminRecord.name] = "Default Admin";
    row = *(UserTemplateModel->append());
    row[adminRecord.id] = 1;
    row[adminRecord.name] = "Default User";
    row = *(UserTemplateModel->append());
    row[adminRecord.id] = 2;
    row[adminRecord.name] = "(None)";
    UserTemplate->set_active(2);
    DIR *testdir;
    if ((testdir = opendir(tmpldir.c_str()))) {
        closedir(testdir);
    } else if (mkdir(tmpldir.c_str(), 0700)) {
        tmpldir = "";
        return;
    }
    Glib::Dir dir(tmpldir);
    int ctr = 3;
    for (Glib::DirIterator iter = dir.begin(); iter != dir.end(); iter++) {
        row = *(UserTemplateModel->append());
        row[adminRecord.id] = ctr++;
        row[adminRecord.name] = *iter;
    }
}/*}}}*/
void
ManglerAdmin::UserTemplate_changed_cb(void) {/*{{{*/
    Gtk::TreeStore::iterator iter = UserTemplate->get_active();
    setWidgetSensitive("UserTemplateDelete", tmpldir.length() && iter && (*iter)[adminRecord.id] > 2);
}/*}}}*/
void
ManglerAdmin::UserTemplateLoad_clicked_cb(void) {/*{{{*/
    Gtk::TreeStore::iterator iter = UserTemplate->get_active();
    if (!iter) {
        return;
    }
    uint32_t id = (*iter)[adminRecord.id];
    Glib::ustring name = (*iter)[adminRecord.name];
    v3_account a;
    memset(&a, 0, sizeof(v3_account));
    if (id < 2) {
        a.perms.recv_bcast       = true;
        a.perms.add_phantom      = true;
        a.perms.record           = true;
        a.perms.send_complaint   = true;
        a.perms.switch_chan      = true;
        a.perms.bcast            = true;
        a.perms.bcast_lobby      = true;
        a.perms.bcast_user       = true;
        a.perms.bcast_x_chan     = true;
        a.perms.send_tts_bind    = true;
        a.perms.send_wav_bind    = true;
        a.perms.send_page        = true;
        a.perms.set_phon_name    = true;
        a.perms.send_comment     = true;
        a.perms.gen_comment_snds = true;
        a.perms.event_snds       = true;
        a.perms.glbl_chat        = true;
        a.perms.start_priv_chat  = true;
        a.perms.see_guest        = true;
        a.perms.see_nonguest     = true;
        a.perms.see_motd         = true;
        a.perms.see_srv_comment  = true;
        a.perms.see_chan_list    = true;
        a.perms.see_chan_comment = true;
        a.perms.see_user_comment = true;
        if (id == 0) {
            a.perms.in_reserve_list   = true;
            a.perms.recv_complaint    = true;
            a.perms.add_user          = true;
            a.perms.del_user          = true;
            a.perms.ban_user          = true;
            a.perms.kick_user         = true;
            a.perms.move_user         = true;
            a.perms.assign_chan_admin = true;
        }
    } else if (id > 2 && tmpldir.length() && name.length()) {
        iniFile tmplfile(tmpldir + "/" + name, true, false);
        if (!tmplfile.contains("Profile")) {
            statusbarPush("Error: No 'Profile' section found in template.");
            return;
        }
        iniSection &tmpl(tmplfile["Profile"]);
        a.perms.lock_acct           = tmpl["Locked"].toBool();
        a.perms.in_reserve_list     = tmpl["Reserved"].toBool();
        a.perms.recv_bcast          = tmpl["RecvStreams"].toBool();
        a.perms.add_phantom         = tmpl["Phantoms"].toBool();
        a.perms.record              = tmpl["Record"].toBool();
        a.perms.inactive_exempt     = tmpl["IgnoreInactivity"].toBool();
        a.perms.send_complaint      = tmpl["SendComplaints"].toBool();
        a.perms.recv_complaint      = tmpl["RecvComplaints"].toBool();
        a.perms.switch_chan         = tmpl["SwitchChannels"].toBool();
        a.lock_reason               = ::strdup(ustring_to_c(tmpl["LockedReason"].toUString()).c_str());
        a.perms.dfl_chan            = v3_get_channel_id(ustring_to_c(tmpl["DefChan"].toUString()).c_str());
        a.perms.dupe_ip             = tmpl["DuplicateIPs"].toInt();
        a.perms.bcast               = tmpl["Broadcast"].toBool();
        a.perms.bcast_lobby         = tmpl["BroadcastLobby"].toBool();
        a.perms.bcast_user          = tmpl["BroadcastU2U"].toBool();
        a.perms.bcast_x_chan        = tmpl["BroadcastCrossChannel"].toBool();
        a.perms.send_tts_bind       = tmpl["SendTTS"].toBool();
        a.perms.send_wav_bind       = tmpl["SendWave"].toBool();
        a.perms.send_page           = tmpl["SendPages"].toBool();
        a.perms.set_phon_name       = tmpl["SetPhonetic"].toBool();
        a.perms.send_comment        = tmpl["SendComments"].toBool();
        a.perms.gen_comment_snds    = tmpl["CommentSounds"].toBool();
        a.perms.event_snds          = tmpl["EventSounds"].toBool();
        a.perms.mute_glbl           = tmpl["MuteGlobally"].toBool();
        a.perms.mute_other          = tmpl["MuteOthersPTT"].toBool();
        a.perms.glbl_chat           = tmpl["Chat"].toBool();
        a.perms.start_priv_chat     = tmpl["InitPrivateChat"].toBool();
        a.perms.eq_out              = tmpl["Equalizer"].toBool();
        a.perms.see_guest           = tmpl["SeeGuest"].toBool();
        a.perms.see_nonguest        = tmpl["SeeNonGuest"].toBool();
        a.perms.see_motd            = tmpl["SeeMotd"].toBool();
        a.perms.see_srv_comment     = tmpl["SeeServerComment"].toBool();
        a.perms.see_chan_list       = tmpl["SeeChannels"].toBool();
        a.perms.see_chan_comment    = tmpl["SeeChannelComments"].toBool();
        a.perms.see_user_comment    = tmpl["SeeUserComments"].toBool();
        a.perms.srv_admin           = tmpl["ServerAdmin"].toBool();
        a.perms.add_user            = tmpl["AddUsers"].toBool();
        a.perms.del_user            = tmpl["DeleteUsers"].toBool();
        a.perms.ban_user            = tmpl["BanUsers"].toBool();
        a.perms.kick_user           = tmpl["KickUsers"].toBool();
        a.perms.move_user           = tmpl["MoveUsers"].toBool();
        a.perms.assign_chan_admin   = tmpl["EditChanAdmins"].toBool();
        a.perms.edit_rank           = tmpl["EditRanks"].toBool();
        a.perms.edit_motd           = tmpl["EditMotd"].toBool();
        a.perms.edit_guest_motd     = tmpl["EditGuestMotd"].toBool();
        a.perms.issue_rcon_cmd      = tmpl["RCon"].toBool();
        a.perms.edit_voice_target   = tmpl["GroupEditVoice"].toBool();
        a.perms.edit_command_target = tmpl["GroupEditCmd"].toBool();
        a.perms.assign_rank         = tmpl["RankAssign"].toBool();
        a.perms.assign_reserved     = tmpl["ReservedAssign"].toBool();
    }
    populateUserEditor(&a, true);
    if (a.lock_reason) {
        ::free(a.lock_reason);
    }
    statusbarPush("Loaded '" + name + "' template.");
}/*}}}*/
void
ManglerAdmin::UserTemplateDelete_clicked_cb(void) {/*{{{*/
    Gtk::TreeStore::iterator iter = UserTemplate->get_active();
    if (!iter || tmpldir.empty()) {
        return;
    }
    Glib::ustring name = (*iter)[adminRecord.name];
    Gtk::MessageDialog confirm("<b>Are you sure you want to delete \"" + name + "\"?</b>",
            true, Gtk::MESSAGE_WARNING, Gtk::BUTTONS_YES_NO, true);
    if (confirm.run() == Gtk::RESPONSE_YES) {
        unlink(Glib::ustring(tmpldir + "/" + name).c_str());
        loadUserTemplates();
    }
}/*}}}*/
void
ManglerAdmin::UserTemplateSave_clicked_cb(void) {/*{{{*/
    if (!tmpldialog) {
        tmpldialog = new Gtk::FileChooserDialog("Save User Template", Gtk::FILE_CHOOSER_ACTION_SAVE);
        tmpldialog->add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
        tmpldialog->add_button(Gtk::Stock::SAVE, Gtk::RESPONSE_OK);
        tpl_filter.set_name("Ventrilo User Editor Profile (*.vuep, *.ini)");
        tpl_filter.add_pattern("*.vuep");
        tpl_filter.add_pattern("*.ini");
        tmpldialog->add_filter(tpl_filter);
        all_filter.set_name("All Files");
        all_filter.add_pattern("*");
        tmpldialog->add_filter(all_filter);
        if (tmpldir.length()) {
            tmpldialog->set_current_folder(tmpldir);
        }
    }
    int result = tmpldialog->run();
    tmpldialog->hide();
    if (result != Gtk::RESPONSE_OK) {
        return;
    }
    Glib::ustring filename = tmpldialog->get_filename();
    if (tmpldialog->get_filter() == &tpl_filter) {
        Glib::PatternSpec vuep("*.vuep"), ini("*.ini");
        if (!vuep.match(filename) && !ini.match(filename)) {
            filename.append(".vuep");
        }
    }
    Glib::ustring name = filename.substr(tmpldialog->get_current_folder().length() + 1);
    char *path = v3_get_channel_path(getFromCombobox("UserDefaultChannel", 0));
    iniFile tmplfile(filename, false, false);
    iniSection &tmpl(tmplfile["Profile"]);
    tmpl["Locked"]                = (int)getFromCheckbutton("UserLocked");
    tmpl["Reserved"]              = (int)getFromCheckbutton("UserInReservedList");
    tmpl["RecvStreams"]           = (int)getFromCheckbutton("UserReceiveBroadcasts");
    tmpl["Phantoms"]              = (int)getFromCheckbutton("UserAddPhantoms");
    tmpl["Record"]                = (int)getFromCheckbutton("UserAllowRecord");
    tmpl["IgnoreInactivity"]      = (int)getFromCheckbutton("UserIgnoreTimers");
    tmpl["SendComplaints"]        = (int)getFromCheckbutton("UserSendComplaints");
    tmpl["RecvComplaints"]        = (int)getFromCheckbutton("UserReceiveComplaints");
    tmpl["SwitchChannels"]        = (int)getFromCheckbutton("UserSwitchChannels");
    tmpl["LockedReason"]          = getFromEntry("UserLockedReason");
    tmpl["DefChan"]               = c_to_ustring(path);
    tmpl["DuplicateIPs"]          = (int)getFromCombobox("UserDuplicateIPs", 0);
    tmpl["Broadcast"]             = (int)getFromCheckbutton("UserBroadcast");
    tmpl["BroadcastLobby"]        = (int)getFromCheckbutton("UserBroadcastLobby");
    tmpl["BroadcastU2U"]          = (int)getFromCheckbutton("UserBroadcastU2U");
    tmpl["BroadcastCrossChannel"] = (int)getFromCheckbutton("UserBroadcastxChan");
    tmpl["SendTTS"]               = (int)getFromCheckbutton("UserSendTTSBinds");
    tmpl["SendWave"]              = (int)getFromCheckbutton("UserSendWaveBinds");
    tmpl["SendPages"]             = (int)getFromCheckbutton("UserSendPages");
    tmpl["SetPhonetic"]           = (int)getFromCheckbutton("UserSetPhonetic");
    tmpl["SendComments"]          = (int)getFromCheckbutton("UserSendComment");
    tmpl["CommentSounds"]         = (int)getFromCheckbutton("UserGenCommentSounds");
    tmpl["EventSounds"]           = (int)getFromCheckbutton("UserEventSounds");
    tmpl["MuteGlobally"]          = (int)getFromCheckbutton("UserMuteGlobally");
    tmpl["MuteOthersPTT"]         = (int)getFromCheckbutton("UserMuteOthers");
    tmpl["Chat"]                  = (int)getFromCheckbutton("UserGlobalChat");
    tmpl["InitPrivateChat"]       = (int)getFromCheckbutton("UserPrivateChat");
    tmpl["Equalizer"]             = (int)getFromCheckbutton("UserEqOut");
    tmpl["SeeGuest"]              = (int)getFromCheckbutton("UserSeeGuests");
    tmpl["SeeNonGuest"]           = (int)getFromCheckbutton("UserSeeNonGuests");
    tmpl["SeeMotd"]               = (int)getFromCheckbutton("UserSeeMOTD");
    tmpl["SeeServerComment"]      = (int)getFromCheckbutton("UserSeeServerComment");
    tmpl["SeeChannels"]           = (int)getFromCheckbutton("UserSeeChannelList");
    tmpl["SeeChannelComments"]    = (int)getFromCheckbutton("UserSeeChannelComments");
    tmpl["SeeUserComments"]       = (int)getFromCheckbutton("UserSeeUserComments");
    tmpl["ServerAdmin"]           = (int)getFromCheckbutton("UserServerAdmin");
    tmpl["AddUsers"]              = (int)getFromCheckbutton("UserAddUsers");
    tmpl["DeleteUsers"]           = (int)getFromCheckbutton("UserRemoveUsers");
    tmpl["BanUsers"]              = (int)getFromCheckbutton("UserBanUsers");
    tmpl["KickUsers"]             = (int)getFromCheckbutton("UserKickUsers");
    tmpl["MoveUsers"]             = (int)getFromCheckbutton("UserMoveUsers");
    tmpl["EditChanAdmins"]        = (int)getFromCheckbutton("UserAssignChanAdmin");
    tmpl["EditRanks"]             = (int)getFromCheckbutton("UserEditRanks");
    tmpl["EditMotd"]              = (int)getFromCheckbutton("UserEditMOTD");
    tmpl["EditGuestMotd"]         = (int)getFromCheckbutton("UserEditGuestMOTD");
    tmpl["RCon"]                  = (int)getFromCheckbutton("UserIssueRcon");
    tmpl["GroupEditVoice"]        = (int)getFromCheckbutton("UserEditVoiceTargets");
    tmpl["GroupEditCmd"]          = (int)getFromCheckbutton("UserEditCommandTargets");
    tmpl["RankAssign"]            = (int)getFromCheckbutton("UserAssignRank");
    tmpl["ReservedAssign"]        = (int)getFromCheckbutton("UserAssignReserved");
    tmplfile.save();
    if (path) {
        ::free(path);
    }
    loadUserTemplates();
    statusbarPush("Saved '" + name + "' template.");
}/*}}}*/

/* ----------  Rank Editor Related Methods  ---------- */
Gtk::TreeModel::iterator
ManglerAdmin::getRank(uint16_t id, Gtk::TreeModel::Children children) {/*{{{*/
    Gtk::TreeModel::iterator iter = children.begin();
    while (iter != children.end()) {
        if ((*iter)[rankRecord.id] == id) break;
        iter++;
    }
    return iter;
}/*}}}*/
void
ManglerAdmin::rankUpdated(v3_rank *rank) {/*{{{*/
    Gtk::TreeModel::iterator iter = getRank(rank->id, RankEditorModel->children());
    if (! iter) iter = RankEditorModel->append();
    (*iter)[rankRecord.id] = rank->id;
    (*iter)[rankRecord.level] = rank->level;
    (*iter)[rankRecord.name] = c_to_ustring(rank->name);
    (*iter)[rankRecord.description] = c_to_ustring(rank->description);
    queue_resize(RankEditorTree);
    /* now handle the User Rank combo box */
    /* the poorly named getAccount() will work fine for this */
    Gtk::TreeModel::Row row = getAccount(rank->id, UserRankModel->children());
    if (! row) row = *(UserRankModel->append());
    row[adminRecord.id] = rank->id;
    row[adminRecord.name] = c_to_ustring(rank->name);
    /* update status bar */
    statusbarPush(Glib::ustring::compose("Rank %1 (%2) updated.", Glib::ustring::format(rank->id), c_to_ustring(rank->name)));
}/*}}}*/
void
ManglerAdmin::rankAdded(v3_rank *rank) {/*{{{*/
    Gtk::TreeModel::iterator iter = RankEditorModel->append();
    (*iter)[rankRecord.id] = rank->id;
    (*iter)[rankRecord.level] = rank->level;
    (*iter)[rankRecord.name] = c_to_ustring(rank->name);
    (*iter)[rankRecord.description] = rank->description;
    if (currentRankID == 0xffff && c_to_ustring(rank->name) == trimString(getFromEntry("RankName"))) {
        RankEditorTree->set_cursor(RankEditorModel->get_path(iter));
    }
    queue_resize(RankEditorTree);
    /* now handle the User Rank combo box */
    iter = UserRankModel->append();
    (*iter)[adminRecord.id] = rank->id;
    (*iter)[adminRecord.name] = c_to_ustring(rank->name);
    /* update status bar */
    statusbarPush(Glib::ustring::compose("Rank %1 (%2) added.", Glib::ustring::format(rank->id), c_to_ustring(rank->name)));
}/*}}}*/
void
ManglerAdmin::rankRemoved(uint16_t rankid) {/*{{{*/
    Gtk::TreeModel::iterator iter = getRank(rankid, RankEditorModel->children());
    if (iter) {
        Gtk::TreeModel::Row rank = *iter;
        iter = RankEditorTree->get_selection()->get_selected();
        if (iter && (*iter)[rankRecord.id] == rank[rankRecord.id]) {
            clearRankEditor();
        }
        RankEditorModel->erase(rank);
    }
    queue_resize(RankEditorTree);
    /* now handle the User Rank combo box */
    /* the poorly named getAccount() will work fine for this */
    Gtk::TreeModel::Row row = getAccount(rankid, UserRankModel->children());
    if (row) UserRankModel->erase(row);
    /* update status bar */
    statusbarPush(Glib::ustring::compose("Rank %1 removed.", Glib::ustring::format(rankid)));
}/*}}}*/
void
ManglerAdmin::rankRemoved(v3_rank *rank) {/*{{{*/
    rankRemoved(rank->id);
}/*}}}*/
void
ManglerAdmin::RankUpdate_clicked_cb(void) {/*{{{*/
    v3_rank rank;
    rank.id = (currentRankID == 0xffff) ? 0 : currentRankID;
    rank.name = ::strdup(ustring_to_c(getFromEntry("RankName")).c_str());
    rank.description = ::strdup(ustring_to_c(getFromEntry("RankDescription")).c_str());
    rank.level = uint16_t( getFromSpinbutton("RankLevel") );
    v3_rank_update(&rank);
    ::free(rank.name);
    ::free(rank.description);
}/*}}}*/
void
ManglerAdmin::RankEditorTree_cursor_changed_cb(void) {/*{{{*/
    Gtk::TreeModel::iterator iter = RankEditorTree->get_selection()->get_selected();
    bool isRank( false );
    if (iter) {
        isRank = true;
        currentRankID = (*iter)[rankRecord.id];
        copyToEntry("RankName", (*iter)[rankRecord.name]);
        copyToEntry("RankDescription", (*iter)[rankRecord.description]);
        copyToSpinbutton("RankLevel", (*iter)[rankRecord.level]);
    }
    const v3_permissions *perms = v3_get_permissions();
    bool editAccess( RankEditorOpen && perms->edit_rank );
    setWidgetSensitive("RankAdd", editAccess);
    setWidgetSensitive("RankRemove", editAccess && isRank);
    setWidgetSensitive("RankUpdate", editAccess && isRank);
    RankEditor->set_sensitive(isRank);
}/*}}}*/
void
ManglerAdmin::RankAdd_clicked_cb(void) {/*{{{*/
    clearRankEditor();
    currentRankID = 0xffff;
    setWidgetSensitive("RankAdd", false);
    setWidgetSensitive("RankUpdate", true);
    RankEditor->set_sensitive(true);
}/*}}}*/
void
ManglerAdmin::RankRemove_clicked_cb(void) {/*{{{*/
    Gtk::TreeModel::iterator iter = RankEditorTree->get_selection()->get_selected();
    if (! iter) return;
    uint16_t rankid = (*iter)[rankRecord.id];
    Glib::ustring rankname = (*iter)[rankRecord.name];
    Gtk::MessageDialog confirmDlg( Glib::ustring::compose("Are you sure you want to remove rank %1 (%2)?", Glib::ustring::format(rankid), rankname), false, Gtk::MESSAGE_QUESTION, Gtk::BUTTONS_YES_NO, true );
    if (confirmDlg.run() == Gtk::RESPONSE_YES) {
        v3_rank_remove(rankid);
    }
}/*}}}*/
void
ManglerAdmin::clearRankEditor(void) {/*{{{*/
    RankEditorTree->get_selection()->unselect_all();
    RankEditor->set_sensitive(false);
    copyToEntry("RankName", "");
    copyToEntry("RankDescription", "");
    copyToSpinbutton("RankLevel", 0);
    setWidgetSensitive("RankAdd", RankEditorOpen);
    setWidgetSensitive("RankRemove", false);
    setWidgetSensitive("RankUpdate", RankEditorOpen);
    currentRankID = 0;
}/*}}}*/
void
ManglerAdmin::clearRanks(void) {/*{{{*/
    RankEditorModel->clear();
    queue_resize(RankEditorTree);
    clearRankEditor();
    UserRankModel->clear();
    Gtk::TreeModel::Row row = *(UserRankModel->append());
    row[adminRecord.id] = 0;
    row[adminRecord.name] = "(None)";
}/*}}}*/

/* ----------  Ban Editor Related Methods  ---------- */
void
ManglerAdmin::banList(uint16_t id, uint16_t count, uint16_t bitmask_id, uint32_t ip_address, char *user, char *by, char *reason) {/*{{{*/
    if (!id) {
        BanEditorModel->clear();
        queue_resize(BanEditorTree);
    }
    if (!count) {
        clearBanEditor();
        return;
    }
    Gtk::TreeModel::iterator iter = BanEditorModel->append();
    (*iter)[banRecord.id] = id;
    (*iter)[banRecord.ip_val] = ip_address;
    (*iter)[banRecord.netmask_id] = bitmask_id;
    (*iter)[banRecord.ip] = Glib::ustring::compose("%1.%2.%3.%4",
            Glib::ustring::format((ip_address >> 24) & 0xff),
            Glib::ustring::format((ip_address >> 16) & 0xff),
            Glib::ustring::format((ip_address >> 8) & 0xff),
            Glib::ustring::format(ip_address & 0xff));
    (*iter)[banRecord.netmask] = c_to_ustring(_v3_bitmasks[bitmask_id]);
    (*iter)[banRecord.user] = c_to_ustring(user);
    (*iter)[banRecord.by] = c_to_ustring(by);
    (*iter)[banRecord.reason] = c_to_ustring(reason);
    if (ip_address == currentBanIP) {
        BanEditorTree->set_cursor(BanEditorModel->get_path(iter));
    }
    queue_resize(BanEditorTree);
}/*}}}*/
void
ManglerAdmin::BanEditorTree_cursor_changed_cb(void) {/*{{{*/
    Gtk::TreeModel::iterator iter = BanEditorTree->get_selection()->get_selected();
    bool isBan( false );
    if (iter) {
        isBan = true;
        currentBanIP = (*iter)[banRecord.ip_val];
        copyToEntry("BanIPAddress", (*iter)[banRecord.ip]);
        copyToCombobox("BanNetmask", (*iter)[banRecord.netmask_id], BanNetmaskDefault);
        copyToEntry("BanReason", (*iter)[banRecord.reason]);
    }
    setWidgetSensitive("BanAdd", true);
    setWidgetSensitive("BanRemove", isBan);
    BanEditor->set_sensitive(isBan);
}/*}}}*/
void
ManglerAdmin::BanAdd_clicked_cb(void) {/*{{{*/
    clearBanEditor();
    setWidgetSensitive("BanAdd", false);
    BanEditor->set_sensitive(true);
}/*}}}*/
void
ManglerAdmin::BanRemove_clicked_cb(void) {/*{{{*/
    Gtk::TreeModel::iterator iter = BanEditorTree->get_selection()->get_selected();
    if (! iter) return;
    uint32_t ip_address = (*iter)[banRecord.ip_val];
    uint16_t bitmask_id = (*iter)[banRecord.netmask_id];
    clearBanEditor();
    v3_admin_ban_remove(bitmask_id, ip_address);
    v3_admin_ban_list();
}/*}}}*/
void
ManglerAdmin::BanUpdate_clicked_cb(void) {/*{{{*/
    uint32_t ip_address = 0;
    char *ip_str = ::strdup(ustring_to_c(getFromEntry("BanIPAddress")).c_str());
    uint16_t b1 = 0, b2 = 0, b3 = 0, b4 = 0;
    if (::sscanf(ip_str, "%hu.%hu.%hu.%hu", &b1, &b2, &b3, &b4) != 4) {
        ::free(ip_str);
        statusbarPush("Invalid IPv4 address.");
        return;
    }
    ::free(ip_str);
    currentBanIP = ip_address =
            ((b1 << 24) & 0xff000000) |
            ((b2 << 16) & 0x00ff0000) |
            ((b3 <<  8) & 0x0000ff00) |
            ((b4)       & 0x000000ff);
    uint16_t bitmask_id = getFromCombobox("BanNetmask", BanNetmaskDefault);
    char *reason = ::strdup(ustring_to_c(getFromEntry("BanReason")).c_str());
    v3_admin_ban_add(bitmask_id, ip_address, "Remotely added IP", reason);
    ::free(reason);
    v3_admin_ban_list();
}/*}}}*/
void
ManglerAdmin::clearBanEditor(void) {/*{{{*/
    BanEditorTree->get_selection()->unselect_all();
    BanEditor->set_sensitive(false);
    copyToEntry("BanIPAddress", "");
    copyToCombobox("BanNetmask", BanNetmaskDefault);
    copyToEntry("BanReason", "");
    setWidgetSensitive("BanAdd", true);
    setWidgetSensitive("BanRemove", false);
    currentBanIP = -1;
}/*}}}*/
void
ManglerAdmin::clearBans(void) {/*{{{*/
    BanEditorModel->clear();
    queue_resize(BanEditorTree);
    clearBanEditor();
}/*}}}*/

void
ManglerAdmin::clear(void) {/*{{{*/
    adminNotebook->set_current_page(1);
    setWidgetSensitive("ServerVBox", true);
    SrvIsNotUpdating = true;
    ChannelAdded = false;
    SrvEditorOpen = false;
    UserEditorOpen = false;
    RankEditorOpen = false;
    clearChannels();
    clearUsers();
    clearRanks();
    clearBans();
}/*}}}*/

