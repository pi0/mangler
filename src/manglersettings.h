/*
 * vim: softtabstop=4 shiftwidth=4 cindent foldmethod=marker expandtab
 *
 * $LastChangedDate: 2011-06-27 23:50:10 +0430 (Mon, 27 Jun 2011) $
 * $Revision: 1139 $
 * $LastChangedBy: econnell $
 * $URL: http://svn.mangler.org/mangler/trunk/src/manglersettings.h $
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

#ifndef _MANGLERSETTINGS_H
#define _MANGLERSETTINGS_H

#include "manglerconfig.h"
#include <X11/extensions/XInput.h>

class ManglerSettings
{
    public:
        ManglerSettings(Glib::RefPtr<Gtk::Builder> builder);
        Glib::RefPtr<Gtk::Builder> builder;
        Gtk::Window         *settingsWindow;
        Gtk::Button         *button;
        Gtk::Label          *label;
        Gtk::ComboBox       *combobox;
        Gtk::Window         *window;
        Gtk::CheckButton    *checkbutton;
        Gtk::VBox           *vbox;
        Gtk::Table          *table;
        Gtk::SpinButton     *spinbutton;
        Gtk::HScale         *volumehscale;
        Gtk::Adjustment     *volumeAdjustment;
        Gtk::Adjustment     *gainAdjustment;
        Gtk::HScale         *gainhscale;
        sigc::connection    volumeAdjustSignalConnection;

        bool                isDetectingKey;
        bool                isDetectingMouse;
        std::map<uint32_t, Glib::ustring> mouseInputDevices;
        ManglerConfig       config;

        // Audio Player List Combo Box Setup
        Gtk::ComboBox       *audioPlayerComboBox;
        class audioPlayerModelColumns : public Gtk::TreeModel::ColumnRecord
        {
            public:
                audioPlayerModelColumns() { add(id); add(name); }
                Gtk::TreeModelColumn<uint32_t>      id;
                Gtk::TreeModelColumn<Glib::ustring> name;
        };
        audioPlayerModelColumns  audioPlayerColumns;
        Glib::RefPtr<Gtk::ListStore> audioPlayerTreeModel;

        Gtk::ComboBox       *audioSubsystemComboBox;
        class audioSubsystemModelColumns : public Gtk::TreeModel::ColumnRecord
        {
            public:
                audioSubsystemModelColumns() { add(id); add(name); }
                Gtk::TreeModelColumn<Glib::ustring> id;
                Gtk::TreeModelColumn<Glib::ustring> name;
        };
        audioSubsystemModelColumns  audioSubsystemColumns;
        Glib::RefPtr<Gtk::ListStore> audioSubsystemTreeModel;

        // Input Device Combo Box Setup
        Gtk::ComboBox       *inputDeviceComboBox;
        class inputDeviceModelColumns : public Gtk::TreeModel::ColumnRecord
        {
            public:
                inputDeviceModelColumns() { add(id); add(name); add(description); }
                Gtk::TreeModelColumn<int32_t>       id;
                Gtk::TreeModelColumn<Glib::ustring> name;
                Gtk::TreeModelColumn<Glib::ustring> description;
        };
        inputDeviceModelColumns  inputColumns;
        Glib::RefPtr<Gtk::ListStore> inputDeviceTreeModel;
        Gtk::Entry          *inputDeviceCustomName;
        // Output Device Combo Box Setup
        Gtk::ComboBox       *outputDeviceComboBox;
        class outputDeviceModelColumns : public Gtk::TreeModel::ColumnRecord
        {
            public:
                outputDeviceModelColumns() { add(id); add(name); add(description); }
                Gtk::TreeModelColumn<int32_t>       id;
                Gtk::TreeModelColumn<Glib::ustring> name;
                Gtk::TreeModelColumn<Glib::ustring> description;
        };
        outputDeviceModelColumns  outputColumns;
        Glib::RefPtr<Gtk::ListStore> outputDeviceTreeModel;
        Gtk::Entry          *outputDeviceCustomName;

        // Notification Device Combo Box Setup
        Gtk::ComboBox       *notificationDeviceComboBox;
        class notificationDeviceModelColumns : public Gtk::TreeModel::ColumnRecord
        {
            public:
                notificationDeviceModelColumns() { add(id); add(name); add(description); }
                Gtk::TreeModelColumn<int32_t>       id;
                Gtk::TreeModelColumn<Glib::ustring> name;
                Gtk::TreeModelColumn<Glib::ustring> description;
        };
        notificationDeviceModelColumns  notificationColumns;
        Glib::RefPtr<Gtk::ListStore> notificationDeviceTreeModel;
        Gtk::Entry          *notificationDeviceCustomName;

        // Mouse Input Device Combo Box Setup
        Gtk::ComboBox       *mouseDeviceComboBox;
        class mouseDeviceModelColumns : public Gtk::TreeModel::ColumnRecord
        {
            public:
                mouseDeviceModelColumns() { add(id); add(name); }
                Gtk::TreeModelColumn<uint32_t>      id;
                Gtk::TreeModelColumn<Glib::ustring> name;
        };
        mouseDeviceModelColumns  mouseColumns;
        Glib::RefPtr<Gtk::ListStore> mouseDeviceTreeModel;
        
        // on screen display position/alignment
        class osdColumns : public Gtk::TreeModel::ColumnRecord
        {
            public:
                osdColumns() { add(id); add(name); }
                Gtk::TreeModelColumn<int>           id;
                Gtk::TreeModelColumn<Glib::ustring> name;
        };
        osdColumns osdPositionColumns, osdAlignmentColumns;
        Glib::RefPtr<Gtk::ListStore> osdPositionModel, osdAlignmentModel;
        Gtk::ComboBox *osdPosition, *osdAlignment;
        Gtk::SpinButton *osdFontSize;
        Gtk::ColorButton *osdColor;
        

        // members functions
        void showSettingsWindow(void);
        bool settingsPTTKeyDetect(void);
        bool settingsPTTMouseDetect(void);
        void applySettings(void);
        void initSettings(void);

        std::map<uint32_t, Glib::ustring> getInputDeviceList(void);

        // callbacks
        void settingsWindow_show_cb(void);
        void settingsWindow_hide_cb(void);
        void settingsCancelButton_clicked_cb(void);
        void settingsApplyButton_clicked_cb(void);
        void settingsOkButton_clicked_cb(void);
        void settingsEnablePTTKeyCheckButton_toggled_cb(void);
        void settingsPTTKeyButton_clicked_cb(void);
        void settingsEnablePTTMouseCheckButton_toggled_cb(void);
        void settingsPTTMouseButton_clicked_cb(void);
        void settingsEnableAudioIntegrationCheckButton_toggled_cb(void);
        void settingsEnableVoiceActivationCheckButton_toggled_cb(void);
        void settingsEnableOnScreenDisplayCheckButton_toggled_cb(void);
        void audioSubsystemComboBox_changed_cb(void);
        void updateDeviceComboBoxes(void);
        void inputDeviceComboBox_changed_cb(void);
        void outputDeviceComboBox_changed_cb(void);
        void notificationDeviceComboBox_changed_cb(void);
};

#endif
