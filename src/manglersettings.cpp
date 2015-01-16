/*
 * vim: softtabstop=4 shiftwidth=4 cindent foldmethod=marker expandtab
 *
 * $LastChangedDate: 2012-05-24 22:31:28 +0430 (Thu, 24 May 2012) $
 * $Revision: 1160 $
 * $LastChangedBy: Haxar $
 * $URL: http://svn.mangler.org/mangler/trunk/src/manglersettings.cpp $
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
#include "mangler.h"
#include "manglersettings.h"
#include <gdk/gdkx.h>

#include "manglerintegration.h"
#include "mangleraudio.h"
#ifdef HAVE_XOSD
# include "manglerosd.h"
#endif

#define PTT_KEY_GET   "<span weight='light'>&lt;press the set button to define a hotkey&gt;</span>"
#define PTT_KEY_SET   "<span color='red'>&lt;Hold a key combination and click done...&gt;</span>"
#define PTT_MOUSE_GET "<span weight='light'>&lt;press the set button to define a button&gt;</span>"
#define PTT_MOUSE_SET "<span color='red'>&lt;Click a mouse button you wish to use...&gt;</span>"

ManglerSettings::ManglerSettings(Glib::RefPtr<Gtk::Builder> builder) {/*{{{*/
    this->builder = builder;

    // Connect our signals for this window
    builder->get_widget("settingsWindow", settingsWindow);
    /*
     * Set window properties that are not settable in builder
     */
    settingsWindow->set_keep_above(true);

    settingsWindow->signal_show().connect(sigc::mem_fun(this, &ManglerSettings::settingsWindow_show_cb));
    settingsWindow->signal_hide().connect(sigc::mem_fun(this, &ManglerSettings::settingsWindow_hide_cb));

    builder->get_widget("settingsCancelButton", button);
    button->signal_clicked().connect(sigc::mem_fun(this, &ManglerSettings::settingsCancelButton_clicked_cb));

    builder->get_widget("settingsApplyButton", button);
    button->signal_clicked().connect(sigc::mem_fun(this, &ManglerSettings::settingsApplyButton_clicked_cb));

    builder->get_widget("settingsOkButton", button);
    button->signal_clicked().connect(sigc::mem_fun(this, &ManglerSettings::settingsOkButton_clicked_cb));

    builder->get_widget("settingsEnablePTTKeyCheckButton", checkbutton);
    checkbutton->signal_toggled().connect(sigc::mem_fun(this, &ManglerSettings::settingsEnablePTTKeyCheckButton_toggled_cb));

    builder->get_widget("settingsPTTKeyButton", button);
    button->signal_clicked().connect(sigc::mem_fun(this, &ManglerSettings::settingsPTTKeyButton_clicked_cb));

    builder->get_widget("settingsEnablePTTMouseCheckButton", checkbutton);
    checkbutton->signal_toggled().connect(sigc::mem_fun(this, &ManglerSettings::settingsEnablePTTMouseCheckButton_toggled_cb));

    builder->get_widget("settingsPTTMouseButton", button);
    button->signal_clicked().connect(sigc::mem_fun(this, &ManglerSettings::settingsPTTMouseButton_clicked_cb));

    builder->get_widget("settingsEnableAudioIntegrationCheckButton", checkbutton);
    checkbutton->signal_toggled().connect(sigc::mem_fun(this, &ManglerSettings::settingsEnableAudioIntegrationCheckButton_toggled_cb));
    builder->get_widget("settingsAudioIntegrationComboBox", audioPlayerComboBox);
    audioPlayerTreeModel = Gtk::ListStore::create(audioPlayerColumns);
    audioPlayerComboBox->set_model(audioPlayerTreeModel);
    // create a "none" row
    Gtk::TreeModel::Row audioPlayerNoneRow = *(audioPlayerTreeModel->append());
    audioPlayerNoneRow[audioPlayerColumns.id] = MusicClient_None;
    audioPlayerNoneRow[audioPlayerColumns.name] = "None";
#ifdef HAVE_LIBMPDCLIENT
    // add MPD row
    Gtk::TreeModel::Row audioPlayerMPDRow = *(audioPlayerTreeModel->append());
    audioPlayerMPDRow[audioPlayerColumns.id] = MusicClient_MPD;
    audioPlayerMPDRow[audioPlayerColumns.name] = "MPD";
#endif
#ifdef HAVE_DBUS
    // add DBUS client rows
    // rhythmbox
    Gtk::TreeModel::Row audioPlayerRBRow = *(audioPlayerTreeModel->append());
    audioPlayerRBRow[audioPlayerColumns.id] = MusicClient_Rhythmbox;
    audioPlayerRBRow[audioPlayerColumns.name] = "Rhythmbox";
    // amarok
    Gtk::TreeModel::Row audioPlayerAmarokRow = *(audioPlayerTreeModel->append());
    audioPlayerAmarokRow[audioPlayerColumns.id] = MusicClient_Amarok;
    audioPlayerAmarokRow[audioPlayerColumns.name] = "Amarok";
#endif
    audioPlayerComboBox->pack_start(audioPlayerColumns.name);
    audioPlayerComboBox->set_active(audioPlayerNoneRow);

    builder->get_widget("settingsEnableVoiceActivationCheckButton", checkbutton);
    checkbutton->signal_toggled().connect(sigc::mem_fun(this, &ManglerSettings::settingsEnableVoiceActivationCheckButton_toggled_cb));

