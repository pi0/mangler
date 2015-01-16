/*
 * vim: softtabstop=4 shiftwidth=4 cindent foldmethod=marker expandtab
 *
 * $LastChangedDate: 2011-06-27 23:50:10 +0430 (Mon, 27 Jun 2011) $
 * $Revision: 1139 $
 * $LastChangedBy: econnell $
 * $URL: http://svn.mangler.org/mangler/trunk/src/mangler.cpp $
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

#include <gtkmm.h>
#include <iostream>
#include <stdio.h>
#include <gdk/gdkx.h>
#include <X11/extensions/XInput.h>

#include "mangler.h"
#include "manglerui.h"
#include "mangler-icons.h"
#include "channeltree.h"
#include "manglernetwork.h"
#include "mangleraudio.h"
#include "manglersettings.h"
#include "manglerserverlist.h"
#include "manglerchat.h"
#include "manglerprivchat.h"
#include "manglercharset.h"
#include "manglerintegration.h"
#include "mangleradmin.h"
#include "manglerrecorder.h"
#ifdef HAVE_XOSD
# include "manglerosd.h"
#endif
#ifdef HAVE_G15
# include "manglerg15.h"
#endif
#include "locale.h"

using namespace std;

Mangler *mangler;

ManglerConfig Mangler::config;

Mangler::Mangler(struct _cli_options *options) {/*{{{*/
    this->options = options;

    // load all of our icons
    icons.insert(std::make_pair("black_circle",                 Gdk::Pixbuf::create_from_inline(-1, black_circle                )));
    icons.insert(std::make_pair("blue_circle",                  Gdk::Pixbuf::create_from_inline(-1, blue_circle                 )));
    icons.insert(std::make_pair("cyan_circle",                  Gdk::Pixbuf::create_from_inline(-1, cyan_circle                 )));
    icons.insert(std::make_pair("green_circle",                 Gdk::Pixbuf::create_from_inline(-1, green_circle                )));
    icons.insert(std::make_pair("purple_circle",                Gdk::Pixbuf::create_from_inline(-1, purple_circle               )));
    icons.insert(std::make_pair("red_circle",                   Gdk::Pixbuf::create_from_inline(-1, red_circle                  )));
    icons.insert(std::make_pair("yellow_circle",                Gdk::Pixbuf::create_from_inline(-1, yellow_circle               )));
    icons.insert(std::make_pair("mangler_logo",                 Gdk::Pixbuf::create_from_inline(-1, mangler_logo                )));

    icons.insert(std::make_pair("tray_icon",                    Gdk::Pixbuf::create_from_inline(-1, tray_icon_purple            )));
    icons.insert(std::make_pair("tray_icon_blue",               Gdk::Pixbuf::create_from_inline(-1, tray_icon_blue              )));
    icons.insert(std::make_pair("tray_icon_red",                Gdk::Pixbuf::create_from_inline(-1, tray_icon_red               )));
    icons.insert(std::make_pair("tray_icon_green",              Gdk::Pixbuf::create_from_inline(-1, tray_icon_green             )));
    icons.insert(std::make_pair("tray_icon_yellow",             Gdk::Pixbuf::create_from_inline(-1, tray_icon_yellow            )));
    icons.insert(std::make_pair("tray_icon_grey",               Gdk::Pixbuf::create_from_inline(-1, tray_icon_grey              )));
    icons.insert(std::make_pair("tray_icon_purple",             Gdk::Pixbuf::create_from_inline(-1, tray_icon_purple            )));

    icons.insert(std::make_pair("user_icon_red",                Gdk::Pixbuf::create_from_inline(-1, user_icon_red               )));
    icons.insert(std::make_pair("user_icon_yellow",             Gdk::Pixbuf::create_from_inline(-1, user_icon_yellow            )));
    icons.insert(std::make_pair("user_icon_green",              Gdk::Pixbuf::create_from_inline(-1, user_icon_green             )));
    icons.insert(std::make_pair("user_icon_orange",             Gdk::Pixbuf::create_from_inline(-1, user_icon_orange            )));


    try {
        if (options->uifromfile) {
            builder = Gtk::Builder::create_from_file(options->uifilename);
        } else {
            builder = Gtk::Builder::create_from_string(ManglerUI);
        }
        builder->get_widget("manglerWindow", manglerWindow);
        manglerWindow->set_icon(icons["tray_icon"]);
    } catch (const Glib::Error& e) {
        std::cerr << e.what() << std::endl;
        exit(EXIT_FAILURE);
    }
    manglerWindow->signal_show().connect(sigc::mem_fun(this, &Mangler::mangler_show_cb));
    //manglerWindow->signal_hide().connect(sigc::mem_fun(this, &Mangler::manglerWindow_hide_cb));
    Gtk::Main::signal_quit().connect(sigc::mem_fun(this, &Mangler::mangler_quit_cb));

    /*
     * Retreive all buttons from builder and set their singal handler callbacks
     */
    // Quick Connect Button
    builder->get_widget("quickConnectButton", button);
    button->signal_clicked().connect(sigc::mem_fun(this, &Mangler::quickConnectButton_clicked_cb));

    // Server List Button
    builder->get_widget("serverListButton", button);
    button->signal_clicked().connect(sigc::mem_fun(this, &Mangler::serverListButton_clicked_cb));

    // Connect Button
    builder->get_widget("connectButton", button);
    button->signal_clicked().connect(sigc::mem_fun(this, &Mangler::connectButton_clicked_cb));

    // Comment Button
    builder->get_widget("commentButton", button);
    button->signal_clicked().connect(sigc::mem_fun(this, &Mangler::commentButton_clicked_cb));

    // Chat Button
    builder->get_widget("chatButton", button);
    button->signal_clicked().connect(sigc::mem_fun(this, &Mangler::chatButton_clicked_cb));

    // Bindings Button
    builder->get_widget("bindingsButton", button);
    button->signal_clicked().connect(sigc::mem_fun(this, &Mangler::bindingsButton_clicked_cb));

    // Admin Button
    builder->get_widget("adminButton", button);
    button->signal_clicked().connect(sigc::mem_fun(this, &Mangler::adminButton_clicked_cb));
    isAdmin = false;
    isChanAdmin = false;

    // Settings Button
    builder->get_widget("settingsButton", button);
    button->signal_clicked().connect(sigc::mem_fun(this, &Mangler::settingsButton_clicked_cb));

    // About Button
    builder->get_widget("aboutButton", button);
    button->signal_clicked().connect(sigc::mem_fun(this, &Mangler::aboutButton_clicked_cb));

    // Settings Button
    builder->get_widget("xmitButton", togglebutton);
    togglebutton->signal_toggled().connect(sigc::mem_fun(this, &Mangler::xmitButton_toggled_cb));

    // Quick Connect Dialog Buttons
    builder->get_widget("qcConnectButton", button);
    button->signal_clicked().connect(sigc::mem_fun(this, &Mangler::qcConnectButton_clicked_cb));

    builder->get_widget("qcCancelButton", button);
    button->signal_clicked().connect(sigc::mem_fun(this, &Mangler::qcCancelButton_clicked_cb));

    // Input VU Meter
    builder->get_widget("inputVUMeterProgressBar", inputvumeter);

    // Quick Mute Options
    muteMic   = false;
    muteSound = false;
    builder->get_widget("muteMicCheckButton", checkbutton);
    checkbutton->signal_toggled().connect(sigc::mem_fun(this, &Mangler::muteMicCheckButton_toggled_cb));
    builder->get_widget("muteSoundCheckButton", checkbutton);
    checkbutton->signal_toggled().connect(sigc::mem_fun(this, &Mangler::muteSoundCheckButton_toggled_cb));

    // Autoreconnect feature implementation
    wantDisconnect = false;

    // Feature implementation
    motdAlways = false;

    /*
     * Retreive all menu bar items from builder and set their singal handler
     * callbacks.  Most of these can use the same callback as their
     * corresponding button
     */
    builder->get_widget("buttonMenuItem", checkmenuitem);
    checkmenuitem->signal_toggled().connect(sigc::mem_fun(this, &Mangler::buttonMenuItem_toggled_cb));

    builder->get_widget("hideServerInfoMenuItem", checkmenuitem);
    checkmenuitem->signal_toggled().connect(sigc::mem_fun(this, &Mangler::hideServerInfoMenuItem_toggled_cb));

    builder->get_widget("hideGuestFlagMenuItem", checkmenuitem);
    checkmenuitem->signal_toggled().connect(sigc::mem_fun(this, &Mangler::hideGuestFlagMenuItem_toggled_cb));

    builder->get_widget("quickConnectMenuItem", menuitem);
    menuitem->signal_activate().connect(sigc::mem_fun(this, &Mangler::quickConnectButton_clicked_cb));

    builder->get_widget("serverListMenuItem", menuitem);
    menuitem->signal_activate().connect(sigc::mem_fun(this, &Mangler::serverListButton_clicked_cb));

    builder->get_widget("adminLoginMenuItem", menuitem);
    menuitem->signal_activate().connect(sigc::mem_fun(this, &Mangler::adminLoginMenuItem_activate_cb));

    builder->get_widget("adminWindowMenuItem", menuitem);
    menuitem->signal_activate().connect(sigc::mem_fun(this, &Mangler::adminWindowMenuItem_activate_cb));

    builder->get_widget("settingsMenuItem", menuitem);
    menuitem->signal_activate().connect(sigc::mem_fun(this, &Mangler::settingsButton_clicked_cb));
    builder->get_widget("statusIconSettingsMenuItem", menuitem);
    menuitem->signal_activate().connect(sigc::mem_fun(this, &Mangler::settingsButton_clicked_cb));

    builder->get_widget("commentMenuItem", menuitem);
    menuitem->signal_activate().connect(sigc::mem_fun(this, &Mangler::commentButton_clicked_cb));

    builder->get_widget("motdMenuItem", menuitem);
    menuitem->signal_activate().connect(sigc::mem_fun(this, &Mangler::motdMenuItem_activate_cb));

    builder->get_widget("chatMenuItem", menuitem);
    menuitem->signal_activate().connect(sigc::mem_fun(this, &Mangler::chatButton_clicked_cb));

    builder->get_widget("recorderMenuItem", menuitem);
    menuitem->signal_activate().connect(sigc::mem_fun(this, &Mangler::recorderMenuItem_activate_cb));

    builder->get_widget("quitMenuItem", menuitem);
    menuitem->signal_activate().connect(sigc::mem_fun(this, &Mangler::quitMenuItem_activate_cb));
    builder->get_widget("statusIconQuitMenuItem", menuitem);
    menuitem->signal_activate().connect(sigc::mem_fun(this, &Mangler::quitMenuItem_activate_cb));

    builder->get_widget("aboutMenuItem", menuitem);
    menuitem->signal_activate().connect(sigc::mem_fun(this, &Mangler::aboutButton_clicked_cb));

    // connect the signal for our error dialog button
    builder->get_widget("errorOKButton", button);
    button->signal_clicked().connect(sigc::mem_fun(this, &Mangler::errorOKButton_clicked_cb));

    // Set up our message of the day window
    builder->get_widget("motdWindow", motdWindow);
    builder->get_widget("motdNotebook", motdNotebook);
    motdNotebook->set_current_page(1);
    motdNotebook->set_show_tabs(false);
    builder->get_widget("motdUsers", motdUsers);
    builder->get_widget("motdGuests", motdGuests);
    builder->get_widget("motdIgnore", motdIgnore);
    motdIgnore->signal_toggled().connect(sigc::mem_fun(this, &Mangler::motdIgnore_toggled_cb));
    builder->get_widget("motdOkButton", motdOkButton);
    motdOkButton->signal_clicked().connect(sigc::mem_fun(this, &Mangler::motdOkButton_clicked_cb));

    // Set up our generic password dialog box
    builder->get_widget("passwordDialog", passwordDialog);
    builder->get_widget("passwordEntry", passwordEntry);
    builder->get_widget("passwordOkButton", button);
    button->signal_clicked().connect(sigc::mem_fun(this, &Mangler::passwordDialogOkButton_clicked_cb));
    builder->get_widget("passwordCancelButton", button);
    button->signal_clicked().connect(sigc::mem_fun(this, &Mangler::passwordDialogCancelButton_clicked_cb));

    // Set up our kick/ban reason entry dialog box
    builder->get_widget("reasonDialog", reasonDialog);
    builder->get_widget("reasonEntry", reasonEntry);
    builder->get_widget("reasonOkButton", button);
    button->signal_clicked().connect(sigc::mem_fun(this, &Mangler::reasonDialogOkButton_clicked_cb));
    builder->get_widget("reasonCancelButton", button);
    button->signal_clicked().connect(sigc::mem_fun(this, &Mangler::reasonDialogCancelButton_clicked_cb));

    // Set up the text string change dialog box
    builder->get_widget("textStringChangeDialog", textStringChangeDialog);
    builder->get_widget("textStringChangeCommentEntry", textStringChangeCommentEntry);
    builder->get_widget("textStringChangeURLEntry", textStringChangeURLEntry);
    builder->get_widget("textStringChangeIntegrationEntry", textStringChangeIntegrationEntry);
    builder->get_widget("textStringSilenceCommentCheckButton", textStringSilenceCommentCheckButton);
    builder->get_widget("textStringOkButton", button);
    button->signal_clicked().connect(sigc::mem_fun(this, &Mangler::textStringChangeDialogOkButton_clicked_cb));
    builder->get_widget("textStringCancelButton", button);
    button->signal_clicked().connect(sigc::mem_fun(this, &Mangler::textStringChangeDialogCancelButton_clicked_cb));

    // Create Channel Tree
    channelTree = new ManglerChannelTree(builder);

    // Create Network Communication Object
    network = new ManglerNetwork(builder);

    // Create settings object, load the configuration file, and apply.  If the
    // user has PTT key/mouse enabled, start a timer here
    settings = new ManglerSettings(builder);
    isTransmittingButton = 0;
    isTransmittingVA = 0;
    isTransmittingMouse = 0;
    isTransmittingKey = 0;
    isTransmitting = 0;
    Glib::signal_timeout().connect(sigc::mem_fun(this, &Mangler::checkPushToTalkKeys), 100);
    Glib::signal_timeout().connect(sigc::mem_fun(this, &Mangler::checkVoiceActivation), 100);
    Glib::signal_timeout().connect(sigc::mem_fun(this, &Mangler::checkPushToTalkMouse), 100);

    // Create our audio control object for managing devices
    audioControl = new ManglerAudio(AUDIO_CONTROL);
    audioControl->getDeviceList(config["AudioSubsystem"].toUString());

    // If we have eSpeak, go ahead and initialize it
