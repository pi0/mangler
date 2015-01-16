/*
 * vim: softtabstop=4 shiftwidth=4 cindent foldmethod=marker expandtab
 *
 * $LastChangedDate: 2011-06-27 23:50:10 +0430 (Mon, 27 Jun 2011) $
 * $Revision: 1139 $
 * $LastChangedBy: econnell $
 * $URL: http://svn.mangler.org/mangler/trunk/src/mangler.h $
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

#include "config.h"
#include <gtkmm.h>
#include <sys/types.h>
#include <stdint.h>
#include <iostream>
#include <X11/extensions/XInput.h>
#include "manglerconfig.h"

class ManglerChannelTree;
class ManglerNetwork;
class ManglerAudio;
class ManglerSettings;
class ManglerServerList;
class ManglerChat;
class ManglerPrivChat;
class ManglerIntegration;
class ManglerAdmin;
class ManglerRecorder;
#ifdef HAVE_XOSD
class ManglerOsd;
#endif
#ifdef HAVE_G15
class ManglerG15;
#endif

extern "C" {
#include <ventrilo3.h>
}

#ifndef _MANGLER_H
#define _MANGLER_H

struct _cli_options {
    bool uifromfile;
    Glib::ustring uifilename;
    Glib::ustring qc_server;
    Glib::ustring qc_username;
    Glib::ustring qc_password;
};

class Mangler
{
    public:
        Mangler(struct _cli_options *options);
        ~Mangler();
        void onDisconnectHandler(void);
        void startTransmit(void);
        void stopTransmit(void);
        void initialize(void);
        Gtk::Window                         *manglerWindow;
        Glib::RefPtr<Gtk::Builder>          builder;
        Gtk::Button                         *button;
        Gtk::ToggleButton                   *togglebutton;
        Gtk::Dialog                         *dialog;
        Gtk::AboutDialog                    *aboutdialog;
        Gtk::MessageDialog                  *msgdialog;
        Gtk::Window                         *window;
        Gtk::ProgressBar                    *progressbar;
        Gtk::Statusbar                      *statusbar;
        Gtk::Label                          *label;
        Gtk::Entry                          *entry;
        Gtk::ComboBox                       *combobox;
        Gtk::TextView                       *textview;
        Gtk::VBox                           *vbox;
        Gtk::CheckMenuItem                  *checkmenuitem;
        Gtk::MenuItem                       *menuitem;
        Gtk::Table                          *table;
        Gtk::CheckButton                    *checkbutton;
        Gtk::ProgressBar                    *inputvumeter;

        std::map<Glib::ustring, Glib::RefPtr<Gdk::Pixbuf> >  icons;
        Glib::RefPtr<Gtk::StatusIcon>       statusIcon;
        Gtk::Menu                           *statusIconMenu;
        ManglerServerList                   *serverList;
        ManglerChat                         *chat;
        ManglerChannelTree                  *channelTree;
        ManglerNetwork                      *network;
        //int32_t                             connectedServerId;
        std::string                         connectedServerName;
        std::map<uint32_t, ManglerAudio* >  outputAudio;
        std::map<uint16_t, ManglerPrivChat *> privateChatWindows;
        ManglerAudio                        *inputAudio;
        ManglerAudio                        *audioControl;
        ManglerSettings                     *settings;
        ManglerIntegration                  *integration;
        ManglerAdmin                        *admin;
        ManglerRecorder                     *recorder;
        static ManglerConfig                config;
#ifdef HAVE_XOSD
        ManglerOsd                          *osd;
#endif
#ifdef HAVE_G15
        ManglerG15                          *g15;
#endif

        bool                                isTransmitting;
        bool                                isTransmittingButton;
        bool                                isTransmittingVA;
        bool                                isTransmittingKey;
        bool                                isTransmittingMouse;
        bool                                iconified;
        bool                                isAdmin;
        bool                                isChanAdmin;
        bool                                muteSound;
        bool                                muteMic;
        bool                                motdAlways;
        bool                                wantAdminWindow;
        int                                 espeakRate;

        Glib::ustring                       CurrentOpenMouse;
        XDevice                             *dev; // the currently open mouse device pointer


        // Autoreconnect feature stuff - Need ID's to kill threads if needed
        bool                                wantDisconnect;
        time_t                              lastAttempt;
        uint32_t                            lastServer;

        // These are used by the message of the day window
        Gtk::Window                         *motdWindow;
        Gtk::Notebook                       *motdNotebook;
        Gtk::TextView                       *motdUsers;
        Gtk::TextView                       *motdGuests;
        Gtk::CheckButton                    *motdIgnore;
        Gtk::Button                         *motdOkButton;

        // These are used by the password entry dialog
        Gtk::Dialog                         *passwordDialog;
        Gtk::Entry                          *passwordEntry;
        Glib::ustring                       password;
        bool                                passwordStatus;

        // These are used by the kick/ban reason entry dialog
        Gtk::Dialog                         *reasonDialog;
        Gtk::Entry                          *reasonEntry;
        Glib::ustring                       reason;
        bool                                reasonStatus;
        bool                                reasonValid;

        // These are used by the text string entry dialog
        Gtk::Dialog                         *textStringChangeDialog;
        Gtk::Entry                          *textStringChangeCommentEntry;
        Gtk::Entry                          *textStringChangeURLEntry;
        Gtk::Entry                          *textStringChangeIntegrationEntry;
        Gtk::CheckButton                    *textStringSilenceCommentCheckButton;
        Glib::ustring                       comment;
        Glib::ustring                       url;
        Glib::ustring                       integration_text;

        //Glib::Thread                        *networkThread;

        Glib::ustring getPasswordEntry(Glib::ustring title = "Password", Glib::ustring prompt = "Password");
        bool getReasonEntry(Glib::ustring title = "Reason", Glib::ustring prompt = "Reason");
        uint32_t getActiveServer(void);
        void setActiveServer(uint32_t row_number);
        void errorDialog(Glib::ustring message);
        void setTooltip(void);
        std::string stripMotdRtf(const char *input); 

    protected:
        struct _cli_options *options;

        // main window callbacks
        void mangler_show_cb(void);
        bool mangler_quit_cb(void);

        // connection handlers
        void onConnectHandler(
                Glib::ustring hostname,
                Glib::ustring port,
                Glib::ustring username,
                Glib::ustring password,
                Glib::ustring phonetic = "",
                Glib::ustring charset = "",
                bool acceptPages = true,
                bool acceptU2U = true,
                bool acceptPrivateChat = true,
                bool allowRecording = true);
        bool reconnectStatusHandler(void);

        // button signal handlers
        void quickConnectButton_clicked_cb(void);
        void serverListButton_clicked_cb(void);
        void connectButton_clicked_cb(void);
        void commentButton_clicked_cb(void);
        void chatButton_clicked_cb(void);
        void bindingsButton_clicked_cb(void);
        void adminButton_clicked_cb(void);
        void settingsButton_clicked_cb(void);
        void aboutButton_clicked_cb(void);
        void xmitButton_toggled_cb(void);
        void statusIcon_activate_cb(void);
        void statusIcon_scroll_event_cb(GdkEventScroll* event);
        void statusIcon_buttonpress_event_cb(GdkEventButton* event);
        void errorOKButton_clicked_cb(void);

        // menu bar signal handlers
        void buttonMenuItem_toggled_cb(void);
        void hideServerInfoMenuItem_toggled_cb(void);
        void hideGuestFlagMenuItem_toggled_cb(void);
        void motdMenuItem_activate_cb(void);
        void recorderMenuItem_activate_cb(void);
        void quitMenuItem_activate_cb(void);
        void adminLoginMenuItem_activate_cb(void);
        void adminWindowMenuItem_activate_cb(void);

        bool getNetworkEvent(void);
        bool updateIntegration(void); // music player integration
        bool checkPushToTalkKeys(void);
        bool checkVoiceActivation(void);
        bool checkPushToTalkMouse(void);
        bool updateXferAmounts(void);

        // quick mute options
        void muteSoundCheckButton_toggled_cb(void);
        void muteMicCheckButton_toggled_cb(void);
        void muteSoundCheckMenuItem_toggled_cb(void);
        void muteMicCheckMenuItem_toggled_cb(void);

        // quick connect signal handlers
        void qcConnectButton_clicked_cb(void);
        void qcCancelButton_clicked_cb(void);

        // message of the day window signal handlers
        void motdIgnore_toggled_cb(void);
        void motdOkButton_clicked_cb(void);

        // password dialog signal handlers
        void passwordDialogOkButton_clicked_cb(void);
        void passwordDialogCancelButton_clicked_cb(void);

        // kick/ban reason dialog signal handlers
        void reasonDialogOkButton_clicked_cb(void);
        void reasonDialogCancelButton_clicked_cb(void);

        // text string change dialog signal handlers
        void textStringChangeDialogOkButton_clicked_cb(void);
        void textStringChangeDialogCancelButton_clicked_cb(void);
};

class ManglerError
{
    public:
        uint32_t        code;
        Glib::ustring   message;
        Glib::ustring   module;
        ManglerError(uint32_t code, Glib::ustring message, Glib::ustring module = "");
};

GdkFilterReturn ptt_filter(GdkXEvent *gdk_xevent, GdkEvent *event, gpointer data);

extern Mangler *mangler;

#endif