#ifdef HAVE_XOSD
    builder->get_widget("settingsOSD", vbox);
    vbox->set_sensitive(true);
    builder->get_widget("settingsEnableOnScreenDisplayCheckButton", checkbutton);
    checkbutton->signal_toggled().connect(sigc::mem_fun(this, &ManglerSettings::settingsEnableOnScreenDisplayCheckButton_toggled_cb));
    builder->get_widget("settingsOSDverticalPos", osdPosition);
    osdPositionModel = Gtk::ListStore::create(osdPositionColumns);
    osdPosition->set_model(osdPositionModel);
    osdPosition->pack_start(osdPositionColumns.name);
    Gtk::TreeModel::Row posrow;
    posrow = *osdPositionModel->append();
    posrow[osdPositionColumns.id] = XOSD_top;    posrow[osdPositionColumns.name] = "Top";
    posrow = *osdPositionModel->append();
    posrow[osdPositionColumns.id] = XOSD_middle; posrow[osdPositionColumns.name] = "Middle";
    posrow = *osdPositionModel->append();
    posrow[osdPositionColumns.id] = XOSD_bottom; posrow[osdPositionColumns.name] = "Bottom";

    builder->get_widget("settingsOSDhorizontalPos", osdAlignment);
    osdAlignmentModel = Gtk::ListStore::create(osdAlignmentColumns);
    osdAlignment->set_model(osdAlignmentModel);
    osdAlignment->pack_start(osdAlignmentColumns.name);
    Gtk::TreeModel::Row alnrow;
    alnrow = *osdAlignmentModel->append();
    alnrow[osdAlignmentColumns.id] = XOSD_center; alnrow[osdAlignmentColumns.name] = "Center";
    alnrow = *osdAlignmentModel->append();
    alnrow[osdAlignmentColumns.id] = XOSD_left;   alnrow[osdAlignmentColumns.name] = "Left";
    alnrow = *osdAlignmentModel->append();
    alnrow[osdAlignmentColumns.id] = XOSD_right;  alnrow[osdAlignmentColumns.name] = "Right";

    builder->get_widget("settingsOSDfontsize", osdFontSize);
    builder->get_widget("settingsOSDcolor", osdColor);
#endif

    builder->get_widget("audioSubsystemComboBox", audioSubsystemComboBox);
    audioSubsystemTreeModel = Gtk::ListStore::create(audioSubsystemColumns);
    audioSubsystemComboBox->set_model(audioSubsystemTreeModel);
    audioSubsystemComboBox->pack_start(audioSubsystemColumns.name);
    audioSubsystemComboBox->signal_changed().connect(sigc::mem_fun(this, &ManglerSettings::audioSubsystemComboBox_changed_cb));

    builder->get_widget("inputDeviceComboBox", inputDeviceComboBox);
    inputDeviceTreeModel = Gtk::ListStore::create(inputColumns);
    inputDeviceComboBox->set_model(inputDeviceTreeModel);
    inputDeviceComboBox->pack_start(inputColumns.description);
    inputDeviceComboBox->signal_changed().connect(sigc::mem_fun(this, &ManglerSettings::inputDeviceComboBox_changed_cb));

    builder->get_widget("inputDeviceCustomName", inputDeviceCustomName);

    builder->get_widget("outputDeviceComboBox", outputDeviceComboBox);
    outputDeviceTreeModel = Gtk::ListStore::create(outputColumns);
    outputDeviceComboBox->set_model(outputDeviceTreeModel);
    outputDeviceComboBox->pack_start(outputColumns.description);
    outputDeviceComboBox->signal_changed().connect(sigc::mem_fun(this, &ManglerSettings::outputDeviceComboBox_changed_cb));

    builder->get_widget("outputDeviceCustomName", outputDeviceCustomName);

    builder->get_widget("notificationDeviceComboBox", notificationDeviceComboBox);
    notificationDeviceTreeModel = Gtk::ListStore::create(notificationColumns);
    notificationDeviceComboBox->set_model(notificationDeviceTreeModel);
    notificationDeviceComboBox->pack_start(notificationColumns.description);
    notificationDeviceComboBox->signal_changed().connect(sigc::mem_fun(this, &ManglerSettings::notificationDeviceComboBox_changed_cb));

    builder->get_widget("notificationDeviceCustomName", notificationDeviceCustomName);

    mouseInputDevices = getInputDeviceList();
    builder->get_widget("settingsMouseDeviceComboBox", mouseDeviceComboBox);
    mouseDeviceTreeModel = Gtk::ListStore::create(mouseColumns);
    mouseDeviceComboBox->set_model(mouseDeviceTreeModel);
    mouseDeviceComboBox->pack_start(mouseColumns.name);

    // Audio Subsystem
    audioSubsystemTreeModel->clear();
    Gtk::TreeModel::Row audioSubsystemRow;
#ifdef HAVE_PULSE
    audioSubsystemRow = *(audioSubsystemTreeModel->append());
    audioSubsystemRow[audioSubsystemColumns.id] = "pulse";
    audioSubsystemRow[audioSubsystemColumns.name] = "PulseAudio";
#endif
#ifdef HAVE_ALSA
    audioSubsystemRow = *(audioSubsystemTreeModel->append());
    audioSubsystemRow[audioSubsystemColumns.id] = "alsa";
    audioSubsystemRow[audioSubsystemColumns.name] = "ALSA";
#endif
#ifdef HAVE_OSS
    audioSubsystemRow = *(audioSubsystemTreeModel->append());
    audioSubsystemRow[audioSubsystemColumns.id] = "oss";
    audioSubsystemRow[audioSubsystemColumns.name] = "OSS";