#ifdef HAVE_ESPEAK
    if ((espeakRate = espeak_Initialize(AUDIO_OUTPUT_RETRIEVAL, 0, NULL, espeakEVENT_LIST_TERMINATED)) < 0) {
        fprintf(stderr, "espeak: initialize error\n");
        return;
    }
#endif

    // set saved window size from settings
    unsigned windowWidth( config["WindowWidth"].toUInt() );
    unsigned windowHeight( config["WindowHeight"].toUInt() );
    if (windowWidth > 0 && windowHeight > 0) manglerWindow->set_default_size(windowWidth, windowHeight);

    // set the master volume
    v3_set_volume_master(config["MasterVolumeLevel"].toUInt());
    v3_set_volume_xmit(config["InputGainLevel"].toUInt());

    builder->get_widget("buttonMenuItem", checkmenuitem);
    checkmenuitem->set_active(config["ButtonsHidden"].toBool());

    builder->get_widget("hideServerInfoMenuItem", checkmenuitem);
    checkmenuitem->set_active(config["ServerInfoHidden"].toBool());

    builder->get_widget("hideGuestFlagMenuItem", checkmenuitem);
    checkmenuitem->set_active(config["GuestFlagHidden"].toBool());

    // Create Server List Window
    serverList = new ManglerServerList(builder);

    // Create Chat Window
    chat = new ManglerChat(builder);

    // Create Admin Window
    admin = new ManglerAdmin(builder);
    wantAdminWindow = false;

    // Create Recording Window
    recorder = new ManglerRecorder(builder);

    // Add our servers to the main window drop down
    builder->get_widget("serverSelectComboBox", combobox);
    combobox->set_model(serverList->serverListTreeModel);
    combobox->pack_start(serverList->serverListColumns.name);
    Gtk::CellRendererText *renderer = (Gtk::CellRendererText*)(*(combobox->get_cells().begin()));
    renderer->property_ellipsize() = Pango::ELLIPSIZE_END;
    int serverSelection = 0, ctr = 0;
    iniFile::iterator server = config.servers.begin();
    while (server != config.servers.end()) {
        Gtk::TreeRow row = *(serverList->serverListTreeModel->append());
        // is this id useful at all?
        row[serverList->serverListColumns.id] = ctr;
        row[serverList->serverListColumns.name] = Glib::locale_to_utf8(server->first);
        row[serverList->serverListColumns.hostname] = server->second["Hostname"].toUString();
        row[serverList->serverListColumns.port] = server->second["Port"].toUString();
        row[serverList->serverListColumns.username] = server->second["Username"].toUString();
        if (config["LastConnectedServerName"] == server->first) {
            serverSelection = ctr;
        }
        server++; ++ctr;
    }
    // Select the last one used (or the first if unknown)
    combobox->set_active(serverSelection);

    // Status Tray Icon
    statusIcon = Gtk::StatusIcon::create(icons["tray_icon_grey"]);
    statusIcon->signal_activate().connect(sigc::mem_fun(this, &Mangler::statusIcon_activate_cb));
    statusIcon->signal_scroll_event().connect_notify(sigc::mem_fun(this, &Mangler::statusIcon_scroll_event_cb));
    statusIcon->signal_button_press_event().connect_notify(sigc::mem_fun(this, &Mangler::statusIcon_buttonpress_event_cb));
    builder->get_widget("statusIconMenu", statusIconMenu);
    builder->get_widget("muteMicCheckMenuItem", checkmenuitem);
    checkmenuitem->signal_toggled().connect(sigc::mem_fun(this, &Mangler::muteMicCheckMenuItem_toggled_cb));
    checkmenuitem->set_active(config["MuteMic"].toBool());
    builder->get_widget("muteSoundCheckMenuItem", checkmenuitem);
    checkmenuitem->signal_toggled().connect(sigc::mem_fun(this, &Mangler::muteSoundCheckMenuItem_toggled_cb));
    checkmenuitem->set_active(config["MuteSound"].toBool());
    iconified = false;
    setTooltip();

    // Music (Now playing)
    integration = new ManglerIntegration();
    //integration->setClient((MusicClient)settings->config.AudioIntegrationPlayer);
    // TODO: MusicClient?  wants a const char *??
    integration->setClient((MusicClient)config["AudioIntegrationPlayer"].toUInt());
    integration->update(true);

#ifdef HAVE_XOSD
    // Create XOSD Overlay
    osd = new ManglerOsd();
#endif
#ifdef HAVE_G15
    // Create G15 Keyboard LCD Handler
    g15 = new ManglerG15();
#endif

    Glib::signal_timeout().connect(sigc::mem_fun(this, &Mangler::updateIntegration), 1000);
    Glib::signal_timeout().connect(sigc::mem_fun(*this, &Mangler::updateXferAmounts), 500);
    Glib::signal_timeout().connect(sigc::mem_fun(*this, &Mangler::getNetworkEvent), 10);
}/*}}}*/

Mangler::~Mangler() {/*{{{*/
    delete channelTree;
    delete network;
    delete settings;
    delete audioControl;
    delete serverList;
    delete chat;
    delete admin;
    delete recorder;
    delete integration;
#ifdef HAVE_XOSD
    delete osd;
#endif
#ifdef HAVE_G15
    delete g15;
#endif
}/*}}}*/

/*
 * Main Window Callbacks
 */
void Mangler::mangler_show_cb(void) {/*{{{*/
    if (options) {
        // Command Line Quick Connect
        if (!options->qc_server.empty()) {
            Glib::ustring::size_type separator = options->qc_server.find_last_of(":");
            if (separator > 0 && options->qc_server.length() != separator + 1 && options->qc_username.length()) {
                onConnectHandler(
                        options->qc_server.substr(0, separator),
                        options->qc_server.substr(separator + 1),
                        options->qc_username,
                        options->qc_password);
            }
        }
        options = NULL;
    }
}/*}}}*/
bool Mangler::mangler_quit_cb(void) {/*{{{*/
    int w, h;
    manglerWindow->get_size(w, h);
    config["WindowWidth"] = w;
    config["WindowHeight"] = h;
    config.save();

    return true;
}/*}}}*/

/* 
 * Connection Handling
 */
void Mangler::onConnectHandler(
        Glib::ustring hostname,
        Glib::ustring port,
        Glib::ustring username,
        Glib::ustring password,
        Glib::ustring phonetic,
        Glib::ustring charset,
        bool acceptPages,
        bool acceptU2U,
        bool acceptPrivateChat,
        bool allowRecording) {/*{{{*/
    set_charset(charset);
    isAdmin = false;
    isChanAdmin = false;
    v3_set_server_opts(V3_USER_ACCEPT_PAGES, acceptPages);
    v3_set_server_opts(V3_USER_ACCEPT_U2U,   acceptU2U);
    v3_set_server_opts(V3_USER_ACCEPT_CHAT,  acceptPrivateChat);
    v3_set_server_opts(V3_USER_ALLOW_RECORD, allowRecording);

    channelTree->updateLobby("Connecting...");
    Glib::Thread::create(sigc::bind(sigc::mem_fun(this->network, &ManglerNetwork::connect), hostname, port, username, password, phonetic), FALSE);
}/*}}}*/
void Mangler::onDisconnectHandler(void) {/*{{{*/
    Gtk::Button *connectbutton;

    builder->get_widget("connectButton", connectbutton);
    if (connectbutton->get_label() == "gtk-disconnect") {
        builder->get_widget("adminButton", button);
        button->set_sensitive(false);
        builder->get_widget("adminLoginMenuItem", menuitem);
        menuitem->set_label("_Admin Login");
        menuitem->set_sensitive(false);
        builder->get_widget("adminWindowMenuItem", menuitem);
        menuitem->set_sensitive(false);
        builder->get_widget("chatButton", button);
        button->set_sensitive(false);
        builder->get_widget("motdMenuItem", menuitem);
        menuitem->set_sensitive(false);
        builder->get_widget("chatMenuItem", menuitem);
        menuitem->set_sensitive(false);
        builder->get_widget("commentButton", button);
        button->set_sensitive(false);
        builder->get_widget("commentMenuItem", menuitem);
        menuitem->set_sensitive(false);
        isTransmittingMouse = false;
        isTransmittingKey = false;
        isTransmittingVA = false;
        isTransmittingButton = false;
        stopTransmit();

        connectbutton->set_sensitive(true);
#ifdef HAVE_XOSD
        osd->destroyOsd();
#endif
        outputAudio.clear();
        channelTree->clear();
        admin->hide();
        admin->clear();
        builder->get_widget("xmitButton", togglebutton);
        togglebutton->set_active(false);
        builder->get_widget("progressbar", progressbar);
        progressbar->set_text("");
        progressbar->set_fraction(0);
        progressbar->hide();
        builder->get_widget("statusbar", statusbar);
        statusbar->pop();
        statusbar->push("Disconnected");
        //builder->get_widget("serverTabLabel", label);
        //label->set_label("Not Connected");
        builder->get_widget("pingLabel", label);
        label->set_label("N/A");
        builder->get_widget("userCountLabel", label);
        label->set_label("N/A");
        builder->get_widget("codecLabel", label);
        label->set_label("N/A");
        mangler->statusIcon->set(icons["tray_icon_grey"]);
        isAdmin = false;
        isChanAdmin = false;
        wantAdminWindow = false;
        motdWindow->hide();
        motdNotebook->set_current_page(1);
        motdNotebook->set_show_tabs(false);
        motdUsers->get_buffer()->set_text("");
        motdGuests->get_buffer()->set_text("");
        chat->clear();
        recorder->can_record(false);
        if (! connectedServerName.empty()) {
            iniSection &server(config.servers[connectedServerName]);
            connectedServerName = "";
            if (!wantDisconnect && server["PersistentConnection"].toBool()) {
                connectbutton->set_label("gtk-cancel");
                lastAttempt = time(NULL);
                Glib::signal_timeout().connect_seconds(sigc::mem_fun(*this, &Mangler::reconnectStatusHandler), 1);
                return;
            }
        }
    }
    connectbutton->set_label("gtk-connect");
    builder->get_widget("serverSelectComboBox", combobox);
    combobox->set_sensitive(true);
}/*}}}*/
bool Mangler::reconnectStatusHandler(void) {/*{{{*/
    Gtk::Button *connectbutton;
    char buf[64] = "";
    int reconnectTimer = (15 - (time(NULL) - lastAttempt));

    builder->get_widget("connectButton", connectbutton);
    if (connectbutton->get_label() != "gtk-cancel" || wantDisconnect) {
        return false;
    }
    builder->get_widget("statusbar", statusbar);
    snprintf(buf, 63, "Attempting reconnect in %d seconds...", (reconnectTimer < 0) ? 0 : reconnectTimer);
    statusbar->pop();
    statusbar->push(buf);
    if (reconnectTimer <= 0) {
        lastAttempt = time(NULL);
        connectbutton->set_label("gtk-connect");
        Mangler::connectButton_clicked_cb();
        return false;
    }

    return true;
}/*}}}*/

