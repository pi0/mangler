/*
 * vim: softtabstop=4 shiftwidth=4 cindent foldmethod=marker expandtab
 *
 * $LastChangedDate: 2011-06-27 23:50:10 +0430 (Mon, 27 Jun 2011) $
 * $Revision: 1139 $
 * $LastChangedBy: econnell $
 * $URL: http://svn.mangler.org/mangler/trunk/src/manglerrecorder.h $
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

#ifndef _MANGLERRECORDER_H
#define _MANGLERRECORDER_H

class ManglerRecorder {
    public:
        ManglerRecorder(Glib::RefPtr<Gtk::Builder> builder);
        ~ManglerRecorder();
        Glib::RefPtr<Gtk::Builder> builder;
        Gtk::Window            *recWindow;
        Gtk::Dialog            *recInfoDialog;
        Gtk::ScrolledWindow    *recScrolledWindow;
        Gtk::MenuItem          *menuitem;
        Gtk::Button            *button;
        Gtk::Label             *label;
        Gtk::Entry             *entry;
        Gtk::Widget            *widget;
        Gtk::ToggleButton      *recordbutton;
        Gtk::Entry             *fileentry;
        Gtk::FileChooserDialog *filedialog;
        Gtk::TextView          *textview;

        bool isPlaying;
        bool isRecording;
        void show(void);
        void can_record(bool isConnected);
        void record(Glib::ustring username, Glib::ustring text, uint32_t index, uint32_t time, bool stopped, bool flushed);

    protected:
        Glib::ustring recdir;

        void *vrfh;
        Glib::ustring path;
        Glib::ustring filename;
        uint32_t totalduration;

        Glib::Thread *player;

        class recModelColumns : public Gtk::TreeModel::ColumnRecord {
            public:
                recModelColumns() {
                    add(id);
                    add(time_val);
                    add(duration_val);
                    add(diff_val);
                    add(time);
                    add(duration);
                    add(status);
                    add(username);
                    add(text);
                }
                Gtk::TreeModelColumn<uint32_t>      id;
                Gtk::TreeModelColumn<uint32_t>      time_val;
                Gtk::TreeModelColumn<uint32_t>      duration_val;
                Gtk::TreeModelColumn<uint32_t>      diff_val;
                Gtk::TreeModelColumn<Glib::ustring> time;
                Gtk::TreeModelColumn<Glib::ustring> duration;
                Gtk::TreeModelColumn<Glib::ustring> status;
                Gtk::TreeModelColumn<Glib::ustring> username;
                Gtk::TreeModelColumn<Glib::ustring> text;
        } recRecord;

        Glib::RefPtr<Gtk::ListStore>        recListModel;
        Gtk::TreeView                       *recListTree;

        class ManglerRecorderData {
            public:
                ManglerRecorderData(v3_vrf_data *vrfd = NULL) {
                    this->vrfd = vrfd;
                    next = 0;
                    outputAudio = NULL;
                }
                v3_vrf_data  *vrfd;
                double       next;
                ManglerAudio *outputAudio;
        };

        void hide_activate_cb(void);
        void open_activate_cb(void);
        void close_activate_cb(void);
        void saveas_activate_cb(void);
        void delete_activate_cb(void);
        void playpause_clicked_cb(void);
        void stop_clicked_cb(void);
        void record_toggled_cb(void);
        void info_clicked_cb(void);
        void recListTree_cursor_changed_cb(void);
        void recListTree_row_activated_cb(const Gtk::TreeModel::Path& path, Gtk::TreeViewColumn* column);
        void set(bool isRecording);
        void reset(bool destroying = false);
        void play(void);

        void recInfoDialog_cancel_clicked_cb(void);
        void recInfoDialog_save_clicked_cb(void);

        Glib::ustring float_to_ustring(float val, int precision = 2);
        Glib::ustring bytes_to_readable(double size);
        Glib::ustring msec_to_timestamp(uint32_t milliseconds);
        Glib::ustring timestamp(Glib::ustring format = "%Y%m%d-%H%M%S");
};

#endif