#endif

    // Master Volume
    volumeAdjustment = new Gtk::Adjustment(79, 0, 158, 1, 10, 10);
    volumehscale = new Gtk::HScale(*volumeAdjustment);
    volumehscale->add_mark(148, Gtk::POS_LEFT, "200%");
    volumehscale->add_mark(79, Gtk::POS_LEFT, "100%");
    volumehscale->add_mark(0, Gtk::POS_LEFT, "0%");
    volumehscale->set_inverted(false);
    volumehscale->set_draw_value(false);
    builder->get_widget("masterVolumeVbox", vbox);
    vbox->pack_start(*volumehscale);
    volumehscale->show();

    // Input Gain
    gainAdjustment = new Gtk::Adjustment(79, 0, 158, 1, 10, 10);
    gainhscale = new Gtk::HScale(*gainAdjustment);
    gainhscale->add_mark(148, Gtk::POS_LEFT, "200%");
    gainhscale->add_mark(79, Gtk::POS_LEFT, "100%");
    gainhscale->add_mark(0, Gtk::POS_LEFT, "0%");
    gainhscale->set_inverted(false);
    gainhscale->set_draw_value(false);
    builder->get_widget("inputGainVbox", vbox);
    vbox->pack_start(*gainhscale);
    gainhscale->show();
}/*}}}*/
void ManglerSettings::applySettings(void) {/*{{{*/
    Gtk::TreeModel::iterator iter;
    GdkWindow *rootwin = gdk_get_default_root_window();

    // Key Push to Talk
    builder->get_widget("settingsEnablePTTKeyCheckButton", checkbutton);
    Mangler::config["PushToTalkKeyEnabled"] = checkbutton->get_active();
    builder->get_widget("settingsPTTKeyValueLabel", label);
    if (label->get_label() != PTT_KEY_GET && label->get_label() != PTT_KEY_SET) {
        Mangler::config["PushToTalkKeyValue"] = label->get_text();
    } else {
        Mangler::config["PushToTalkKeyValue"] = "";
    }
    //Mangler::config.parsePushToTalkValue(config.PushToTalkKeyValue);

    // Mouse Push to Talk
    builder->get_widget("settingsMouseDeviceComboBox", combobox);
    iter = combobox->get_active();
    if (iter) {
        Gtk::TreeModel::Row row = *iter;
        Mangler::config["MouseDeviceName"] = Glib::ustring( row[mouseColumns.name] );
    }
    builder->get_widget("settingsEnablePTTMouseCheckButton", checkbutton);
    Mangler::config["PushToTalkMouseEnabled"] = checkbutton->get_active();
    builder->get_widget("settingsPTTMouseValueLabel", label);
    if (label->get_label() != PTT_MOUSE_GET && label->get_label() != PTT_MOUSE_SET) {
        Glib::ustring PushToTalkMouseValue = label->get_text();
        if (PushToTalkMouseValue.length() > 6) {
            Mangler::config["PushToTalkMouseValue"] = PushToTalkMouseValue.substr(6);
        } else {
            Mangler::config["PushToTalkMouseValue"] = PushToTalkMouseValue;
        }
    } else {
        Mangler::config["PushToTalkMouseValue"] = "";
    }
    XUngrabButton(GDK_WINDOW_XDISPLAY(rootwin), AnyButton, AnyModifier, GDK_ROOT_WINDOW());
    XAllowEvents (GDK_WINDOW_XDISPLAY(rootwin), AsyncBoth, CurrentTime);
    /*
    if (checkbutton->get_active()) {
        XGrabButton(GDK_WINDOW_XDISPLAY(rootwin), config.PushToTalkMouseValueInt, AnyModifier, GDK_ROOT_WINDOW(), False, ButtonPressMask|ButtonReleaseMask, GrabModeAsync, GrabModeAsync, None, None);
    }
    */

    // Audio Player Integration
    builder->get_widget("settingsEnableAudioIntegrationCheckButton", checkbutton);
    Mangler::config["AudioIntegrationEnabled"] = checkbutton->get_active();
    iter = audioPlayerComboBox->get_active();
    if (iter) {
        Gtk::TreeModel::Row row = *iter;
        uint8_t id = row[audioPlayerColumns.id];
        Mangler::config["AudioIntegrationPlayer"] = id;
        if (Mangler::config["AudioIntegrationEnabled"].toBool()) {
            mangler->integration->setClient((MusicClient)id);
        } else {
            mangler->integration->setClient(MusicClient_None);
        }
    }
    mangler->integration->update(true);

    // Voice Activation
    builder->get_widget("settingsEnableVoiceActivationCheckButton", checkbutton);
    Mangler::config["VoiceActivationEnabled"] = checkbutton->get_active();
    builder->get_widget("settingsVoiceActivationSilenceDurationSpinButton", spinbutton);
    Mangler::config["VoiceActivationSilenceDuration"] = spinbutton->get_value() * 1000.0;
    builder->get_widget("settingsVoiceActivationSensitivitySpinButton", spinbutton);
    Mangler::config["VoiceActivationSensitivity"] = spinbutton->get_value_as_int();

#ifdef HAVE_XOSD
    // On-Screen Display
    builder->get_widget("settingsEnableOnScreenDisplayCheckButton", checkbutton);
    Mangler::config["OnScreenDisplayEnabled"] = checkbutton->get_active();
    if (checkbutton->get_active()) {
        Gtk::TreeModel::iterator pos_iter = osdPosition->get_active();
        if (pos_iter) {
            int vert_pos_int = (*pos_iter)[osdPositionColumns.id];
            Mangler::config["OnScreenDisplayVerticalPosition"] = vert_pos_int;
        }
        Gtk::TreeModel::iterator aln_iter = osdAlignment->get_active();
        if (aln_iter) {
            int horz_aln_int = (*aln_iter)[osdAlignmentColumns.id];
            Mangler::config["OnScreenDisplayHorizontalAlignment"] = horz_aln_int;
        }
        Mangler::config["OnScreenDisplayFontSize"] = osdFontSize->get_value();
        Gdk::Color color = osdColor->get_color();
        char colorstr[16];
        snprintf(colorstr, 15, "#%02x%02x%02x", color.get_red() / 256, color.get_green() / 256, color.get_blue() / 256);
        Mangler::config["OnScreenDisplayColor"] = colorstr;
        mangler->osd->destroyOsd();
    }
#endif

    // Audio Devices
    iter = inputDeviceComboBox->get_active();
    if (iter) {
        Gtk::TreeModel::Row row = *iter;
        Mangler::config["InputDeviceName"] = Glib::ustring( row[inputColumns.name] );
    }
    Mangler::config["InputDeviceCustomName"] = inputDeviceCustomName->get_text();
    iter = outputDeviceComboBox->get_active();
    if (iter) {
        Gtk::TreeModel::Row row = *iter;
        Mangler::config["OutputDeviceName"] = Glib::ustring( row[outputColumns.name] );
    }
    Mangler::config["OutputDeviceCustomName"] = outputDeviceCustomName->get_text();
    iter = notificationDeviceComboBox->get_active();
    if (iter) {
        Gtk::TreeModel::Row row = *iter;
        Mangler::config["NotificationDeviceName"] = Glib::ustring( row[notificationColumns.name] );
    }
    Mangler::config["NotificationDeviceCustomName"] = notificationDeviceCustomName->get_text();
    iter = audioSubsystemComboBox->get_active();
    if (iter) {
        Gtk::TreeModel::Row row = *iter;
        Mangler::config["AudioSubsystem"] = Glib::ustring( row[audioSubsystemColumns.id] );
    }

    // Master Volume
    Mangler::config["MasterVolumeLevel"] = volumeAdjustment->get_value();
    v3_set_volume_master(Mangler::config["MasterVolumeLevel"].toInt());
    mangler->setTooltip();

    // Input Gain
    Mangler::config["InputGainLevel"] = gainAdjustment->get_value();
    v3_set_volume_xmit(Mangler::config["InputGainLevel"].toInt());

    // Notification Sounds
    builder->get_widget("notificationLoginLogoutCheckButton", checkbutton);
    Mangler::config["NotificationLoginLogout"] = checkbutton->get_active();

    builder->get_widget("notificationChannelEnterLeaveCheckButton", checkbutton);
    Mangler::config["NotificationChannelEnterLeave"] = checkbutton->get_active();

    builder->get_widget("notificationTalkStartEndCheckButton", checkbutton);
    Mangler::config["NotificationTransmitStartStop"] = checkbutton->get_active();

    builder->get_widget("notificationTTSCheckButton", checkbutton);
    Mangler::config["NotificationTextToSpeech"] = checkbutton->get_active();

    // Debug Level
    uint32_t debuglevel = 0;
    builder->get_widget("debugStatus", checkbutton);
    debuglevel |= checkbutton->get_active() ? V3_DEBUG_STATUS : 0;

    builder->get_widget("debugError", checkbutton);
    debuglevel |= checkbutton->get_active() ? V3_DEBUG_ERROR : 0;

    builder->get_widget("debugStack", checkbutton);
    debuglevel |= checkbutton->get_active() ? V3_DEBUG_STACK : 0;

    builder->get_widget("debugInternalNet", checkbutton);
    debuglevel |= checkbutton->get_active() ? V3_DEBUG_INTERNAL : 0;

    builder->get_widget("debugPacketDump", checkbutton);
    debuglevel |= checkbutton->get_active() ? V3_DEBUG_PACKET : 0;

    builder->get_widget("debugPacketParse", checkbutton);
    debuglevel |= checkbutton->get_active() ? V3_DEBUG_PACKET_PARSE : 0;

    builder->get_widget("debugEventQueue", checkbutton);
    debuglevel |= checkbutton->get_active() ? V3_DEBUG_EVENT : 0;

    builder->get_widget("debugSocket", checkbutton);
    debuglevel |= checkbutton->get_active() ? V3_DEBUG_SOCKET : 0;

    builder->get_widget("debugNotice", checkbutton);
    debuglevel |= checkbutton->get_active() ? V3_DEBUG_NOTICE : 0;

    builder->get_widget("debugInfo", checkbutton);
    debuglevel |= checkbutton->get_active() ? V3_DEBUG_INFO : 0;

    builder->get_widget("debugMutex", checkbutton);
    debuglevel |= checkbutton->get_active() ? V3_DEBUG_MUTEX : 0;

    builder->get_widget("debugMemory", checkbutton);
    debuglevel |= checkbutton->get_active() ? V3_DEBUG_MEMORY : 0;

    builder->get_widget("debugEncryptedPacket", checkbutton);
    debuglevel |= checkbutton->get_active() ? V3_DEBUG_PACKET_ENCRYPTED : 0;

    Mangler::config["lv3_debuglevel"] = debuglevel;

    v3_debuglevel(debuglevel);
    Mangler::config.config.save();
}/*}}}*/
void ManglerSettings::initSettings(void) {/*{{{*/
    // Key Push to Talk
    builder->get_widget("settingsEnablePTTKeyCheckButton", checkbutton);
    checkbutton->set_active(Mangler::config["PushToTalkKeyEnabled"].toBool());
    builder->get_widget("settingsPTTKeyValueLabel", label);
    if (Mangler::config["PushToTalkKeyValue"].length()) {
        label->set_text(Mangler::config["PushToTalkKeyValue"].toUString());
    } else {
        label->set_markup(PTT_KEY_GET);
    }

    // Mouse Push to Talk
    builder->get_widget("settingsEnablePTTMouseCheckButton", checkbutton);
    checkbutton->set_active(Mangler::config["PushToTalkMouseEnabled"].toBool());
    builder->get_widget("settingsPTTMouseValueLabel", label);
    if (Mangler::config["PushToTalkMouseValue"].length()) {
        label->set_text("Button" + Mangler::config["PushToTalkMouseValue"].toUString());
    } else {
        label->set_markup(PTT_MOUSE_GET);
    }

    // Audio Player Integration
    builder->get_widget("settingsEnableAudioIntegrationCheckButton", checkbutton);
    checkbutton->set_active(Mangler::config["AudioIntegrationEnabled"].toBool());
    int audioPlayerSelection = 0;
    int audioPlayerCtr = 0;
    Gtk::TreeModel::Children apChildren = audioPlayerTreeModel->children();
    for (Gtk::TreeModel::Children::iterator apIter = apChildren.begin();
            apIter != apChildren.end();
            ++apIter, audioPlayerCtr++) {
        Gtk::TreeModel::Row row = *apIter;
        uint8_t id = row[audioPlayerColumns.id];
        if (Mangler::config["AudioIntegrationPlayer"].toUInt() == id) {
            audioPlayerSelection = audioPlayerCtr;
        }
    }
    audioPlayerComboBox->set_active(audioPlayerSelection);
    /*
       iterate through whatever is available based on what we can find and populate the store
       audioPlayerComboBox->set_active(iterOfSelectedinStore);
    */

    // Voice Activation
    builder->get_widget("settingsEnableVoiceActivationCheckButton", checkbutton);
    checkbutton->set_active(Mangler::config["VoiceActivationEnabled"].toBool());
    builder->get_widget("settingsVoiceActivationSilenceDurationSpinButton", spinbutton);
    spinbutton->set_value(Mangler::config["VoiceActivationSilenceDuration"].toDouble() / 1000.0);
    builder->get_widget("settingsVoiceActivationSensitivitySpinButton", spinbutton);
    spinbutton->set_value(Mangler::config["VoiceActivationSensitivity"].toUInt());

#ifdef HAVE_XOSD
    // On-Screen Display
    builder->get_widget("settingsEnableOnScreenDisplayCheckButton", checkbutton);
    checkbutton->set_active(Mangler::config["OnScreenDisplayEnabled"].toBool());
    Gtk::TreeModel::iterator hz_iter = osdAlignmentModel->children().begin();
    while (hz_iter != osdAlignmentModel->children().end()) {
        int hzint = (*hz_iter)[osdAlignmentColumns.id];
        if (Mangler::config["OnScreenDisplayHorizontalAlignment"].toInt() == hzint ||
            (!Mangler::config["OnScreenDisplayHorizontalAlignment"].length() && hzint == XOSD_center)) {
            osdAlignment->set_active(hz_iter);
            break;
        }
        hz_iter++;
    }
    Gtk::TreeModel::iterator vt_iter = osdPositionModel->children().begin();
    while (vt_iter != osdPositionModel->children().end()) {
        int vtint = (*vt_iter)[osdPositionColumns.id];
        if (Mangler::config["OnScreenDisplayVerticalPosition"].toInt() == vtint ||
            (!Mangler::config["OnScreenDisplayVerticalPosition"].length() && vtint == XOSD_top)) {
            osdPosition->set_active(vt_iter);
            break;
        }
        vt_iter++;
    }
    if (Mangler::config["OnScreenDisplayFontSize"].length()) {
        osdFontSize->set_value(Mangler::config["OnScreenDisplayFontSize"].toDouble());
    } else {
        osdFontSize->set_value(8.0);
    }
    if (Mangler::config["OnScreenDisplayColor"].length()) {
        Gdk::Color color;
        color.set(Mangler::config["OnScreenDisplayColor"].toUString());
        osdColor->set_color(color);
    }
#endif

    // Audio Subsystem
    for (Gtk::TreeModel::iterator asIter = audioSubsystemTreeModel->children().begin();
            asIter != audioSubsystemTreeModel->children().end();
            asIter++) {
        if ((*asIter)[audioSubsystemColumns.id] == Mangler::config["AudioSubsystem"].toLower()) {
            audioSubsystemComboBox->set_active(asIter);
        }
    }

    // Audio Devices
    inputDeviceCustomName->set_text(Mangler::config["InputDeviceCustomName"].toUString());
    outputDeviceCustomName->set_text(Mangler::config["OutputDeviceCustomName"].toUString());
    notificationDeviceCustomName->set_text(Mangler::config["NotificationDeviceCustomName"].toUString());

    // Notification Sounds
    builder->get_widget("notificationLoginLogoutCheckButton", checkbutton);
    checkbutton->set_active(Mangler::config["NotificationLoginLogout"].toBool());

    builder->get_widget("notificationChannelEnterLeaveCheckButton", checkbutton);
    checkbutton->set_active(Mangler::config["NotificationChannelEnterLeave"].toBool());

    builder->get_widget("notificationTalkStartEndCheckButton", checkbutton);
    checkbutton->set_active(Mangler::config["NotificationTransmitStartStop"].toBool());

    builder->get_widget("notificationTTSCheckButton", checkbutton);
    checkbutton->set_active(Mangler::config["NotificationTextToSpeech"].toBool());
#ifdef HAVE_ESPEAK
    checkbutton->set_sensitive(true);
#endif

    // Master Volume
    if (Mangler::config["MasterVolumeLevel"].length()) {
        volumeAdjustment->set_value(Mangler::config["MasterVolumeLevel"].toInt());
    }

    // Input Gain
    if (Mangler::config["InputGainLevel"].length()) {
        gainAdjustment->set_value(Mangler::config["InputGainLevel"].toInt());
    }

    // Debug Level
    builder->get_widget("debugStatus", checkbutton);
    uint32_t config_lv3_debuglevel = Mangler::config["lv3_debuglevel"].toULong();
    checkbutton->set_active(config_lv3_debuglevel & V3_DEBUG_STATUS ? 1 : 0);

    builder->get_widget("debugError", checkbutton);
    checkbutton->set_active(config_lv3_debuglevel & V3_DEBUG_ERROR ? 1 : 0);

    builder->get_widget("debugStack", checkbutton);
    checkbutton->set_active(config_lv3_debuglevel & V3_DEBUG_STACK ? 1 : 0);

    builder->get_widget("debugInternalNet", checkbutton);
    checkbutton->set_active(config_lv3_debuglevel & V3_DEBUG_INTERNAL ? 1 : 0);

    builder->get_widget("debugPacketDump", checkbutton);
    checkbutton->set_active(config_lv3_debuglevel & V3_DEBUG_PACKET ? 1 : 0);

    builder->get_widget("debugPacketParse", checkbutton);
    checkbutton->set_active(config_lv3_debuglevel & V3_DEBUG_PACKET_PARSE ? 1 : 0);

    builder->get_widget("debugEventQueue", checkbutton);
    checkbutton->set_active(config_lv3_debuglevel & V3_DEBUG_EVENT ? 1 : 0);

    builder->get_widget("debugSocket", checkbutton);
    checkbutton->set_active(config_lv3_debuglevel & V3_DEBUG_SOCKET ? 1 : 0);

    builder->get_widget("debugNotice", checkbutton);
    checkbutton->set_active(config_lv3_debuglevel & (uint32_t)V3_DEBUG_NOTICE ? 1 : 0);

    builder->get_widget("debugInfo", checkbutton);
    checkbutton->set_active(config_lv3_debuglevel & V3_DEBUG_INFO ? 1 : 0);

    builder->get_widget("debugMutex", checkbutton);
    checkbutton->set_active(config_lv3_debuglevel & V3_DEBUG_MUTEX ? 1 : 0);

    builder->get_widget("debugMemory", checkbutton);
    checkbutton->set_active(config_lv3_debuglevel & V3_DEBUG_MEMORY ? 1 : 0);

    builder->get_widget("debugEncryptedPacket", checkbutton);
    checkbutton->set_active(config_lv3_debuglevel & V3_DEBUG_PACKET_ENCRYPTED ? 1 : 0);
}/*}}}*/