/*
 * Button signal handler callbacks
 */
void Mangler::quickConnectButton_clicked_cb(void) {/*{{{*/
    builder->get_widget("quickConnectDialog", dialog);
    dialog->set_icon(icons["tray_icon"]);

    builder->get_widget("qcServerName", entry);
    entry->set_text(config["qc_lastserver.hostname"].toCString());
    builder->get_widget("qcPort", entry);
    entry->set_text(config["qc_lastserver.port"].toCString());
    builder->get_widget("qcUsername", entry);
    entry->set_text(config["qc_lastserver.username"].toCString());
    builder->get_widget("qcPassword", entry);
    entry->set_text(config["qc_lastserver.password"].toCString());

    builder->get_widget("qcConnectButton", button);
    button->set_sensitive(!v3_is_loggedin());

    dialog->set_keep_above(true);
    dialog->run();
    dialog->hide();
}/*}}}*/
void Mangler::serverListButton_clicked_cb(void) {/*{{{*/
    builder->get_widget("serverListWindow", window);
    window->set_icon(icons["tray_icon"]);
    window->show();
}/*}}}*/
void Mangler::connectButton_clicked_cb(void) {/*{{{*/
    Gtk::Button *connectbutton;
    Gtk::TreeModel::iterator iter;

    builder->get_widget("connectButton", connectbutton);
    if (connectbutton->get_label() == "gtk-cancel") {
        wantDisconnect = true;
        onDisconnectHandler();
        builder->get_widget("statusbar", statusbar);
        statusbar->pop();
        statusbar->push("Disconnected");
    } else if (connectbutton->get_label() == "gtk-connect") {
        builder->get_widget("serverSelectComboBox", combobox);
        iter = combobox->get_active();
        if (iter) {
            Gtk::TreeModel::Row row = *iter;
            connectedServerName = Glib::ustring( row[serverList->serverListColumns.name] );
            iniSection &server(config.servers[connectedServerName]);
            Glib::ustring hostname = server["hostname"].toCString();
            Glib::ustring port     = server["port"].toCString();
            Glib::ustring username = server["username"].toCString();
            Glib::ustring password = server["password"].toCString();
            Glib::ustring phonetic = server["phonetic"].toCString();
            if (!server.size() || hostname.empty() || port.empty() || username.empty()) {
                builder->get_widget("statusbar", statusbar);
                statusbar->pop();
                statusbar->push("Not connected.");
                if (hostname.empty()) {
                    errorDialog("You have not specified a hostname for this server.");
                    return;
                }
                if (port.empty()) {
                    errorDialog("You have not specified a port for this server.");
                    return;
                }
                if (username.empty()) {
                    errorDialog("You have not specified a username for this server.");
                    return;
                }
                return;
            }
            config["LastConnectedServerName"] = connectedServerName;
            config.config.save();
            wantDisconnect = false;
            onConnectHandler(
                    hostname,
                    port,
                    username,
                    password,
                    phonetic,
                    server["Charset"].toUString(),
                    server["acceptPages"].length() ? server["acceptPages"].toBool() : true,
                    server["acceptU2U"].length() ? server["acceptU2U"].toBool() : true,
                    server["acceptPrivateChat"].length() ? server["acceptPrivateChat"].toBool() : true,
                    server["allowRecording"].length() ? server["allowRecording"].toBool() : true);
        }
    } else {
        wantDisconnect = true;
        v3_logout();
    }
    return;
}/*}}}*/
void Mangler::commentButton_clicked_cb(void) {/*{{{*/
    if (v3_is_loggedin()) {
        textStringChangeCommentEntry->set_text(comment);
        textStringChangeURLEntry->set_text(url);
        textStringChangeIntegrationEntry->set_text(integration_text);
        textStringChangeDialog->run();
        textStringChangeDialog->hide();
    }
}/*}}}*/
void Mangler::chatButton_clicked_cb(void) {/*{{{*/
    if (v3_is_loggedin()) {
        if (!chat->isOpen) {
            chat->chatWindow->set_icon(icons["tray_icon"]);
            chat->chatWindow->show();
        } else {
            chat->chatWindow->present();
        }
    }
}/*}}}*/
void Mangler::bindingsButton_clicked_cb(void) {/*{{{*/
    //fprintf(stderr, "bindings button clicked\n");
    static Glib::ustring color = "red";
    if (color == "red") {
        color = "green";
        statusIcon->set(icons["tray_icon_green"]);
    } else if (color == "green") {
        color = "blue";
        statusIcon->set(icons["tray_icon_blue"]);
    } else if (color == "blue") {
        color = "yellow";
        statusIcon->set(icons["tray_icon_yellow"]);
    } else if (color == "yellow") {
        color = "red";
        statusIcon->set(icons["tray_icon_red"]);
    }
}/*}}}*/
void Mangler::adminButton_clicked_cb(void) {/*{{{*/
    Glib::ustring password;
    if (! isAdmin) {
        password = mangler->getPasswordEntry("Admin Password");
        if (password.length()) {
            v3_admin_login((char *)password.c_str());
            wantAdminWindow = true;
            // if we tried sending a password, the only options are either
            // success or get booted from the server.
        }
    } else {
        admin->show();
    }
}/*}}}*/
void Mangler::adminLoginMenuItem_activate_cb(void) {/*{{{*/
    builder->get_widget("adminLoginMenuItem", menuitem);
    if (menuitem->get_label() == "_Admin Logout") {
        v3_admin_logout();
    } else {
        adminButton_clicked_cb();
    }
}/*}}}*/
void Mangler::adminWindowMenuItem_activate_cb(void) {/*{{{*/
    admin->show();
}/*}}}*/
void Mangler::settingsButton_clicked_cb(void) {/*{{{*/
    settings->settingsWindow->show();
}/*}}}*/
void Mangler::aboutButton_clicked_cb(void) {/*{{{*/
    builder->get_widget("aboutWindow", aboutdialog);
    aboutdialog->set_keep_above(true);
    aboutdialog->set_logo(icons["mangler_logo"]);
    aboutdialog->run();
    aboutdialog->hide();
}/*}}}*/
void Mangler::xmitButton_toggled_cb(void) {/*{{{*/
    builder->get_widget("xmitButton", togglebutton);
    if (togglebutton->get_active()) {
        isTransmittingButton = true;
        startTransmit();
    } else {
        isTransmittingButton = false;
        if (! isTransmittingKey && ! isTransmittingMouse && ! isTransmittingVA) {
            stopTransmit();
        }
    }
}/*}}}*/

/*
 * Menu bar signal handler callbacks
 */
void Mangler::buttonMenuItem_toggled_cb(void) {/*{{{*/
    builder->get_widget("buttonMenuItem", checkmenuitem);
    builder->get_widget("mainWindowButtonVBox", vbox);
    if (checkmenuitem->get_active()) {
        vbox->hide();
        config["ButtonsHidden"] = true;
    } else {
        config["ButtonsHidden"] = false;
        vbox->show();
    }
    config.config.save();
}/*}}}*/
void Mangler::hideServerInfoMenuItem_toggled_cb(void) {/*{{{*/
    builder->get_widget("hideServerInfoMenuItem", checkmenuitem);
    builder->get_widget("serverTable", table);
    if (checkmenuitem->get_active()) {
        table->hide();
        config["ServerInfoHidden"] = true;
    } else {
        config["ServerInfoHidden"] = false;
        table->show();
    }
    config.config.save();
}/*}}}*/
void Mangler::hideGuestFlagMenuItem_toggled_cb(void) {/*{{{*/
    builder->get_widget("hideGuestFlagMenuItem", checkmenuitem);
    if (checkmenuitem->get_active()) {
        config["GuestFlagHidden"] = true;
    } else {
        config["GuestFlagHidden"] = false;
    }
    channelTree->refreshAllUsers();
    config.config.save();
}/*}}}*/
void Mangler::motdMenuItem_activate_cb(void) {/*{{{*/
    motdIgnore->set_sensitive(!connectedServerName.empty());
    motdIgnore->set_active(connectedServerName.length() && config.servers[connectedServerName]["MotdIgnore"].toBool());
    motdOkButton->grab_focus();
    motdWindow->present();
}/*}}}*/
void Mangler::recorderMenuItem_activate_cb(void) {/*{{{*/
    recorder->recWindow->set_icon(icons["tray_icon"]);
    recorder->show();
}/*}}}*/
void Mangler::quitMenuItem_activate_cb(void) {/*{{{*/
    Gtk::Main::quit();
}/*}}}*/

/*
 * Other signal handler callbacks
 */
