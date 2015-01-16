/*
 * vim: softtabstop=4 shiftwidth=4 cindent foldmethod=marker expandtab
 *
 * $LastChangedDate: 2011-06-27 23:50:10 +0430 (Mon, 27 Jun 2011) $
 * $Revision: 1139 $
 * $LastChangedBy: econnell $
 * $URL: http://svn.mangler.org/mangler/trunk/src/channeltree.h $
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

#ifndef _CHANNELTREE_H
#define _CHANNELTREE_H
#include <sys/types.h>

class channelModelColumns : public Gtk::TreeModel::ColumnRecord/*{{{*/
{
    public:
        channelModelColumns() {
            add(displayName);
            add(icon);
            add(isUser);
            add(isGuest);
            add(id);
            add(parent_id);
            add(name);
            add(comment);
            add(phonetic);
            add(url);
            add(integration_text);
            add(rank);
            add(last_transmit);
            add(password);
            add(muted);
            add(phantom);
        }

        Gtk::TreeModelColumn<Glib::ustring>                 displayName;
        Gtk::TreeModelColumn<Glib::RefPtr<Gdk::Pixbuf> >    icon;
        Gtk::TreeModelColumn<bool>                          isUser;
        Gtk::TreeModelColumn<bool>                          isGuest;
        Gtk::TreeModelColumn<uint32_t>                      id;
        Gtk::TreeModelColumn<uint32_t>                      parent_id;
        Gtk::TreeModelColumn<Glib::ustring>                 name;
        Gtk::TreeModelColumn<Glib::ustring>                 comment;
        Gtk::TreeModelColumn<Glib::ustring>                 phonetic;
        Gtk::TreeModelColumn<Glib::ustring>                 url;
        Gtk::TreeModelColumn<Glib::ustring>                 integration_text;
        Gtk::TreeModelColumn<Glib::ustring>                 rank;
        Gtk::TreeModelColumn<Glib::ustring>                 last_transmit;
        Gtk::TreeModelColumn<Glib::ustring>                 password;
        Gtk::TreeModelColumn<bool>                          muted;
        Gtk::TreeModelColumn<bool>                          phantom;
};/*}}}*/
class ManglerChannelStore : public Gtk::TreeStore
{
    public:
        static Glib::RefPtr<ManglerChannelStore> create();
        channelModelColumns                 c;

    protected:
        ManglerChannelStore() {
            set_column_types(c);
        }
        virtual bool row_draggable_vfunc(const Gtk::TreeModel::Path& path) const;
        virtual bool row_drop_possible_vfunc(const Gtk::TreeModel::Path& dest, const Gtk::SelectionData& selection_data) const;
        virtual bool drag_data_received_vfunc(const Gtk::TreeModel::Path& dest, const Gtk::SelectionData& selection_data);
};

class ManglerChannelTree
{
    private:
        Glib::RefPtr<Gtk::Builder>          builder;
        channelModelColumns                 channelRecord;
        Glib::RefPtr<ManglerChannelStore>   channelStore;
        Gtk::TreeModel::iterator            channelIter;
        Gtk::TreeModel::Row                 channelRow;
        Gtk::TreeViewColumn                 *column;
        Gtk::CellRendererPixbuf             *pixrenderer;
        Gtk::CellRendererText               *textrenderer;
        Gtk::MenuItem                       *menuitem;
        Gtk::CheckMenuItem                  *checkmenuitem;
        void renderCellData(Gtk::CellRenderer *cell, const Gtk::TreeModel::iterator& iter);