// Settings Window Callbacks
void ManglerSettings::showSettingsWindow(void) {/*{{{*/
    settingsWindow->show();
}/*}}}*/

void ManglerSettings::settingsWindow_show_cb(void) {/*{{{*/
    Gtk::TreeModel::Row row;
    isDetectingKey = false;
    isDetectingMouse = false;

    initSettings();
    // these callbacks initialize the state
    settingsEnablePTTKeyCheckButton_toggled_cb();
    settingsEnablePTTMouseCheckButton_toggled_cb();

    updateDeviceComboBoxes();

    // Clear the mouse device store and rebuild it from the audioControl vector
    mouseDeviceTreeModel->clear();
    int mouseSelection = 0;
    int mouseCtr = 0;
    for (
            std::map<uint32_t, Glib::ustring>::iterator i = mouseInputDevices.begin();
            i != mouseInputDevices.end();
            i++, mouseCtr++) {
        Gtk::TreeModel::Row row = *(mouseDeviceTreeModel->append());
        row[mouseColumns.id] = (*i).first;
        row[mouseColumns.name] = (*i).second;
        if (Mangler::config["MouseDeviceName"] == (*i).second) {
            mouseSelection = mouseCtr;
        }
    }
    if (!mouseSelection) {
        int mouseCtr = 0;
        for (
                std::map<uint32_t, Glib::ustring>::iterator i = mouseInputDevices.begin();
                i != mouseInputDevices.end();
                i++, mouseCtr++) {
            if ((*i).second.find("Mouse") != std::string::npos) {
                mouseSelection = mouseCtr;
            }
        }
    }
    // TODO: get the currently selected item from settings object and select it
    mouseDeviceComboBox->set_active(mouseSelection);
}/*}}}*/
void ManglerSettings::settingsWindow_hide_cb(void) {/*{{{*/
    isDetectingKey = false;
    isDetectingMouse = false;
}/*}}}*/
void ManglerSettings::settingsCancelButton_clicked_cb(void) {/*{{{*/
    // additional cleanup should happen in ManglerSettings::settingsWindow_hide_cb()
    settingsWindow->hide();
}/*}}}*/
void ManglerSettings::settingsApplyButton_clicked_cb(void) {/*{{{*/
    applySettings();
}/*}}}*/
void ManglerSettings::settingsOkButton_clicked_cb(void) {/*{{{*/
    applySettings();
    settingsWindow->hide();
}/*}}}*/
void ManglerSettings::settingsEnablePTTKeyCheckButton_toggled_cb(void) {/*{{{*/
    builder->get_widget("settingsEnablePTTKeyCheckButton", checkbutton);
    if (checkbutton->get_active()) {
        // box was checked
        builder->get_widget("settingsPTTKeyLabel", label);
        label->set_sensitive(true);
        builder->get_widget("settingsPTTKeyValueLabel", label);
        label->set_sensitive(true);
        builder->get_widget("settingsPTTKeyButton", button);
        button->set_sensitive(true);
    } else {
        // box was unchecked
        builder->get_widget("settingsPTTKeyLabel", label);
        label->set_sensitive(false);
        builder->get_widget("settingsPTTKeyValueLabel", label);
        label->set_sensitive(false);
        builder->get_widget("settingsPTTKeyButton", button);
        button->set_sensitive(false);
    }
}/*}}}*/
void ManglerSettings::settingsEnableAudioIntegrationCheckButton_toggled_cb(void) {/*{{{*/
    builder->get_widget("settingsEnableAudioIntegrationCheckButton", checkbutton);
    if (checkbutton->get_active()) {
        builder->get_widget("settingsAudioIntegrationLabel", label);
        label->set_sensitive(true);
        audioPlayerComboBox->set_sensitive(true);
    } else {
        builder->get_widget("settingsAudioIntegrationLabel", label);
        label->set_sensitive(false);
        audioPlayerComboBox->set_sensitive(false);
    }
}/*}}}*/
void ManglerSettings::settingsEnableVoiceActivationCheckButton_toggled_cb(void) {/*{{{*/
    builder->get_widget("settingsEnableVoiceActivationCheckButton", checkbutton);
    if (checkbutton->get_active()) {
        builder->get_widget("settingsVoiceActivationTable", table);
        table->set_sensitive(true);
    } else {
        builder->get_widget("settingsVoiceActivationTable", table);
        table->set_sensitive(false);
    }
}/*}}}*/
void ManglerSettings::settingsEnableOnScreenDisplayCheckButton_toggled_cb(void) {/*{{{*/
    builder->get_widget("settingsEnableOnScreenDisplayCheckButton", checkbutton);
    builder->get_widget("settingsOSDTable", table);
    table->set_sensitive(checkbutton->get_active());
}/*}}}*/
/*
 * When this button is pressed, we need to disable all of the other items on
 * the page until the user clicks the button again.  This starts a timer to
 * check keyboard state and update the settingsPTTKeyLabel to the key
 * combination value.  Setting isDetectingKey to false will cause the timer
 * callback to stop running.
 * The timer callback is ManglerSettings::settingsPTTKeyDetect.
 */