void Mangler::statusIcon_activate_cb(void) {/*{{{*/
    if (iconified == true) {
        manglerWindow->deiconify();
        manglerWindow->present();
        manglerWindow->set_skip_pager_hint(false);
        manglerWindow->set_skip_taskbar_hint(false);
        iconified = false;
    } else {
        manglerWindow->iconify();
        manglerWindow->set_skip_pager_hint(true);
        manglerWindow->set_skip_taskbar_hint(true);
        iconified = true;
    }
}/*}}}*/
void Mangler::statusIcon_buttonpress_event_cb(GdkEventButton* event) {/*{{{*/
    if ((event->type == GDK_BUTTON_PRESS) && (event->button == 3)) {
        builder->get_widget("statusIconMenu", statusIconMenu);
        builder->get_widget("muteMicCheckButton", checkbutton);
        builder->get_widget("muteMicCheckMenuItem", checkmenuitem);
        checkmenuitem->set_active(checkbutton->get_active());

        builder->get_widget("muteSoundCheckButton", checkbutton);
        builder->get_widget("muteSoundCheckMenuItem", checkmenuitem);
        checkmenuitem->set_active(checkbutton->get_active());
        statusIconMenu->popup(event->button, event->time);
    }
}
/*}}}*/
void Mangler::statusIcon_scroll_event_cb(GdkEventScroll* event) {/*{{{*/
    if (event->type != GDK_SCROLL) {
        return;
    }
    int volume = config["MasterVolumeLevel"].toInt();
    switch (event->direction) {
      case GDK_SCROLL_UP:
        volume = (volume + 5 > 148) ? 148 : volume + 5;
        break;
      case GDK_SCROLL_DOWN:
        volume = (volume - 5 < 0) ? 0 : volume - 5;
        break;
      default:
        return;
    }
    if (volume + 5 > 79 && volume - 5 < 79) {
        volume = 79;
    }
    config["MasterVolumeLevel"] = volume;
    settings->volumeAdjustment->set_value(volume);
    v3_set_volume_master(volume);
    setTooltip();
}/*}}}*/
void Mangler::setTooltip(void) {/*{{{*/
    Glib::ustring tooltip = "Mangler: volume: ";
    float value = config["MasterVolumeLevel"].toInt();
    value = (value > 79) ? ((value-79)/69)*100+100 : (value/79)*100;
    tooltip += Glib::ustring::format((int)value) + "%" + ((config["MuteSound"].toBool()) ? " (muted)" : "");
    statusIcon->set_tooltip_text(tooltip);
}/*}}}*/
void Mangler::startTransmit(void) {/*{{{*/
    const v3_codec *codec;
    v3_user *user;

    if (! v3_is_loggedin()) {
        return;
    }
    if (muteMic) {
        return;
    }
    user = v3_get_user(v3_get_user_id());
    if (! user) {
        return;
    }
    if (isTransmitting) {
        v3_free_user(user);
        return;
    }
    isTransmitting = true;
    if ((codec = v3_get_channel_codec(user->channel))) {
        //fprintf(stderr, "channel %d codec rate: %d at sample size %d\n", user->channel, codec->rate, codec->pcmframesize);
        v3_free_user(user);
        channelTree->setUserIcon(v3_get_user_id(), "orange");
        statusIcon->set(icons["tray_icon_yellow"]);
        inputAudio = new ManglerAudio(AUDIO_INPUT, codec->rate, 1, codec->pcmframesize);
    }
}/*}}}*/
void Mangler::stopTransmit(void) {/*{{{*/
    if (!isTransmitting) {
        return;
    }
    isTransmitting = false;
    if (v3_is_loggedin()) {
        channelTree->setUserIcon(v3_get_user_id(), "red");
        statusIcon->set(icons["tray_icon_red"]);
    }
    if (inputAudio) {
        inputAudio->finish();
        inputAudio = NULL;
    }
}/*}}}*/

// Quick Sound Mute
void Mangler::muteSoundCheckButton_toggled_cb(void) {/*{{{*/
    builder->get_widget("muteSoundCheckButton", checkbutton);
    muteSound = checkbutton->get_active();
    config["MuteSound"] = muteSound;
    setTooltip();
}/*}}}*/

// Quick Mic Mute
void Mangler::muteMicCheckButton_toggled_cb(void) {/*{{{*/
    builder->get_widget("muteMicCheckButton", checkbutton);
    muteMic = checkbutton->get_active();
    config["MuteMic"] = muteMic;
    if (muteMic && isTransmitting) {
        stopTransmit();
    } else if (!muteMic && (isTransmittingMouse || isTransmittingKey || isTransmittingButton || isTransmittingVA)) {
        startTransmit();
    }
}/*}}}*/

// Status Icon Sound Mute
void Mangler::muteSoundCheckMenuItem_toggled_cb(void) {/*{{{*/
    builder->get_widget("muteSoundCheckButton", checkbutton);
    builder->get_widget("muteSoundCheckMenuItem", checkmenuitem);
    checkbutton->set_active(checkmenuitem->get_active());
}/*}}}*/

// Status Icon Mic Mute
void Mangler::muteMicCheckMenuItem_toggled_cb(void) {/*{{{*/
    builder->get_widget("muteMicCheckButton", checkbutton);
    builder->get_widget("muteMicCheckMenuItem", checkmenuitem);
    checkbutton->set_active(checkmenuitem->get_active());
}/*}}}*/

// Quick Connect Callbacks
void Mangler::qcConnectButton_clicked_cb(void) {/*{{{*/
    builder->get_widget("qcServerName", entry);
    Glib::ustring hostname = entry->get_text();
    builder->get_widget("qcPort", entry);
    Glib::ustring port = entry->get_text();
    builder->get_widget("qcUsername", entry);
    Glib::ustring username = entry->get_text();
    builder->get_widget("qcPassword", entry);
    Glib::ustring password = entry->get_text();
    //fprintf(stderr, "connecting to: %s:%s\n", server.c_str(), port.c_str());
    config["qc_lastserver.hostname"] = hostname;
    config["qc_lastserver.port"]     = port;
    config["qc_lastserver.username"] = username;
    config["qc_lastserver.password"] = password;
    config.config.save();
    connectedServerName = "";
    onConnectHandler(hostname, port, username, password);
}/*}}}*/
void Mangler::qcCancelButton_clicked_cb(void) {/*{{{*/
}/*}}}*/

// MOTD Window Callbacks
void Mangler::motdIgnore_toggled_cb(void) {/*{{{*/
    if (connectedServerName.length()) {
        config.servers[connectedServerName]["MotdIgnore"] = motdIgnore->get_active();
    }
}/*}}}*/
void Mangler::motdOkButton_clicked_cb(void) {/*{{{*/
    motdWindow->hide();
}/*}}}*/

/*
 * Timeout Callbacks
 *
 * Inbound event processing happens here.
 */