    public:
        ManglerChannelTree(Glib::RefPtr<Gtk::Builder> builder);
        Gtk::TreeView                       *channelView;
        Gtk::Menu                           *rcmenu_user;
        sigc::connection                    signalMute;
        sigc::connection                    signalChannelMute;
        sigc::connection                    signalGlobalMute;
        Gtk::Menu                           *rcmenu_channel;
        sigc::connection                    signalDefaultChannel;
        Gtk::Window                         *window;
        Gtk::Label                          *label;
        Gtk::LinkButton                     *linkbutton;
        Gtk::VScale                         *volumevscale;
        Gtk::Adjustment                     *volumeAdjustment;
        sigc::connection                    volumeAdjustSignalConnection;
        Gtk::VBox                           *vbox;
        bool                                sortManual;
        void addChannel(uint8_t protect_mode, uint32_t id, uint32_t parent_id, Glib::ustring name, Glib::ustring comment = "", Glib::ustring phonetic = "");
        void addUser(uint32_t id, uint32_t channel, Glib::ustring name, Glib::ustring comment = "", Glib::ustring phonetic = "", Glib::ustring url = "", Glib::ustring integration_text = "", bool guest = false, bool phantom = false, Glib::ustring rank = "");
        void updateLobby(Glib::ustring name, Glib::ustring comment = "", Glib::ustring phonetic = "");
        void updateUser(uint32_t id, uint32_t parent_id, Glib::ustring name, Glib::ustring comment, Glib::ustring phonetic, Glib::ustring url, Glib::ustring integration_text, bool guest, bool phantom, Glib::ustring rank = "");
        void updateChannel(uint8_t protect_mode, uint32_t id, uint32_t parent_id, Glib::ustring name, Glib::ustring comment, Glib::ustring phonetic);
        void refreshChannel(uint32_t id);
        void _refreshAllChannels(Gtk::TreeModel::Children children);
        void refreshAllChannels();
        void refreshUser(uint32_t id);
        void _refreshAllUsers(Gtk::TreeModel::Children children);
        void refreshAllUsers();
        int  sortFunction(const Gtk::TreeModel::iterator& a, const Gtk::TreeModel::iterator& b);
        Glib::ustring getLastTransmit(uint16_t userid);
        void setLastTransmit(uint16_t userid, Glib::ustring last_transmit);
        void removeUser(uint32_t id);
        void removeChannel(uint32_t id);
        void setUserIcon(uint16_t id, Glib::ustring color, bool updateLastTransmit = false);
        Gtk::TreeModel::Row getChannel(uint32_t id, Gtk::TreeModel::Children children);
        Gtk::TreeModel::Row getUser(uint32_t id);
        bool _getUser(const Gtk::TreeModel::iterator &iter, uint32_t id, Gtk::TreeModel::iterator *r_iter);
        uint16_t getUserChannelId(uint16_t userid);
        Glib::ustring getChannelSavedPassword(uint16_t channel_id);
        void setChannelSavedPassword(uint16_t channel_id, Glib::ustring password);
        void forgetChannelSavedPassword(uint16_t channel_id);
        bool isMuted(uint16_t userid);
        void muteUserToggle(uint16_t userid);
        void queue_resize(void);
        void expand_all(void);
        void collapse_all(void);
        void clear(void);

        void channelView_row_activated_cb(const Gtk::TreeModel::Path& path, Gtk::TreeViewColumn* column);
        void channelView_columns_changed_cb(void);
        void userSettingsMenuItem_activate_cb(void);
        void channelView_buttonpress_event_cb(GdkEventButton* event);
        void copyCommentMenuItem_activate_cb(void);
        void copyURLMenuItem_activate_cb(void);
        void privateChatMenuItem_activate_cb(void);
        void addPhantomMenuItem_activate_cb(void);
        void removePhantomMenuItem_activate_cb(void);
        void kickUserMenuItem_activate_cb(void);
        void banUserMenuItem_activate_cb(void);
        void muteUserMenuItem_activate_cb(void);
        void muteUserChannelMenuItem_activate_cb(void);
        void muteUserGlobalMenuItem_activate_cb(void);
        void pageUserMenuItem_activate_cb(void);
        void setDefaultChannelMenuItem_activate_cb(void);
        Glib::ustring volumevscale_format_value_cb(double value);
        void volumeAdjustment_value_changed_cb(uint16_t id);
        void userSettingsWindow(Gtk::TreeModel::Row row);
};

Glib::ustring getTimeString(void);
int natsort(const char *l, const char *r);

#endif