void ManglerSettings::settingsPTTKeyButton_clicked_cb(void) {/*{{{*/
    builder->get_widget("settingsPTTKeyButton", button);
    if (button->get_label() == "Set") {
        isDetectingKey = true;
        button->set_label("Done");
        builder->get_widget("settingsCancelButton", button);
        button->set_sensitive(false);
        builder->get_widget("settingsApplyButton", button);
        button->set_sensitive(false);
        builder->get_widget("settingsOkButton", button);
        button->set_sensitive(false);
        Glib::signal_timeout().connect( sigc::mem_fun(*this, &ManglerSettings::settingsPTTKeyDetect), 100 );
    } else {
        isDetectingKey = false;
        button->set_label("Set");
        builder->get_widget("settingsCancelButton", button);
        button->set_sensitive(true);
        builder->get_widget("settingsApplyButton", button);
        button->set_sensitive(true);
        builder->get_widget("settingsOkButton", button);
        button->set_sensitive(true);
        builder->get_widget("settingsPTTKeyValueLabel", label);
        // if the text is as follows, the user pressed done without any keys
        // pressed down and reset it to the default text
        if (label->get_label() == PTT_KEY_SET) {
            label->set_markup(PTT_KEY_GET);
        }
    }
}/*}}}*/
void ManglerSettings::settingsEnablePTTMouseCheckButton_toggled_cb(void) {/*{{{*/
    builder->get_widget("settingsEnablePTTMouseCheckButton", checkbutton);
    if (checkbutton->get_active()) {
        // box was checked
        builder->get_widget("settingsPTTMouseLabel", label);
        label->set_sensitive(true);
        builder->get_widget("settingsPTTMouseValueLabel", label);
        label->set_sensitive(true);
        builder->get_widget("settingsPTTMouseButton", button);
        button->set_sensitive(true);
        builder->get_widget("settingsMouseDeviceLabel", label);
        label->set_sensitive(true);
        builder->get_widget("settingsMouseDeviceComboBox", combobox);
        combobox->set_sensitive(true);
    } else {
        // box was unchecked
        builder->get_widget("settingsPTTMouseLabel", label);
        label->set_sensitive(false);
        builder->get_widget("settingsPTTMouseValueLabel", label);
        label->set_sensitive(false);
        builder->get_widget("settingsPTTMouseButton", button);
        button->set_sensitive(false);
        builder->get_widget("settingsMouseDeviceLabel", label);
        label->set_sensitive(false);
        builder->get_widget("settingsMouseDeviceComboBox", combobox);
        combobox->set_sensitive(false);
    }
}/*}}}*/
void ManglerSettings::settingsPTTMouseButton_clicked_cb(void) {/*{{{*/
    isDetectingMouse = true;
    builder->get_widget("settingsPTTMouseValueLabel", label);
    label->set_markup(PTT_MOUSE_SET);
    builder->get_widget("settingsPTTMouseButton", button);
    button->set_sensitive(false);
    builder->get_widget("settingsCancelButton", button);
    button->set_sensitive(false);
    builder->get_widget("settingsApplyButton", button);
    button->set_sensitive(false);
    builder->get_widget("settingsOkButton", button);
    button->set_sensitive(false);
    builder->get_widget("settingsMouseDeviceComboBox", combobox);
    combobox->set_sensitive(false);
    Glib::signal_timeout().connect( sigc::mem_fun(*this, &ManglerSettings::settingsPTTMouseDetect), 100 );
}/*}}}*/
std::map<uint32_t, Glib::ustring> ManglerSettings::getInputDeviceList(void) {/*{{{*/
    GdkWindow   *rootwin = gdk_get_default_root_window();
    XDeviceInfo *xdev;
    int ndevices_return;
    std::map<uint32_t, Glib::ustring> devicelist;

    xdev = XListInputDevices(GDK_WINDOW_XDISPLAY(rootwin), &ndevices_return);
    for (int ctr = 0; ctr < ndevices_return; ctr++) {
        if (xdev[ctr].use == IsXExtensionPointer) {
            devicelist[xdev[ctr].id] = xdev[ctr].name;
        }
    }
    XFreeDeviceList(xdev);
    return(devicelist);
}/*}}}*/