bool Mangler::getNetworkEvent() {/*{{{*/
    v3_event *ev;
    while ((ev = v3_get_event(V3_NONBLOCK)) != NULL) {
        v3_user *u;
        v3_channel *c;
        Glib::ustring rank = "";
        gdk_threads_enter();
        // if we're not logged in, just ignore whatever messages we receive
        // *unless* it's a disconnect message.  This prevents old messages in
        // the queue from attempting to interact with the GUI after a
        // disconnection
        switch (ev->type) {
            case V3_EVENT_PING:/*{{{*/
                if (v3_is_loggedin()) {
                    char buf[64];
                    builder->get_widget("pingLabel", label);
                    builder->get_widget("statusbar", statusbar);
                    if (ev->ping != 65535) {
                        snprintf(buf, 16, "%dms", ev->ping);
#ifdef HAVE_G15
                        g15->update("", "", "", "", buf);
#endif
                        label->set_text(c_to_ustring(buf));
                        snprintf(buf, 63, "Ping: %dms - Users: %d/%d", ev->ping, v3_user_count(), v3_get_max_clients());
                        statusbar->pop();
                        statusbar->push(c_to_ustring(buf));
                    } else {
#ifdef HAVE_G15
                        g15->update("", "", "", "", "checking...");
#endif
                        label->set_text("checking...");
                        snprintf(buf, 63, "Ping: checking... - Users: %d/%d", v3_user_count(), v3_get_max_clients());
                        statusbar->pop();
                        statusbar->push(c_to_ustring(buf));
                    }
                    builder->get_widget("userCountLabel", label);
                    snprintf(buf, 16, "%d/%d", v3_user_count(), v3_get_max_clients());
                    label->set_text(c_to_ustring(buf));
                }
                break;/*}}}*/
            case V3_EVENT_STATUS:/*{{{*/
                if (v3_is_loggedin()) {
                    builder->get_widget("progressbar", progressbar);
                    builder->get_widget("statusbar", statusbar);
                    if (ev->status.percent == 100) {
                        progressbar->hide();
                    } else {
                        progressbar->show();
                        progressbar->set_fraction(ev->status.percent/(float)100);
                    }
                    statusbar->pop();
                    statusbar->push(ev->status.message);
                    //fprintf(stderr, "got event type %d: %d %s\n", ev->type, ev->status.percent, ev->status.message);
                }
                break;/*}}}*/
            case V3_EVENT_USER_LOGIN:/*{{{*/
                u = v3_get_user(ev->user.id);
                if (!u) {
                    fprintf(stderr, "couldn't retreive user id %d\n", ev->user.id);
                    break;
                }
                if (!(ev->flags & V3_LOGIN_FLAGS_EXISTING) && (v3_get_user_channel(v3_get_user_id()) == ev->channel.id)) {
                    audioControl->playNotification("channelenter");
                }
#ifdef HAVE_G15
                if (!(ev->flags & V3_LOGIN_FLAGS_EXISTING)) {
                        g15->update("", "", u->name, "", "");
                }
#endif
                //fprintf(stderr, "adding user id %d: %s to channel %d\n", ev->user.id, u->name, ev->channel.id);
                if (u->rank_id) {
                    v3_rank *r;
                    if ((r = v3_get_rank(u->rank_id))) {
                        rank = c_to_ustring(r->name);
                        v3_free_rank(r);
                    }
                }
                channelTree->addUser(
                        (uint32_t)u->id,
                        (uint32_t)ev->channel.id,
                        c_to_ustring(u->name),
                        c_to_ustring(u->comment),
                        u->phonetic,
                        u->url,
                        c_to_ustring(u->integration_text),
                        (bool)u->guest,
                        (bool)u->real_user_id,
                        rank);
                if (connectedServerName.length() && u->id != v3_get_user_id()) {
                    // If we have a per user volume set for this user name, set it now.
                    if (config.hasUserVolume(connectedServerName, u->name)) {
                        v3_set_volume_user(u->id, config.UserVolume(connectedServerName, u->name).toInt());
                    }
                    // If the user was muted, mute them again.
                    if (config.UserMuted(connectedServerName, u->name).toBool()) {
                        channelTree->muteUserToggle(u->id);
                    }
                }
                v3_free_user(u);
                break;/*}}}*/
            case V3_EVENT_USER_MODIFY:/*{{{*/
                if (v3_is_loggedin()) {
                    u = v3_get_user(ev->user.id);
                    if (!u) {
                        fprintf(stderr, "couldn't retreive user id %d\n", ev->user.id);
                        break;
                    }
                    if (u->id == 0) {
                        channelTree->updateLobby(c_to_ustring(u->name), c_to_ustring(u->comment), c_to_ustring(u->phonetic));
                    } else {
                        if (u->rank_id) {
                            v3_rank *r;
                            if ((r = v3_get_rank(u->rank_id))) {
                                rank = c_to_ustring(r->name);
                                v3_free_rank(r);
                            }
                        }
                        channelTree->updateUser(
                                (uint32_t)u->id,
                                (uint32_t)ev->channel.id,
                                c_to_ustring(u->name),
                                c_to_ustring(u->comment),
                                u->phonetic,
                                u->url,
                                c_to_ustring(u->integration_text),
                                (bool)u->guest,
                                (bool)u->real_user_id,
                                rank);
                    }
                    v3_free_user(u);
                }
                break;/*}}}*/
            case V3_EVENT_CHAN_MODIFY:/*{{{*/
                if (v3_is_loggedin()) {
                    const v3_codec *codec_info;
                    c = v3_get_channel(ev->channel.id);
                    if (!c) {
                        fprintf(stderr, "couldn't retreive channel id %d\n", ev->user.id);
                        break;
                    }
                    channelTree->updateChannel(
                            (uint8_t)c->protect_mode,
                            (uint32_t)c->id,
                            (uint32_t)c->parent,
                            c_to_ustring(c->name),
                            c_to_ustring(c->comment),
                            c->phonetic);
                    if (! isAdmin && ! isChanAdmin && v3_is_channel_admin(c->id)) {
                        isChanAdmin = true;
                    }
                    admin->channelUpdated(c);
                    if (ev->channel.id == v3_get_user_channel(v3_get_user_id())) {
                        builder->get_widget("codecLabel", label);
                        if ((codec_info = v3_get_channel_codec(ev->channel.id))) {
                            label->set_text(codec_info->name);
                        } else {
                            label->set_text("Unsupported Codec");
                        }
                    }
                    v3_free_channel(c);
                }
                break;/*}}}*/
            case V3_EVENT_USER_LOGOUT:/*{{{*/
                if (v3_is_loggedin()) {
                    if (outputAudio[ev->user.id]) {
                        outputAudio[ev->user.id]->finish();
                        outputAudio.erase(ev->user.id);
                    }
                    if (v3_get_user_channel(v3_get_user_id()) == ev->channel.id) {
                        audioControl->playNotification("channelleave");
                    }
                    // can't get any user info... it's already gone by this point
                    //fprintf(stderr, "removing user id %d\n", ev->user.id);
                    channelTree->removeUser(ev->user.id);
                    chat->removeUser(ev->user.id);
#ifdef HAVE_XOSD
                    osd->removeUser(ev->user.id);
#endif
                }
                break;/*}}}*/
            case V3_EVENT_CHAN_REMOVE:/*{{{*/
                if (v3_is_loggedin()) {
                    // can't get any channel info... it's already gone by this point
                    //fprintf(stderr, "removing channel id %d\n", ev->channel.id);
                    channelTree->removeChannel(ev->channel.id);
                    admin->channelRemoved(ev->channel.id);
                }
                break;/*}}}*/
            case V3_EVENT_LOGIN_COMPLETE:/*{{{*/
                if (v3_is_loggedin()) {
                    const v3_codec *codec_info;

                    builder->get_widget("adminButton", button);
                    button->set_sensitive(true);
                    builder->get_widget("adminLoginMenuItem", menuitem);
                    menuitem->set_sensitive(true);
                    builder->get_widget("adminWindowMenuItem", menuitem);
                    menuitem->set_sensitive(true);
                    builder->get_widget("chatButton", button);
                    button->set_sensitive(true);
                    builder->get_widget("motdMenuItem", menuitem);
                    menuitem->set_sensitive(true);
                    builder->get_widget("chatMenuItem", menuitem);
                    menuitem->set_sensitive(true);
                    builder->get_widget("commentButton", button);
                    button->set_sensitive(true);
                    builder->get_widget("commentMenuItem", menuitem);
                    menuitem->set_sensitive(true);

                    builder->get_widget("codecLabel", label);
                    if ((codec_info = v3_get_channel_codec(0))) {
                        label->set_text(codec_info->name);
                    } else {
                        label->set_text("Unsupported Codec");
                    }
                    channelTree->expand_all();
                    audioControl->playNotification("login");
                    if (connectedServerName.length()) {
                        iniSection &server(config.servers[connectedServerName]);
                        if (server["PersistentComments"].toBool()) {
                            comment = server["Comment"].toUString();
                            url = server["URL"].toUString();
                            v3_set_text(
                                    (char *)ustring_to_c(comment).c_str(),
                                    (char *)ustring_to_c(url).c_str(),
                                    (char *)ustring_to_c(integration_text).c_str(),
                                    true);
                        }
                        uint32_t channel_id = v3_get_channel_id(ustring_to_c(server["DefaultChannel"].toUString()).c_str());
                        if (channel_id && (c = v3_get_channel(channel_id))) {
                            Glib::ustring password = channelTree->getChannelSavedPassword(channel_id);
                            uint16_t pw_channel = 0;
                            if ((pw_channel = v3_channel_requires_password(channel_id)) &&
                                (password = channelTree->getChannelSavedPassword(pw_channel)).empty() &&
                                (password = getPasswordEntry("Channel Password")).length()) {
                                channelTree->setChannelSavedPassword(pw_channel, password);
                            }
                            if (!pw_channel || (pw_channel && password.length())) {
                                v3_change_channel(channel_id, (char *)((pw_channel) ? password.c_str() : ""));
                            }
                            v3_free_channel(c);
                        }
                    }
                    if (chat->isOpen) {
                        v3_join_chat();
                    }
                    recorder->can_record(true);
#ifdef HAVE_G15
                    g15->addevent("connected to server");
                    g15->update(connectedServerName, "", "", "", "");
#endif
                }
                break;/*}}}*/
            case V3_EVENT_USER_CHAN_MOVE:/*{{{*/
                {
                    u = v3_get_user(ev->user.id);
                    if (! u) {
                        fprintf(stderr, "failed to retreive user information for user id %d\n", ev->user.id);
                        break;
                    }
                    if (ev->user.id == v3_get_user_id()) {
                        // we're moving channels... update the codec label
                        const v3_codec *codec_info;
                        builder->get_widget("codecLabel", label);
                        if ((codec_info = v3_get_channel_codec(ev->channel.id))) {
                            label->set_text(codec_info->name);
                        } else {
                            label->set_text("Unsupported Codec");
                        }
#ifdef HAVE_G15
                        if ((c = v3_get_channel(ev->channel.id))) {
                            string event = "switched to: ";
                            event.append(c->name);
                            g15->addevent(event);
                            free(c);
                        }
#endif
#ifdef HAVE_XOSD
                        osd->destroyOsd();
#endif
                    } else {
                        if (ev->channel.id == v3_get_user_channel(v3_get_user_id())) {
                            // they're joining our channel
                            audioControl->playNotification("channelenter");
#ifdef HAVE_G15
                            string event = "joined channel: ";
                            event.append(u->name);
                            g15->addevent(event);
                            g15->update("", "", "", u->name, "");
#endif
                        } else if (channelTree->getUserChannelId(ev->user.id) == v3_get_user_channel(v3_get_user_id())) {
                            // they're leaving our channel
                            audioControl->playNotification("channelleave");
#ifdef HAVE_G15
                            string event = "left channel: ";
                            event.append(u->name);
                            g15->addevent(event);
#endif
                        }
#ifdef HAVE_XOSD
                        osd->removeUser(ev->user.id);
#endif
                    }
                    if (u->rank_id) {
                        v3_rank *r;
                        if ((r = v3_get_rank(u->rank_id))) {
                            rank = c_to_ustring(r->name);
                            v3_free_rank(r);
                        }
                    }
                    //fprintf(stderr, "moving user id %d to channel id %d\n", ev->user.id, ev->channel.id);
                    Glib::ustring last_transmit = channelTree->getLastTransmit((uint32_t)ev->user.id);
                    channelTree->removeUser((uint32_t)ev->user.id);
                    channelTree->addUser(
                            (uint32_t)u->id,
                            (uint32_t)ev->channel.id,
                            c_to_ustring(u->name),
                            c_to_ustring(u->comment),
                            u->phonetic,
                            u->url,
                            c_to_ustring(u->integration_text),
                            (bool)u->guest,
                            (bool)u->real_user_id,
                            rank);
                    channelTree->setLastTransmit(ev->user.id, last_transmit);
                    if (connectedServerName.length() && u->id != v3_get_user_id()) {
                        // If we have a per user volume set for this user name, set it now.
                        if (config.hasUserVolume(connectedServerName, u->name)) {
                            v3_set_volume_user(u->id, config.UserVolume(connectedServerName, u->name).toInt());
                        }
                        // If the user was muted, mute them again.
                        if (config.UserMuted(connectedServerName, u->name).toBool()) {
                            channelTree->muteUserToggle(u->id);
                        }
                    }
                    // if there was an audio stream open for this user, close it
                    if (outputAudio[ev->user.id]) {
                        outputAudio[ev->user.id]->finish();
                        outputAudio.erase(ev->user.id);
                    }
                    channelTree->refreshAllUsers();
                    chat->chatUserTreeModelFilter->refilter();
                    v3_free_user(u);
                }
                break;/*}}}*/
            case V3_EVENT_CHAN_MOVE:/*{{{*/
                channelTree->refreshAllChannels();
                admin->channelResort();
                break;/*}}}*/
            case V3_EVENT_CHAN_ADD:/*{{{*/
                c = v3_get_channel(ev->channel.id);
                if (! c) {
                    fprintf(stderr, "failed to retreive channel information for channel id %d\n", ev->channel.id);
                    break;
                }
                channelTree->addChannel(
                        (uint8_t)c->protect_mode,
                        (uint32_t)c->id,
                        (uint32_t)c->parent,
                        c_to_ustring(c->name),
                        c_to_ustring(c->comment),
                        c->phonetic);
                if (! isAdmin && ! isChanAdmin && v3_is_channel_admin(c->id)) {
                    isChanAdmin = true;
                }
                admin->channelAdded(c);
                v3_free_channel(c);
                break;/*}}}*/
            case V3_EVENT_CHAN_BADPASS:/*{{{*/
                channelTree->forgetChannelSavedPassword(ev->channel.id);
                errorDialog(c_to_ustring(ev->error.message));
                break;/*}}}*/
            case V3_EVENT_ERROR_MSG:/*{{{*/
                errorDialog(c_to_ustring(ev->error.message));
                break;/*}}}*/
            case V3_EVENT_USER_TALK_START:/*{{{*/
                if (v3_is_loggedin()) {
                    channelTree->refreshUser(ev->user.id);
                }
#ifdef HAVE_G15
                if (v3_get_user_channel(ev->user.id) == v3_get_user_channel(v3_get_user_id())) {
                    if ((u = v3_get_user(ev->user.id))) {
                        string event = "talking: ";
                        event.append(u->name);
                        g15->addevent(event);
                        g15->update("", u->name, "", "", "");
                        free(u);
                    }
                }
#endif
                break;/*}}}*/
            case V3_EVENT_USER_TALK_END:
            case V3_EVENT_USER_TALK_MUTE:/*{{{*/
                if (v3_is_loggedin()) {
                    channelTree->refreshUser(ev->user.id);
#ifdef HAVE_XOSD
                    osd->removeUser(ev->user.id);
#endif
                    if (outputAudio[ev->user.id]) {
                        outputAudio[ev->user.id]->finish();
                        outputAudio.erase(ev->user.id);
                    }
                }
                break;/*}}}*/
            case V3_EVENT_PLAY_AUDIO:/*{{{*/
                if (v3_is_loggedin()) {
                    channelTree->setUserIcon(ev->user.id, "green", true);
#ifdef HAVE_XOSD
                    osd->addUser(ev->user.id);
#endif
                    if (!channelTree->isMuted(ev->user.id) && !muteSound) {
                        // Open a stream if we don't have one for this user
                        if (!outputAudio[ev->user.id]) {
                            outputAudio[ev->user.id] = new ManglerAudio(AUDIO_OUTPUT, ev->pcm.rate, ev->pcm.channels);
                        }
                        // And queue the audio
                        if (outputAudio[ev->user.id]) {
                            outputAudio[ev->user.id]->queue(ev->pcm.length, (uint8_t *)ev->data->sample);
                        }
                    } else if (outputAudio[ev->user.id]) {
                        outputAudio[ev->user.id]->finish();
                        outputAudio.erase(ev->user.id);
                    }
                }
                break;/*}}}*/
            case V3_EVENT_RECORD_UPDATE:/*{{{*/
                recorder->record(
                        c_to_ustring(ev->text.name),
                        c_to_ustring(ev->status.message),
                        ev->record.index,
                        ev->record.time,
                        ev->record.stopped,
                        ev->record.flushed);
                break;/*}}}*/
            case V3_EVENT_DISPLAY_MOTD:/*{{{*/
                {
                    Glib::ustring motdKey;
                    uint32_t motdhash = 0;

                    if (!ev->flags) {
                        motdKey = "MotdHashUser";
                        motdNotebook->set_show_tabs(true);
                        motdNotebook->set_current_page(0);
                        motdUsers->get_buffer()->set_text(c_to_ustring(stripMotdRtf(ev->data->motd).c_str()));
                    } else {
                        motdKey = "MotdHash";
                        motdGuests->get_buffer()->set_text(c_to_ustring(stripMotdRtf(ev->data->motd).c_str()));
                    }
                    motdIgnore->set_sensitive(!connectedServerName.empty());
                    motdIgnore->set_active(connectedServerName.length() && config.servers[connectedServerName]["MotdIgnore"].toBool());
                    if (motdIgnore->get_active() || !strlen(ev->data->motd)) {
                        break;
                    }
                    if (connectedServerName.length()) {
                        // we're not launching a space shuttle here, no need for
                        // anything super complex
                        for (uint32_t ctr = 0; ctr < strlen(ev->data->motd); ctr++) {
                            motdhash += ev->data->motd[ctr] + ctr;
                        }
                    }
                    if (motdAlways || connectedServerName.empty() || config.servers[connectedServerName][motdKey].toULong() != motdhash) {
                        if (connectedServerName.length()) {
                            config.servers[connectedServerName][motdKey] = motdhash;
                            config.servers.save();
                        }
                        motdOkButton->grab_focus();
                        motdWindow->show();
                    }
                }
                break;/*}}}*/
            case V3_EVENT_DISCONNECT:/*{{{*/
                onDisconnectHandler();
                audioControl->playNotification("logout");
                break;/*}}}*/
            case V3_EVENT_CHAT_JOIN:/*{{{*/
                {
                    chat->addUser(ev->user.id);
                    u = v3_get_user(ev->user.id);
                    if (!u) {
                        fprintf(stderr, "couldn't retreive user id %d\n", ev->user.id);
                        break;
                    }
                    if (u->id != 0) {
                        if (u->rank_id) {
                            v3_rank *r;
                            if ((r = v3_get_rank(u->rank_id))) {
                                rank = c_to_ustring(r->name);
                                v3_free_rank(r);
                            }
                        }
                        channelTree->updateUser(
                                (uint32_t)u->id,
                                (uint32_t)u->channel,
                                c_to_ustring(u->name),
                                c_to_ustring(u->comment),
                                u->phonetic,
                                u->url,
                                c_to_ustring(u->integration_text),
                                (bool)u->guest,
                                (bool)u->real_user_id,
                                rank);
                    }
                    v3_free_user(u);
                }
                break;/*}}}*/
            case V3_EVENT_CHAT_LEAVE:/*{{{*/
                {
                    chat->removeUser(ev->user.id);
                    u = v3_get_user(ev->user.id);
                    if (!u) {
                        fprintf(stderr, "couldn't retreive user id %d\n", ev->user.id);
                        break;
                    }
                    if (u->id != 0) {
                        if (u->rank_id) {
                            v3_rank *r;
                            if ((r = v3_get_rank(u->rank_id))) {
                                rank = c_to_ustring(r->name);
                                v3_free_rank(r);
                            }
                        }
                        channelTree->updateUser(
                                (uint32_t)u->id,
                                (uint32_t)u->channel,
                                c_to_ustring(u->name),
                                c_to_ustring(u->comment),
                                u->phonetic,
                                u->url,
                                c_to_ustring(u->integration_text),
                                (bool)u->guest,
                                (bool)u->real_user_id,
                                rank);
                    }
                    v3_free_user(u);
                }
                break;/*}}}*/
            case V3_EVENT_CHAT_MESSAGE:/*{{{*/
                if (v3_is_loggedin()) {
                    if (ev->user.id == 0) {
                        chat->addRconMessage(c_to_ustring(ev->data->chatmessage));
                    } else {
                        chat->addChatMessage(ev->user.id, c_to_ustring(ev->data->chatmessage));
                    }
                }
                break;/*}}}*/
            case V3_EVENT_PRIVATE_CHAT_START:/*{{{*/
                {
                    uint16_t remote;
                    if (ev->user.privchat_user1 == v3_get_user_id()) {
                        remote = ev->user.privchat_user2;
                    } else {
                        remote = ev->user.privchat_user1;
                    }
                    if (privateChatWindows[remote]) {
                        privateChatWindows[remote]->remoteReopened();
                    } else {
                        v3_user *u;
                        Glib::ustring name = "unknown";
                        if ((u = v3_get_user(remote)) != NULL) {
                            name = c_to_ustring(u->name);
                            v3_free_user(u);
                        }
                        privateChatWindows[remote] = new ManglerPrivChat(remote);
                        privateChatWindows[remote]->addMessage("*** opened private chat with " + name);
                    }
                }
                break;/*}}}*/
            case V3_EVENT_PRIVATE_CHAT_END:/*{{{*/
                {
                    if (privateChatWindows[ev->user.privchat_user2]) {
                        privateChatWindows[ev->user.privchat_user2]->remoteClosed();
                    }
                }
                break;/*}}}*/
            case V3_EVENT_PRIVATE_CHAT_AWAY:/*{{{*/
                {
                    if (privateChatWindows[ev->user.privchat_user2]) {
                        privateChatWindows[ev->user.privchat_user2]->remoteAway();
                    }
                }
                break;/*}}}*/
            case V3_EVENT_PRIVATE_CHAT_BACK:/*{{{*/
                {
                    if (privateChatWindows[ev->user.privchat_user2]) {
                        privateChatWindows[ev->user.privchat_user2]->remoteBack();
                    }
                }
                break;/*}}}*/
            case V3_EVENT_PRIVATE_CHAT_MESSAGE:/*{{{*/
                {
                    uint16_t remote;
                    if (ev->user.privchat_user1 == v3_get_user_id()) {
                        remote = ev->user.privchat_user2;
                    } else {
                        remote = ev->user.privchat_user1;
                    }
                    if (privateChatWindows[remote]) {
                        if (!ev->flags) { // set to true on error
                            privateChatWindows[remote]->addChatMessage(ev->user.privchat_user2, c_to_ustring(ev->data->chatmessage));
                        } else {
                            privateChatWindows[remote]->addMessage("*** error sending message to remote user");
                        }
                    }
                }
                break;/*}}}*/
            case V3_EVENT_TEXT_TO_SPEECH_MESSAGE:/*{{{*/
                {
                    if ((u = v3_get_user(ev->user.id))) {
                        //fprintf(stderr, "TTS: %s: %s\n", u->name, ev->data->chatmessage);
                        audioControl->playText(c_to_ustring((strlen(u->phonetic)) ? u->phonetic : u->name) + ": " + c_to_ustring(ev->data->chatmessage));
                        v3_free_user(u);
                    }
                }
                break;/*}}}*/
            case V3_EVENT_PLAY_WAVE_FILE_MESSAGE:/*{{{*/
                {
                    /*
                    if ((u = v3_get_user(ev->user.id))) {
                        fprintf(stderr, "WAV: %s: %s\n", u->name, ev->data->chatmessage);
                        v3_free_user(u);
                    }
                    */
                }
                break;/*}}}*/
            case V3_EVENT_USER_PAGE:/*{{{*/
                {
                    if ((u = v3_get_user(ev->user.id))) {
                        //fprintf(stderr, "Page from: %s\n", u->name);
                        audioControl->playText("Page from: " + c_to_ustring((strlen(u->phonetic)) ? u->phonetic : u->name));
                        v3_free_user(u);
                    }
                }
                break;/*}}}*/
            case V3_EVENT_ADMIN_AUTH:/*{{{*/
                {
                    const v3_permissions *perms = v3_get_permissions();
                    if (perms->srv_admin && !isAdmin) {
                        isAdmin = true;
                        builder->get_widget("adminLoginMenuItem", menuitem);
                        menuitem->set_label("_Admin Logout");
                        if (wantAdminWindow) {
                            wantAdminWindow = false;
                            admin->show();
                        }
                    } else {
                        isAdmin = false;
                        builder->get_widget("adminLoginMenuItem", menuitem);
                        menuitem->set_label("_Admin Login");
                    }
                    v3_user *lobby;
                    if ((lobby = v3_get_user(0))) {
                        channelTree->updateLobby(c_to_ustring(lobby->name), c_to_ustring(lobby->comment), c_to_ustring(lobby->phonetic));
                        v3_free_user(lobby);
                    }
                }
                break;/*}}}*/
            case V3_EVENT_CHAN_ADMIN_UPDATED:/*{{{*/
                channelTree->refreshAllChannels();
                break;/*}}}*/
            case V3_EVENT_USER_GLOBAL_MUTE_CHANGED:
            case V3_EVENT_USER_CHANNEL_MUTE_CHANGED:/*{{{*/
                channelTree->refreshUser(ev->user.id);
                if (outputAudio[ev->user.id]) {
                    outputAudio[ev->user.id]->finish();
                    outputAudio.erase(ev->user.id);
                }
                break;/*}}}*/
            case V3_EVENT_SERVER_PROPERTY_UPDATED:/*{{{*/
                switch (ev->serverproperty.property) {
                    case V3_SRV_PROP_CHAT_FILTER:
                        chat->isPerChannel = ev->serverproperty.value;
                        chat->chatUserTreeModelFilter->refilter();
                        break;
                    case V3_SRV_PROP_CHAN_ORDER:
                        channelTree->sortManual = ev->serverproperty.value;
                        channelTree->refreshAllChannels();
                        admin->channelSort(ev->serverproperty.value);
                        break;
                    case V3_SRV_PROP_MOTD_ALWAYS:
                        motdAlways = ev->serverproperty.value;
                        break;
                }
                break;/*}}}*/
            case V3_EVENT_USERLIST_ADD:/*{{{*/
                {
                    v3_account *account = v3_get_account(ev->account.id);
                    if (account) {
                        admin->accountAdded(account);
                        v3_free_account(account);
                    }
                }
                break;/*}}}*/
            case V3_EVENT_USERLIST_REMOVE:/*{{{*/
                admin->accountRemoved(ev->account.id);
                break;/*}}}*/
            case V3_EVENT_USERLIST_MODIFY:/*{{{*/
                {
                    v3_account *account = v3_get_account(ev->account.id);
                    if (account) {
                        admin->accountUpdated(account);
                        v3_free_account(account);
                    }
                }
                break;/*}}}*/
            case V3_EVENT_RANK_ADD:/*{{{*/
                {
                    v3_rank *rank = v3_get_rank(ev->data->rank.id);
                    admin->rankAdded(rank);
                }
                break;/*}}}*/
            case V3_EVENT_RANK_REMOVE:/*{{{*/
                {
                    admin->rankRemoved(ev->data->rank.id);
                }
                break;/*}}}*/
            case V3_EVENT_RANK_MODIFY:/*{{{*/
                {
                    v3_rank *rank = v3_get_rank(ev->data->rank.id);
                    admin->rankUpdated(rank);
                }
                break;/*}}}*/
            case V3_EVENT_PERMS_UPDATED:/*{{{*/
                admin->permsUpdated();
                break;/*}}}*/
            case V3_EVENT_USER_RANK_CHANGE:/*{{{*/
                {
                    if (v3_is_loggedin()) {
                        u = v3_get_user(ev->user.id);
                        if (!u) {
                            fprintf(stderr, "couldn't retreive user id %d\n", ev->user.id);
                            break;
                        }
                        if (u->id == 0) {
                            channelTree->updateLobby(c_to_ustring(u->name), c_to_ustring(u->comment), c_to_ustring(u->phonetic));
                        } else {
                            if (u->rank_id) {
                                v3_rank *r;
                                if ((r = v3_get_rank(u->rank_id))) {
                                    rank = c_to_ustring(r->name);
                                    v3_free_rank(r);
                                }
                            }
                            channelTree->updateUser(
                                    (uint32_t)u->id,
                                    (uint32_t)u->channel,
                                    c_to_ustring(u->name),
                                    c_to_ustring(u->comment),
                                    u->phonetic,
                                    u->url,
                                    c_to_ustring(u->integration_text),
                                    (bool)u->guest,
                                    (bool)u->real_user_id,
                                    rank);
                        }
                        v3_free_user(u);
                    }
                }
                break;/*}}}*/
            case V3_EVENT_SRV_PROP_RECV:/*{{{*/
                admin->serverSettingsUpdated(ev->data->srvprop);
                break;/*}}}*/
            case V3_EVENT_SRV_PROP_SENT:/*{{{*/
                admin->serverSettingsSendDone();
                break;/*}}}*/
            case V3_EVENT_ADMIN_BAN_LIST:/*{{{*/
                admin->banList(
                        ev->data->ban.id,
                        ev->data->ban.count,
                        ev->data->ban.bitmask_id,
                        ev->data->ban.ip_address,
                        ev->data->ban.user,
                        ev->data->ban.by,
                        ev->data->ban.reason);
                break;/*}}}*/
            default:
                fprintf(stderr, "******************************************************** got unknown event type %d\n", ev->type);
        }
        channelTree->expand_all();
        v3_free_event(ev);
        gdk_threads_leave();
    }
    return true;
}/*}}}*/