void ManglerSettings::audioSubsystemComboBox_changed_cb(void) {/*{{{*/
    Gtk::TreeModel::iterator iter;

    iter = audioSubsystemComboBox->get_active();
    if (iter) {
        Gtk::TreeModel::Row row = *iter;
        Glib::ustring id = row[audioSubsystemColumns.id];
        Glib::ustring name = row[audioSubsystemColumns.name];
        if (mangler) {
            mangler->audioControl->getDeviceList(id);
            updateDeviceComboBoxes();
        }
    }
}/*}}}*/

bool
ManglerSettings::settingsPTTKeyDetect(void) {/*{{{*/
    char        pressed_keys[32];
    Glib::ustring ptt_keylist;
    GdkWindow   *rootwin = gdk_get_default_root_window();

    // TODO: window close event needs to set isDetectingKey
    if (!isDetectingKey) {
        return false;
    }
    /*
     * Query the X keymap and get a list of all the keys are pressed.  Convert
     * keycodes to keysyms to keynames and build a human readable string that
     * will be the actual value stored in the settings file
     */
    XQueryKeymap(GDK_WINDOW_XDISPLAY(rootwin), pressed_keys);
    for (int ctr = 0; ctr < 256; ctr++) {
        if ((pressed_keys[ctr >> 3] >> (ctr & 0x07)) & 0x01) {
            char *keystring = XKeysymToString(XKeycodeToKeysym(GDK_WINDOW_XDISPLAY(rootwin), ctr, 0));
            if (keystring == NULL)
                continue;
            std::string keyname = keystring;

            if (keyname.length() > 1) {
                ptt_keylist.insert(0, "<" + keyname + ">" + (ptt_keylist.empty() ? "" : "+"));
            } else {
                keyname[0] = toupper(keyname[0]);
                ptt_keylist.append((ptt_keylist.empty() ? "" : "+") + keyname);
            }
        }
    }
    builder->get_widget("settingsPTTKeyValueLabel", label);
    if (ptt_keylist.empty()) {
        label->set_markup(PTT_KEY_SET);
    } else {
        label->set_text(ptt_keylist);
    }
    return(true);
}/*}}}*/