bool Mangler::checkPushToTalkKeys(void) {/*{{{*/
    char        pressed_keys[32];
    GdkWindow   *rootwin = gdk_get_default_root_window();
    vector<int>::iterator i;
    bool        ptt_on = true;

    if (! config["PushToTalkKeyEnabled"].toBool()) {
        isTransmittingKey = false;
        return true;
    }
    vector<int> ptt_keycodes = config.PushToTalkXKeyCodes();
    XQueryKeymap(GDK_WINDOW_XDISPLAY(rootwin), pressed_keys);
    for (   i = ptt_keycodes.begin();
            i < ptt_keycodes.end();
            i++) {
        if (!((pressed_keys[*i >> 3] >> (*i & 0x07)) & 0x01)) {
            ptt_on = false;
            break;
        }
    }
    if (ptt_on) {
        isTransmittingKey = true;
        startTransmit();
    } else {
        isTransmittingKey = false;
        if (! isTransmittingButton && ! isTransmittingMouse && ! isTransmittingVA) {
            stopTransmit();
        }
    }
    return(true);

}/*}}}*/
bool Mangler::checkVoiceActivation(void) {/*{{{*/
    if (Mangler::config["VoiceActivationEnabled"].toBool()) {
        isTransmittingVA = true;
        startTransmit();
    } else {
        isTransmittingVA = false;
        if (! isTransmittingButton && ! isTransmittingMouse && ! isTransmittingKey) {
            stopTransmit();
        }
    }
    return true;
}/*}}}*/
bool Mangler::checkPushToTalkMouse(void) {/*{{{*/
    GdkWindow   *rootwin = gdk_get_default_root_window();
    XDeviceInfo *xdev;
    XDeviceState *xds;
    XButtonState *xbs = NULL;
    XInputClass *xic = NULL;
    int ctr;
    int ndevices_return;
    int state = 1;
    int byte = config["PushToTalkMouseValue"].toInt() / 8;
    int bit = config["PushToTalkMouseValue"].toInt() % 8;
    bool        ptt_on = false;

    if (! config["PushToTalkMouseEnabled"].toBool()) {
        isTransmittingMouse = false;
        return true;
    }
    if (config["MouseDeviceName"].empty()) {
        return true;
    }

    if (CurrentOpenMouse != config["MouseDeviceName"].toString()) {
        if (dev) {
            XCloseDevice(GDK_WINDOW_XDISPLAY(rootwin), dev);
        }
        xdev = XListInputDevices(GDK_WINDOW_XDISPLAY(rootwin), &ndevices_return);
        for (ctr = 0; ctr < ndevices_return; ctr++) {
            Glib::ustring name = xdev[ctr].name;
            if (config["MouseDeviceName"] == name && xdev[ctr].use == IsXExtensionPointer) {
                break;
            }
        }
        if (ctr == ndevices_return) {
            XFreeDeviceList(xdev);
            return true;
        }
        dev = XOpenDevice(GDK_WINDOW_XDISPLAY(rootwin), xdev[ctr].id);
        XFreeDeviceList(xdev);
        if (! dev) {
            return true;
        }
        CurrentOpenMouse = config["MouseDeviceName"].toString();
    }
    xds = (XDeviceState *)XQueryDeviceState(GDK_WINDOW_XDISPLAY(rootwin), dev);
    for (ctr = 0, xic = xds->data; ctr < xds->num_classes; ctr++, xic += xic->length/2) {
        if (xic->c_class == ButtonClass) {
            xbs = (XButtonState*) xic;
        }
    }
    if (!xbs) {
        return true;
    }
    state = state << bit;
    /* debug mouse state buttons
    for (int ctr = 1; ctr < 10; ctr++) {
        int byte = ctr / 8;
        int bit = ctr % 8;
        state = 1 << bit;
        fprintf(stderr, "(%d/%d)", byte, bit);
        fprintf(stderr, "b%d: %d -- ", ctr, (xbs->buttons[byte] & state) >> bit);
    }
    fprintf(stderr, "\n");
    */
    if ((xbs->buttons[byte] & state) >> bit) {
        ptt_on = true;
    }
    XFreeDeviceState(xds);

    if (ptt_on) {
        isTransmittingMouse = true;
        startTransmit();
    } else {
        isTransmittingMouse = false;
        if (! isTransmittingButton && ! isTransmittingKey && ! isTransmittingVA) {
            stopTransmit();
        }
    }
    return(true);
}/*}}}*/
bool Mangler::updateXferAmounts(void) {/*{{{*/
    double bytes;
    uint32_t packets;
    char buf[1024];

    builder->get_widget("sentLabel", label);
    bytes = v3_get_bytes_sent();
    packets = v3_get_packets_sent();
    if (bytes > 1024*1024) {
        snprintf(buf, 1023, "%2.2f megabytes / %u packets", bytes/1024/1024, packets);
    } else if (bytes > 1024) {
        snprintf(buf, 1023, "%2.2f kilobytes / %u packets", bytes/1024, packets);
    } else if (bytes) {
        snprintf(buf, 1023, "%.0f bytes / %u packets", bytes, packets);
    } else {
        snprintf(buf, 1023, "N/A");
    }
    label->set_text(buf);

    builder->get_widget("recvLabel", label);
    bytes = v3_get_bytes_recv();
    packets = v3_get_packets_recv();
    if (bytes > 1024*1024) {
        snprintf(buf, 1023, "%2.2f megabytes / %u packets", bytes/1024/1024, packets);
    } else if (bytes > 1024) {
        snprintf(buf, 1023, "%2.2f kilobytes / %u packets", bytes/1024, packets);
    } else if (bytes) {
        snprintf(buf, 1023, "%.0f bytes / %u packets", bytes, packets);
    } else {
        snprintf(buf, 1023, "N/A");
    }
    label->set_text(buf);

    return(true);
}/*}}}*/
/* {{{ GdkFilterReturn ptt_filter(GdkXEvent *gdk_xevent, GdkEvent *event, gpointer data) {
    GdkWindow   *rootwin = gdk_get_default_root_window();
    XEvent *xevent = (XEvent *)gdk_xevent;

    if (! mangler) {
        return GDK_FILTER_CONTINUE;
    }
    if (! mangler->settings->config.PushToTalkMouseEnabled) {
        mangler->isTransmittingKey = false;
        return GDK_FILTER_CONTINUE;
    }
    if (xevent->type == ButtonPress && !mangler->isTransmittingMouse && xevent->xbutton.button == mangler->settings->config.PushToTalkMouseValueInt) {
        fprintf(stderr, "press\n");
        mangler->startTransmit();
        mangler->isTransmittingMouse = true;
        fprintf(stderr, "allow\n");
        XAllowEvents(GDK_WINDOW_XDISPLAY(rootwin), AsyncPointer, CurrentTime);
        fprintf(stderr, "ungrab pointer\n");
        XUngrabPointer(GDK_WINDOW_XDISPLAY(rootwin), CurrentTime);
        fprintf(stderr, "ungrab button\n");
        XUngrabButton(GDK_WINDOW_XDISPLAY(rootwin), mangler->settings->config.PushToTalkMouseValueInt, AnyModifier, GDK_ROOT_WINDOW());
        fprintf(stderr, "grab button\n");
        XGrabButton(GDK_WINDOW_XDISPLAY(rootwin),   mangler->settings->config.PushToTalkMouseValueInt, AnyModifier, GDK_ROOT_WINDOW(), True, ButtonPressMask|ButtonReleaseMask, GrabModeAsync, GrabModeAsync, None, None);
        return GDK_FILTER_CONTINUE;
    } else if ((xevent->type == ButtonRelease) && (xevent->xbutton.button == mangler->settings->config.PushToTalkMouseValueInt)) {
        fprintf(stderr, "release\n");
        mangler->stopTransmit();
        mangler->isTransmittingMouse = false;
        fprintf(stderr, "allow\n");
        XAllowEvents(GDK_WINDOW_XDISPLAY(rootwin), AsyncPointer, CurrentTime);
        fprintf(stderr, "ungrab pointer\n");
        XUngrabPointer(GDK_WINDOW_XDISPLAY(rootwin), CurrentTime);
        fprintf(stderr, "ungrab button\n");
        XUngrabButton(GDK_WINDOW_XDISPLAY(rootwin), mangler->settings->config.PushToTalkMouseValueInt, AnyModifier, GDK_ROOT_WINDOW());
        fprintf(stderr, "grab button\n");
        XGrabButton(GDK_WINDOW_XDISPLAY(rootwin), mangler->settings->config.PushToTalkMouseValueInt, AnyModifier, GDK_ROOT_WINDOW(), True, ButtonPressMask|ButtonReleaseMask, GrabModeAsync, GrabModeAsync, None, None);
        return GDK_FILTER_CONTINUE;
    }
    return GDK_FILTER_CONTINUE;
} }}} */
bool Mangler::updateIntegration(void) {/*{{{*/
    if (v3_is_loggedin()) {
        if (! config["AudioIntegrationEnabled"].toBool() || integration->client == MusicClient_None) {
            if (integration_text != "") {
                    integration_text = "";
                    v3_set_text(
                            (char *)ustring_to_c(comment).c_str(),
                            (char *)ustring_to_c(url).c_str(),
                            (char *)"",
                            true);
            }
            return true;
        }
        Glib::ustring formatted_text = integration->format();
        switch (integration->get_mode()) {
            // Polling (mpd)
            case 0:
                if ( ((integration->update(false) || !integration->first()) ) || integration_text != formatted_text ) {
                    integration_text =  integration->format();
                    v3_set_text(
                            (char *)ustring_to_c(comment).c_str(),
                            (char *)ustring_to_c(url).c_str(),
                            (char *)ustring_to_c(integration_text).c_str(),
                            true);
                }
                break;

                // Listening / callbacks (dbus ones)
            case 1:
                if (integration_text != formatted_text) {
                    integration_text = formatted_text;
                    v3_set_text(
                            (char *)ustring_to_c(comment).c_str(),
                            (char *)ustring_to_c(url).c_str(),
                            (char *)ustring_to_c(integration_text).c_str(),
                            true);
                }
                break;
        }
    }
    return true;
}/*}}}*/

Glib::ustring Mangler::getPasswordEntry(Glib::ustring title, Glib::ustring prompt) {/*{{{*/
    password = "";
    passwordEntry->set_text("");
    passwordDialog->set_keep_above(true);
    passwordDialog->set_title(title);
    passwordDialog->run();
    passwordDialog->hide();
    return(password);
}/*}}}*/
void Mangler::passwordDialogOkButton_clicked_cb(void) {/*{{{*/
    password = passwordEntry->get_text();
}/*}}}*/
void Mangler::passwordDialogCancelButton_clicked_cb(void) {/*{{{*/
    password = "";
}/*}}}*/

bool Mangler::getReasonEntry(Glib::ustring title, Glib::ustring prompt) {/*{{{*/
    reason = "";
    reasonValid = false;
    reasonEntry->set_text("");
    reasonDialog->set_keep_above(true);
    reasonDialog->set_title(title);
    reasonDialog->run();
    reasonDialog->hide();
    return reasonValid;
}/*}}}*/
void Mangler::reasonDialogOkButton_clicked_cb(void) {/*{{{*/
    reason = reasonEntry->get_text();
    reasonValid = true;
}/*}}}*/
void Mangler::reasonDialogCancelButton_clicked_cb(void) {/*{{{*/
    reasonValid = false;
}/*}}}*/