bool
ManglerSettings::settingsPTTMouseDetect(void) {/*{{{*/
    GdkWindow *rootwin = gdk_get_default_root_window();
    Glib::ustring buttonname;
    char mousebutton[8];
    static bool grabbed = false;
    int flag = 0;
    //int x, y;
    XEvent ev;


    // TODO: window close event needs to set isDetectingKey
    if (!isDetectingMouse) {
        return false;
    }
    if (! grabbed) {
        XUngrabPointer(GDK_WINDOW_XDISPLAY(rootwin), CurrentTime);
        XGrabPointer(GDK_WINDOW_XDISPLAY(rootwin), GDK_ROOT_WINDOW(), False, ButtonPress, GrabModeAsync, GrabModeAsync, None, None, CurrentTime);
        grabbed = true;
    }
    while (flag == 0) {
        while (!XPending(GDK_WINDOW_XDISPLAY(rootwin))) {
            usleep(100000);
        }
        XNextEvent(GDK_WINDOW_XDISPLAY(rootwin), &ev);
        switch (ev.type) {
            case ButtonPress:
                snprintf(mousebutton, 7, "%d", ev.xbutton.button);
                buttonname = "Button" + Glib::ustring(mousebutton);
                flag = 1;
                XUngrabPointer(GDK_WINDOW_XDISPLAY(rootwin), CurrentTime);
                grabbed = false;
                isDetectingMouse = false;
                //x = ev.xbutton.x_root;
                //y = ev.xbutton.y_root;
                break;
            case MotionNotify:
                break;
            default:
                break;
        }
        XAllowEvents(GDK_WINDOW_XDISPLAY(rootwin), AsyncBoth, CurrentTime);
    }
    isDetectingMouse = false;
    builder->get_widget("settingsPTTMouseValueLabel", label);
    label->set_markup(buttonname);
    builder->get_widget("settingsCancelButton", button);
    button->set_sensitive(true);
    builder->get_widget("settingsApplyButton", button);
    button->set_sensitive(true);
    builder->get_widget("settingsOkButton", button);
    button->set_sensitive(true);
    builder->get_widget("settingsOkButton", button);
    button->set_sensitive(true);
    builder->get_widget("settingsPTTMouseButton", button);
    button->set_sensitive(true);
    builder->get_widget("settingsMouseDeviceComboBox", combobox);
    combobox->set_sensitive(true);

    return(true);
}/*}}}*/