void Mangler::textStringChangeDialogOkButton_clicked_cb(void) {/*{{{*/
    comment          = textStringChangeCommentEntry->get_text();
    url              = textStringChangeURLEntry->get_text();
    integration_text = textStringChangeIntegrationEntry->get_text();
    if (! connectedServerName.empty()) {
        if (config.servers[connectedServerName]["PersistentComments"].toBool()) {
            config.servers[connectedServerName]["Comment"] = comment;
            config.servers.save();
        }
    }
    v3_set_text(
            (char *)ustring_to_c(comment).c_str(),
            (char *)ustring_to_c(url).c_str(),
            (char *)ustring_to_c(integration_text).c_str(),
            textStringSilenceCommentCheckButton->get_active());
}/*}}}*/
void Mangler::textStringChangeDialogCancelButton_clicked_cb(void) {/*{{{*/
    textStringChangeCommentEntry->set_text(comment);
    textStringChangeURLEntry->set_text(url);
    textStringChangeIntegrationEntry->set_text(integration_text);
}/*}}}*/

// Misc Functions
uint32_t Mangler::getActiveServer(void) {/*{{{*/
    builder->get_widget("serverSelectComboBox", combobox);
    return combobox->get_active_row_number();
}/*}}}*/
void Mangler::setActiveServer(uint32_t row_number) {/*{{{*/
    builder->get_widget("serverSelectComboBox", combobox);
    combobox->set_active(row_number);
}/*}}}*/
void Mangler::errorDialog(Glib::ustring message) {/*{{{*/
    builder->get_widget("errorWindow", window);
    window->set_keep_above(true);
    window->set_icon(icons["tray_icon"]);
    builder->get_widget("errorMessageLabel", label);
    label->set_text(message);
    window->show();
    /*
    builder->get_widget("errorDialog", msgdialog);
    msgdialog->set_icon(icons["tray_icon"]);
    msgdialog->set_keep_above(true);
    msgdialog->set_message(message);
    msgdialog->run();
    msgdialog->hide();
    */
}/*}}}*/
void Mangler::errorOKButton_clicked_cb(void) {/*{{{*/
    builder->get_widget("errorWindow", window);
    window->hide();
}/*}}}*/

ManglerError::ManglerError(uint32_t code, Glib::ustring message, Glib::ustring module) {/*{{{*/
    this->code = code;
    this->message = message;
    this->module = module;
}/*}}}*/

std::string Mangler::stripMotdRtf(const char *input) {/*{{{*/
    // commentary by x87bliss (Russ Kubes) for your reading pleasure

    std::string motd; // I use std::string since the RTF in the MOTDs is not unicode

    if (!input || strlen(input) < 5 || memcmp("{\\rtf", input, 5) != 0) { // if the motd is not RTF
        motd = input;
        return motd;
    }

    // Note this function only strips RTF markup to make RTF MOTDs human readable. It doesn't support any advanced interpretation of RTF markup.
    size_t inputlen = strlen(input);
    motd.reserve(inputlen); // allocate enough space, the resulting string will be smaller than input length
    // rtf_status is what is the loop looking at, i.e. do we need to interpret controls
    enum {RTF_TEXT, RTF_CONTROL, RTF_CONTROL_END, RTF_IGNORE, RTF_BIN} rtf_status = RTF_TEXT;

    size_t ignorecount; // number of closing braces to ignore for "\*"
    size_t controlpos; // stores the position of the first character of a control word

    for (size_t pos = 0; pos < inputlen; ++pos) {
        if (rtf_status == RTF_IGNORE) {
            if (inputlen - pos > 4 && memcmp("\\bin", input + pos, 4) == 0 && input[pos + 4] >= '0' && input[pos + 4] <= '9') {
                // Encountered some binary data that we have to ignore. We have to ignore this a special way since the data may contain special characters
                pos += 4;
                size_t binskip = 0; // how much data to skip
                for (; input[pos] >= '0' && input[pos] <= '9'; ++pos) {
                    binskip = (binskip * 10) + (input[pos] - '0');
                }
                pos += binskip;
                continue; // every \binN has a space between it and its binary data, continue skips that too.
            }
            if (input[pos] == '{') {
                ++ignorecount;
            }
            if (input[pos] == '}' && --ignorecount == 0) {
                rtf_status = RTF_TEXT; // we can stop ignoring now
            }
            continue; // still ignoring, or ignore the ending brace;
        }
        if (rtf_status == RTF_CONTROL) {
            if ((input[pos] >= 'a' && input[pos] <= 'z') || // valid characters in a control word are a-z, A-Z, 0-9, and - (for negative numbers)
                (input[pos] >= 'A' && input[pos] <= 'Z') ||
                (input[pos] >= '0' && input[pos] <= '9') ||
                (input[pos] == '\'' && pos == controlpos) || // apostrophe is valid, but only as the first character of a control word
                input[pos] == '-') {
                continue;
            } else {
                rtf_status = RTF_CONTROL_END; // Any control word has ended
            }
        }
        if (rtf_status == RTF_CONTROL_END) {
            // note all RTF control words are case-sensitive. So memcmp can be used instead of stricmp
            size_t controllen = pos - controlpos;
            // check all exact matches first
            if (controllen == 3 && input[controlpos] == '\'') { // 8-bit hex character
                char hh = -1, h; // hh is the full interpreted hex character, h is a working value for each hex digit
                h = input[controlpos + 1];
                if (h >= '0' && h <= '9') {
                    hh = h - '0'; // temporarily store the value and check for errors in the second digit
                } else if (h >= 'a' && h <= 'f') {
                    hh = h - 'a' + 10;
                } else if (h >= 'A' && h <= 'F') {
                    hh = h - 'A' + 10; // technically only lowercase letters are supported by RTF
                }
                h = input[controlpos + 2];
                if (h >= '0' && h <= '9') {
                    h = h - '0'; // temporarily store the value
                } else if (h >= 'a' && h <= 'f') {
                    h = h - 'a' + 10;
                } else if (h >= 'A' && h <= 'F') {
                    h = h - 'A' + 10; // technically only lowercase letters are supported by RTF
                } else {
                    h = -1;
                }
                if (hh >= 0 && h >= 0) { // both hex digits were valid
                    hh = (hh * 16) + h; // convert each value to the whole character
                    motd.append(1, hh);
                }
            } else if (controllen == 3 && memcmp("tab", input + controlpos, 3) == 0) {
                motd.append(1, '\t'); // in case Mangler's motd window doesn't properly handle tabs, this can be changed to (4, ' ') for 4 spaces
            } else if (controllen == 3 && memcmp("par", input + controlpos, 3) == 0) {
                motd.append("\r\n");
            } else if (controllen == 4 && (memcmp("line", input + controlpos, 4) == 0 ||
                    memcmp("page", input + controlpos, 4) == 0 || memcmp("sect", input + controlpos, 4) == 0)) {
                motd.append("\r\n");
            } else if (controllen == 7 && (memcmp("fonttbl", input + controlpos, 7) == 0 || // nonvisible formatting table
                    memcmp("filetbl", input + controlpos, 4) == 0)) { // sub-documents table
                ignorecount = 1;
                rtf_status = RTF_IGNORE; // we're going to ignore a table
            } else if (controllen == 8 && memcmp("colortbl", input + controlpos, 8) == 0) { // nonvisible formatting table
                ignorecount = 1;
                rtf_status = RTF_IGNORE; // we're going to ignore a table
            } else if (controllen == 10 && (memcmp("stylesheet", input + controlpos, 10) == 0 || // nonvisible formatting table
                    memcmp("listtables", input + controlpos, 10) == 0)) { // stores formatting information for lists
                ignorecount = 1;
                rtf_status = RTF_IGNORE; // we're going to ignore a table
            } else if (controllen == 6 && memcmp("revtbl", input + controlpos, 6) == 0) { // This table contains nonvisible information about revisions
                ignorecount = 1;
                rtf_status = RTF_IGNORE; // we're going to ignore a table
            } else if (controllen == 4 && memcmp("info", input + controlpos, 4) == 0) { // The info table contains nonvisible metadata, like author etc..
                ignorecount = 1;
                rtf_status = RTF_IGNORE; // we're going to ignore a table
            } else if (controllen > 3 && memcmp("bin", input + controlpos, 3) == 0 && input[controlpos + 3] >= '0' && input[controlpos + 3] <= '9') {
                // Now start to look for partial matches
                // We encountered some binary data that we need to skip
                size_t binskip = 0; // how much data to skip
                for (size_t bpos = controlpos + 3; input[bpos] >= '0' && input [bpos] <= '9'; ++bpos) {
                    binskip = (binskip * 10) + (input[bpos] - '0');
                }
                pos += binskip;
                rtf_status = RTF_TEXT;
                continue;
            }
            if (input[pos] != ' ') {
                --pos; // we need to reevaluate the delimiter if it wasn't a space
            }
            if (rtf_status != RTF_IGNORE) {
                rtf_status = RTF_TEXT;
            }
            continue;
        }
        if (rtf_status == RTF_TEXT) {
            if (input[pos] == '\\') {
                // First check for "\{", "\}", "\\" and "\*", which each would break this function if not explicitly looked for right away
                if (inputlen - pos < 2) {
                    break; // we need at least 2 characters, probably a malformed RTF message.
                }
                ++pos; // increment it here once, so we don't have to add one for each check
                if (input[pos] == '{' || input[pos] == '}' || input[pos] == '\\') {
                    motd.append(1, input[pos]);
                    continue;
                } else if (input[pos] == '*') {
                    ignorecount = 1;
                    rtf_status = RTF_IGNORE;
                    continue;
                } else {
                    controlpos = pos--; // the control word starts at the incremented pos, but we need to decrement it again to examine it
                    rtf_status = RTF_CONTROL;
                    continue;
                }
            } else if (input[pos] != '{' && input[pos] != '}' && // we're just ignoring groups since we're not formatting
                    input[pos] != '\r' && input[pos] != '\n') { // ignore CRs and LFs in the RTF text
                motd.append(1, input[pos]);
            }
        }
    }
    return motd;
}/*}}}*/

int
main(int argc, char **argv) {
    Gtk::Main kit(argc, argv);
    struct _cli_options options = { 0 };
    char *locale;

    extern char *optarg;
    int opt;

    while ((opt = getopt(argc, argv, "hd:s:u:p:")) != -1) {
        switch (opt) {
          case 'h':
            fprintf(stderr, "%s: optional arguments: -d <ui definition>.ui -s hostname:port -u username -p password\n", argv[0]);
            exit(EXIT_FAILURE);
          case 'd':
            options.uifilename = c_to_ustring(optarg);
            options.uifromfile = true;
            fprintf(stderr, "using ui definition file: %s\n", optarg);
            break;
          case 's':
            options.qc_server = c_to_ustring(optarg);
            break;
          case 'u':
            options.qc_username = c_to_ustring(optarg);
            break;
          case 'p':
            options.qc_password = c_to_ustring(optarg);
            break;
        }
    }
    if (!(locale = setlocale(LC_ALL, ""))) {
        fprintf(stderr, "Can't set the specified locale! " "Check LANG, LC_CTYPE, LC_ALL.\n");
        exit(EXIT_FAILURE);
    }
    //fprintf(stderr, "initialized locale: %s\n", locale);
    if (!Glib::thread_supported()) {
        Glib::thread_init();
    }
    gdk_threads_init();
    mangler = new Mangler(&options);
    gdk_threads_enter();
    Gtk::Main::run(*mangler->manglerWindow);
    gdk_threads_leave();
    delete mangler;

    exit(EXIT_SUCCESS);
    return 0;
}