void
ManglerSettings::updateDeviceComboBoxes(void) {/*{{{*/
    Gtk::TreeModel::Row row;
    Gtk::TreeModel::iterator iter;

    // Clear the input device store and rebuild it from the audioControl vector
    inputDeviceTreeModel->clear();
    row = *(inputDeviceTreeModel->append());
    row[inputColumns.id] = -1;
    row[inputColumns.name] = "Default";
    row[inputColumns.description] = "Default";
    int inputSelection = 0;
    int inputCtr = 1;
    for (
            std::vector<ManglerAudioDevice*>::iterator i = mangler->audioControl->inputDevices.begin();
            i <  mangler->audioControl->inputDevices.end();
            i++, inputCtr++) {
        Gtk::TreeModel::Row row = *(inputDeviceTreeModel->append());
        row[inputColumns.id] = (*i)->id;
        row[inputColumns.name] = (*i)->name;
        row[inputColumns.description] = (*i)->description;
        if (Mangler::config["InputDeviceName"] == (*i)->name) {
            inputSelection = inputCtr;
        }
    }
    iter = audioSubsystemComboBox->get_active();
    if (iter && (*iter)[audioSubsystemColumns.id] != "pulse") {
        row = *(inputDeviceTreeModel->append());
        row[inputColumns.id] = -2;
        row[inputColumns.name] = "Custom";
        row[inputColumns.description] = "Custom";
        if (Mangler::config["InputDeviceName"] == "Custom") {
            inputSelection = inputCtr;
        }
    }
    // TODO: get the currently selected item from settings object and select it
    inputDeviceComboBox->set_active(inputSelection);

    // Clear the output device store and rebuild it from the audioControl vector
    outputDeviceTreeModel->clear();
    row = *(outputDeviceTreeModel->append());
    row[outputColumns.id] = -1;
    row[outputColumns.name] = "Default";
    row[outputColumns.description] = "Default";
    int outputSelection = 0;
    int outputCtr = 1;
    for (
            std::vector<ManglerAudioDevice*>::iterator i = mangler->audioControl->outputDevices.begin();
            i <  mangler->audioControl->outputDevices.end();
            i++, outputCtr++) {
        Gtk::TreeModel::Row row = *(outputDeviceTreeModel->append());
        row[outputColumns.id] = (*i)->id;
        row[outputColumns.name] = (*i)->name;
        row[outputColumns.description] = (*i)->description;
        if (Mangler::config["OutputDeviceName"] == (*i)->name) {
            outputSelection = outputCtr;
        }
    }
    iter = audioSubsystemComboBox->get_active();
    if (iter && (*iter)[audioSubsystemColumns.id] != "pulse") {
        row = *(outputDeviceTreeModel->append());
        row[outputColumns.id] = -2;
        row[outputColumns.name] = "Custom";
        row[outputColumns.description] = "Custom";
        if (Mangler::config["OutputDeviceName"] == "Custom") {
            outputSelection = outputCtr;
        }
    }
    // TODO: get the currently selected item from settings object and select it
    outputDeviceComboBox->set_active(outputSelection);

    // Clear the notification device store and rebuild it from the audioControl vector
    notificationDeviceTreeModel->clear();
    row = *(notificationDeviceTreeModel->append());
    row[notificationColumns.id] = -1;
    row[notificationColumns.name] = "Default";
    row[notificationColumns.description] = "Default";
    int notificationSelection = 0;
    int notificationCtr = 1;
    for (
            std::vector<ManglerAudioDevice*>::iterator i = mangler->audioControl->outputDevices.begin();
            i <  mangler->audioControl->outputDevices.end();
            i++, notificationCtr++) {
        Gtk::TreeModel::Row row = *(notificationDeviceTreeModel->append());
        row[notificationColumns.id] = (*i)->id;
        row[notificationColumns.name] = (*i)->name;
        row[notificationColumns.description] = (*i)->description;
        if (Mangler::config["NotificationDeviceName"] == (*i)->name) {
            notificationSelection = notificationCtr;
        }
    }
    iter = audioSubsystemComboBox->get_active();
    if (iter && (*iter)[audioSubsystemColumns.id] != "pulse") {
        row = *(notificationDeviceTreeModel->append());
        row[notificationColumns.id] = -2;
        row[notificationColumns.name] = "Custom";
        row[notificationColumns.description] = "Custom";
        if (Mangler::config["NotificationDeviceName"] == "Custom") {
            notificationSelection = notificationCtr;
        }
    }
    // TODO: get the currently selected item from settings object and select it
    notificationDeviceComboBox->set_active(notificationSelection);
}/*}}}*/

void
ManglerSettings::inputDeviceComboBox_changed_cb(void) {/*{{{*/
    Gtk::TreeModel::iterator iter = inputDeviceComboBox->get_active();

    builder->get_widget("CustomInputLabel", label);
    if (iter && (*iter)[inputColumns.id] == -2) {
        inputDeviceCustomName->show();
        label->show();
        return;
    }
    inputDeviceCustomName->hide();
    label->hide();
}/*}}}*/

void
ManglerSettings::outputDeviceComboBox_changed_cb(void) {/*{{{*/
    Gtk::TreeModel::iterator iter = outputDeviceComboBox->get_active();

    builder->get_widget("CustomOutputLabel", label);
    if (iter && (*iter)[outputColumns.id] == -2) {
        outputDeviceCustomName->show();
        label->show();
        return;
    }
    outputDeviceCustomName->hide();
    label->hide();
}/*}}}*/

void
ManglerSettings::notificationDeviceComboBox_changed_cb(void) {/*{{{*/
    Gtk::TreeModel::iterator iter = notificationDeviceComboBox->get_active();

    builder->get_widget("CustomNotificationLabel", label);
    if (iter && (*iter)[notificationColumns.id] == -2) {
        notificationDeviceCustomName->show();
        label->show();
        return;
    }
    notificationDeviceCustomName->hide();
    label->hide();
}/*}}}*/

