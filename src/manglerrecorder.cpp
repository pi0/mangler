/*
 * vim: softtabstop=4 shiftwidth=4 cindent foldmethod=marker expandtab
 *
 * $LastChangedDate: 2011-06-27 23:50:10 +0430 (Mon, 27 Jun 2011) $
 * $Revision: 1139 $
 * $LastChangedBy: econnell $
 * $URL: http://svn.mangler.org/mangler/trunk/src/manglerrecorder.cpp $
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
#include "manglerrecorder.h"
#include "mangleraudio.h"
#include "manglercharset.h"

#include <unistd.h>
#include <sys/stat.h>
#include <dirent.h>

ManglerRecorder::ManglerRecorder(Glib::RefPtr<Gtk::Builder> builder) {/*{{{*/
    this->builder = builder;

    builder->get_widget("recWindow", recWindow);
    builder->get_widget("recHide", menuitem);
    menuitem->signal_activate().connect(sigc::mem_fun(this, &ManglerRecorder::hide_activate_cb));

    builder->get_widget("recOpen", menuitem);
    menuitem->signal_activate().connect(sigc::mem_fun(this, &ManglerRecorder::open_activate_cb));
    builder->get_widget("recClose", menuitem);
    menuitem->signal_activate().connect(sigc::mem_fun(this, &ManglerRecorder::close_activate_cb));
    builder->get_widget("recSaveAs", menuitem);
    menuitem->signal_activate().connect(sigc::mem_fun(this, &ManglerRecorder::saveas_activate_cb));
    builder->get_widget("recDelete", menuitem);
    menuitem->signal_activate().connect(sigc::mem_fun(this, &ManglerRecorder::delete_activate_cb));

    builder->get_widget("recPlayPause", button);
    button->signal_clicked().connect(sigc::mem_fun(this, &ManglerRecorder::playpause_clicked_cb));
    builder->get_widget("recStop", button);
    button->signal_clicked().connect(sigc::mem_fun(this, &ManglerRecorder::stop_clicked_cb));
    builder->get_widget("recRecord", recordbutton);
    recordbutton->signal_toggled().connect(sigc::mem_fun(this, &ManglerRecorder::record_toggled_cb));
    builder->get_widget("recInfo", button);
    button->signal_clicked().connect(sigc::mem_fun(this, &ManglerRecorder::info_clicked_cb));

    builder->get_widget("recOpenEntry", fileentry);
    builder->get_widget("recOpenButton", button);
    button->signal_clicked().connect(sigc::mem_fun(this, &ManglerRecorder::open_activate_cb));

    filedialog = new Gtk::FileChooserDialog("Open Recording", Gtk::FILE_CHOOSER_ACTION_OPEN);
    filedialog->add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
    filedialog->add_button(Gtk::Stock::OPEN, Gtk::RESPONSE_OK);
    Gtk::FileFilter vrf_filter;
    vrf_filter.set_name("Ventrilo Recording File (*.vrf)");
    vrf_filter.add_pattern("*.vrf");
    filedialog->add_filter(vrf_filter);
    Gtk::FileFilter all_filter;
    all_filter.set_name("All Files");
    all_filter.add_pattern("*");
    filedialog->add_filter(all_filter);

    builder->get_widget("recScrolledWindow", recScrolledWindow);
    recListModel = Gtk::ListStore::create(recRecord);
    builder->get_widget("recListTree", recListTree);
    recListTree->set_model(recListModel);
    recListTree->append_column("Time", recRecord.time);
    recListTree->append_column("Duration", recRecord.duration);
    recListTree->append_column("Status", recRecord.status);
    recListTree->append_column("Username", recRecord.username);
    recListTree->append_column("", recRecord.text);
    recListTree->signal_cursor_changed().connect(sigc::mem_fun(this, &ManglerRecorder::recListTree_cursor_changed_cb));
    recListTree->signal_row_activated().connect(sigc::mem_fun(this, &ManglerRecorder::recListTree_row_activated_cb));

    builder->get_widget("recInfoDialog", recInfoDialog);
    builder->get_widget("recInfoCancel", button);
    button->signal_clicked().connect(sigc::mem_fun(this, &ManglerRecorder::recInfoDialog_cancel_clicked_cb));
    builder->get_widget("recInfoSave", button);
    button->signal_clicked().connect(sigc::mem_fun(this, &ManglerRecorder::recInfoDialog_save_clicked_cb));

    recdir = ManglerConfig::confdir() + "/recordings";
    DIR *testdir;
    if ((testdir = opendir(recdir.c_str()))) {
        closedir(testdir);
    } else {
        mkdir(recdir.c_str(), 0700);
    }
    filedialog->set_current_folder(recdir);

    isPlaying   = false;
    isRecording = false;

    vrfh = NULL;
    player = NULL;
}/*}}}*/
ManglerRecorder::~ManglerRecorder() {/*{{{*/
    reset(true);
    delete filedialog;
}/*}}}*/

void
ManglerRecorder::show(void) {/*{{{*/
    recWindow->present();
}/*}}}*/
void
ManglerRecorder::hide_activate_cb(void) {/*{{{*/
    recWindow->hide();
}/*}}}*/
void
ManglerRecorder::open_activate_cb(void) {/*{{{*/
    int result = filedialog->run();
    filedialog->hide();
    if (result == Gtk::RESPONSE_OK) {
        path = filedialog->get_current_folder();
        filename = filedialog->get_filename();
        set(false);
    }
}/*}}}*/
void
ManglerRecorder::close_activate_cb(void) {/*{{{*/
    reset();
}/*}}}*/
void
ManglerRecorder::saveas_activate_cb(void) {/*{{{*/
}/*}}}*/
void
ManglerRecorder::delete_activate_cb(void) {/*{{{*/
    if (!vrfh || filename.empty()) {
        return;
    }
    Gtk::MessageDialog confirm("<b>Are you sure you want to delete \"" + fileentry->get_text() + "\"?</b>",
            true, Gtk::MESSAGE_WARNING, Gtk::BUTTONS_YES_NO, true);
    if (confirm.run() == Gtk::RESPONSE_YES) {
        reset();
        unlink(filename.c_str());
    }
}/*}}}*/
void
ManglerRecorder::playpause_clicked_cb(void) {/*{{{*/
    if (!vrfh) {
        return;
    }
    isPlaying = true;
    player = Glib::Thread::create(sigc::mem_fun(*this, &ManglerRecorder::play), false);
}/*}}}*/
void
ManglerRecorder::stop_clicked_cb(void) {/*{{{*/
    player = NULL;
}/*}}}*/
void
ManglerRecorder::record_toggled_cb(void) {/*{{{*/
    if (recordbutton->get_active()) {
        path = recdir;
        filename = path + "/" + timestamp() + ".vrf";
        set(true);
    } else {
        set(false);
    }
}/*}}}*/
void
ManglerRecorder::info_clicked_cb(void) {/*{{{*/
    if (!vrfh) {
        return;
    }
    v3_vrf_data vrfd;
    if (v3_vrf_get_info(vrfh, &vrfd) != V3_OK) {
        mangler->errorDialog(c_to_ustring(_v3_error(NULL)));
        return;
    }
    builder->get_widget("recInfoByEntry", entry);
    entry->set_text(c_to_ustring(vrfd.username));
    builder->get_widget("recInfoComment", textview);
    textview->get_buffer()->set_text(c_to_ustring(vrfd.comment));
    builder->get_widget("recInfoURL", textview);
    textview->get_buffer()->set_text(c_to_ustring(vrfd.url));
    builder->get_widget("recInfoCopyright", textview);
    textview->get_buffer()->set_text(c_to_ustring(vrfd.copyright));
    recInfoDialog->set_icon(mangler->icons["tray_icon"]);
    recInfoDialog->present();
}/*}}}*/
void
ManglerRecorder::recListTree_cursor_changed_cb(void) {/*{{{*/
    if (!vrfh) {
        return;
    }
    builder->get_widget("recPlayPause", widget);
    widget->set_sensitive(recListTree->get_selection()->get_selected());
}/*}}}*/
void
ManglerRecorder::recListTree_row_activated_cb(const Gtk::TreeModel::Path& path, Gtk::TreeViewColumn* column) {/*{{{*/
    if (!vrfh) {
        return;
    }
    isPlaying = false;
    player = Glib::Thread::create(sigc::mem_fun(*this, &ManglerRecorder::play), false);
}/*}}}*/
void
ManglerRecorder::set(bool isRecording) {/*{{{*/
    reset();
    if (isRecording) {
        if (v3_vrf_record_start(filename.c_str()) != V3_OK) {
            mangler->errorDialog(c_to_ustring(_v3_error(NULL)));
            recordbutton->set_active(false);
            return;
        }
        this->isRecording = true;
    } else {
        this->isRecording = false;
        if (!(vrfh = v3_vrf_init(filename.c_str()))) {
            mangler->errorDialog(c_to_ustring(_v3_error(NULL)));
            return;
        }
        v3_vrf_data vrfd;
        if (v3_vrf_get_info(vrfh, &vrfd) != V3_OK) {
            reset();
            mangler->errorDialog(c_to_ustring(_v3_error(NULL)));
            return;
        }
        builder->get_widget("recSize", label);
        label->set_text(bytes_to_readable(vrfd.size));
        builder->get_widget("recCount", label);
        label->set_text(float_to_ustring(v3_vrf_get_count(vrfh), 0));
        builder->get_widget("recPlatform", label);
        label->set_text(c_to_ustring(vrfd.platform));
        builder->get_widget("recVersion", label);
        label->set_text(c_to_ustring(vrfd.version));
        builder->get_widget("recCodec", label);
        label->set_text(c_to_ustring(v3_get_codec(vrfd.codec, vrfd.codecformat)->name));
        totalduration = 0;
        for (uint32_t ctr = 0, cnt = v3_vrf_get_count(vrfh); ctr < cnt; ctr++) {
            if (v3_vrf_get_segment(vrfh, ctr, &vrfd) != V3_OK) {
                reset();
                mangler->errorDialog(c_to_ustring(_v3_error(NULL)));
                return;
            }
            Gtk::TreeModel::Row row = *(recListModel->append());
            row[recRecord.id]           = ctr;
            row[recRecord.time_val]     = vrfd.time;
            row[recRecord.duration_val] = vrfd.duration;
            row[recRecord.time]         = msec_to_timestamp(vrfd.time);
            row[recRecord.duration]     = float_to_ustring(vrfd.duration / 1000.0, 2);
            row[recRecord.status]       = "";
            row[recRecord.username]     = c_to_ustring(vrfd.username);
            row[recRecord.text]         = "";
            totalduration += vrfd.duration;
        }
        builder->get_widget("recDuration", label);
        label->set_text(float_to_ustring(totalduration / 60000.0, 1) + " min");
        builder->get_widget("recInfo", widget);
        widget->set_sensitive(true);
    }
    fileentry->set_text(filename.substr(path.length() + 1));
    builder->get_widget("recType", label);
    label->set_text("VRF");
    recListTree->set_sensitive(true);
    builder->get_widget("recClose", widget);
    widget->set_sensitive(true);
    builder->get_widget("recDelete", widget);
    widget->set_sensitive(true);
    builder->get_widget("recInfos", widget);
    widget->set_sensitive(true);
    recScrolledWindow->get_vadjustment()->set_value(0);
    if (recListModel->children().size()) {
        recListTree->set_cursor(recListModel->get_path(recListModel->children().begin()));
        builder->get_widget("recPlayPause", widget);
        widget->grab_focus();
    }
}/*}}}*/
void
ManglerRecorder::reset(bool destroying) {/*{{{*/
    player = (Glib::Thread*)destroying;
    isPlaying = false;
    if (isRecording) {
        v3_vrf_record_stop();
        recordbutton->set_active(false);
    }
    isRecording = false;
    if (vrfh) {
        v3_vrf_destroy(vrfh);
        vrfh = NULL;
    }
    recListModel->clear();
    if (destroying) {
        return;
    }
    for (int ctr = 0, cnt = recListTree->get_columns().size(); ctr < cnt; ctr++) {
        recListTree->get_column(ctr)->queue_resize();
    }
    recListTree->set_sensitive(false);
    fileentry->set_text("");
    builder->get_widget("recType", label);
    label->set_text("N/A");
    builder->get_widget("recSize", label);
    label->set_text("N/A");
    builder->get_widget("recCount", label);
    label->set_text("N/A");
    builder->get_widget("recDuration", label);
    label->set_text("N/A");
    builder->get_widget("recPlatform", label);
    label->set_text("N/A");
    builder->get_widget("recVersion", label);
    label->set_text("N/A");
    builder->get_widget("recCodec", label);
    label->set_text("N/A");
    builder->get_widget("recClose", widget);
    widget->set_sensitive(false);
    builder->get_widget("recDelete", widget);
    widget->set_sensitive(false);
    builder->get_widget("recInfos", widget);
    widget->set_sensitive(false);
    builder->get_widget("recPlayPause", widget);
    widget->set_sensitive(false);
    builder->get_widget("recInfo", widget);
    widget->set_sensitive(false);
    builder->get_widget("recInfoByEntry", entry);
    entry->set_text("");
    builder->get_widget("recInfoComment", textview);
    textview->get_buffer()->set_text("");
    builder->get_widget("recInfoURL", textview);
    textview->get_buffer()->set_text("");
    builder->get_widget("recInfoCopyright", textview);
    textview->get_buffer()->set_text("");
    recInfoDialog->hide();
}/*}}}*/
void
ManglerRecorder::can_record(bool isConnected) {/*{{{*/
    if (!isConnected && isRecording) {
        set(false);
    }
    recordbutton->set_sensitive(isConnected);
}/*}}}*/
void
ManglerRecorder::record(Glib::ustring username, Glib::ustring text, uint32_t index, uint32_t time, bool stopped, bool flushed) {/*{{{*/
    if (!isRecording) {
        return;
    }
    Gtk::TreeModel::Children children = recListModel->children();
    Gtk::TreeModel::Row row;
    if (!flushed && !children[index]) {
        row = *(recListModel->append());
        row[recRecord.id]           = index;
        row[recRecord.time_val]     = time;
        row[recRecord.duration_val] = 0;
        row[recRecord.diff_val]     = 0;
        row[recRecord.time]         = msec_to_timestamp(time);
        row[recRecord.duration]     = float_to_ustring(0, 2);
        row[recRecord.status]       = "Rec";
        row[recRecord.username]     = username;
        row[recRecord.text]         = (text.length()) ? text : "";
        builder->get_widget("recCount", label);
        label->set_text(float_to_ustring(children.size(), 0));
        recListTree->set_cursor(recListModel->get_path(row));
    }
    if (flushed) {
        for (Gtk::TreeModel::iterator iter = children.begin(); iter != children.end(); iter++) {
            (*iter)[recRecord.status] = "*";
        }
        return;
    }
    row = children[index];
    if (stopped) {
        row[recRecord.status] = "*";
    } else {
        row[recRecord.duration_val] = time - row[recRecord.time_val];
    }
    if (stopped || row[recRecord.diff_val] + 100 < row[recRecord.duration_val]) {
        row[recRecord.diff_val] = time - row[recRecord.time_val];
        row[recRecord.duration] = float_to_ustring(row[recRecord.duration_val] / 1000.0, 2);
    }
}/*}}}*/
void
ManglerRecorder::play(void) {/*{{{*/
    Glib::Thread *self = Glib::Thread::self();
    Gtk::TreeModel::iterator iter = recListTree->get_selection()->get_selected();
    Gtk::TreeModel::Children children = recListModel->children();
    if (!vrfh || !iter || children.empty() || player != self) {
        if (player == self) {
            player = NULL;
        }
        return;
    } else {
        gdk_threads_enter();
        builder->get_widget("recStop", widget);
        widget->set_sensitive(true);
        gdk_threads_leave();
    }
    std::map<uint32_t, ManglerRecorderData *> recData;
    std::map<uint32_t, ManglerRecorderData *>::iterator recIter;
    v3_vrf_data *next = NULL;
    double elapsed;
    struct timeval start, now, diff;
    elapsed = (*iter)[recRecord.time_val];
    gettimeofday(&start, NULL);
    for (;;) {
        if (!next && iter != children.end()) {
            next = (v3_vrf_data *)malloc(sizeof(v3_vrf_data));
            v3_vrf_data_init(next);
            if (v3_vrf_get_segment(vrfh, (*iter)[recRecord.id], next) != V3_OK) {
                free(next);
                next = NULL;
                iter++;
            } else if (isPlaying && children.size()) {
                gdk_threads_enter();
                Gtk::Adjustment *vadjustment = recScrolledWindow->get_vadjustment();
                float adj = vadjustment->get_upper() * ((*iter)[recRecord.id] / (float)children.size()) - vadjustment->get_page_size() / 2.0;
                if (adj < 0) {
                    adj = 0;
                } else if (adj > vadjustment->get_upper() - vadjustment->get_page_size()) {
                    adj = vadjustment->get_upper() - vadjustment->get_page_size();
                }
                vadjustment->set_value(adj);
                recListTree->set_cursor(recListModel->get_path(iter));
                gdk_threads_leave();
            }
            if (!isPlaying) {
                iter = children.end();
            }
        }
        gettimeofday(&now, NULL);
        timeval_subtract(&diff, &now, &start);
        gettimeofday(&start, NULL);
        elapsed += diff.tv_sec * 1000.0 + diff.tv_usec / 1000.0;
        if ((player != self || (iter == children.end() && !next)) && recData.empty()) {
            if (next) {
                v3_vrf_data_destroy(next);
                free(next);
            }
            break;
        }
        if (next && next->time <= elapsed) {
            recData[next->id] = new ManglerRecorderData(next);
            next = NULL;
            if (isPlaying) {
                iter++;
            }
        }
        double duration = 0;
        for (recIter = recData.begin(); recIter != recData.end() && recData.size();) {
            ManglerRecorderData *recd = recIter->second;
            if (player == self && recd->next > elapsed) {
                recIter++;
                continue;
            }
            v3_vrf_data *vrfd = recd->vrfd;
            if (player == self) {
                v3_vrf_get_audio(vrfh, vrfd->id, vrfd);
            }
            switch ((player == self) ? vrfd->type : V3_VRF_DATA_NULL) {
              case V3_VRF_DATA_AUDIO:
                if (!recd->outputAudio) {
                    recd->outputAudio = new ManglerAudio(AUDIO_OUTPUT, vrfd->rate, vrfd->channels, 0, 0, false);
                }
                if (vrfd->length) {
                    recd->outputAudio->queue(vrfd->length, (uint8_t *)vrfd->data);
                }
                if (!recd->next) {
                    recd->next = vrfd->time;
                    gdk_threads_enter();
                    children[recIter->first][recRecord.status] = "Play";
                    builder->get_widget("recCodec", label);
                    label->set_text(c_to_ustring(v3_get_codec(vrfd->codec, vrfd->codecformat)->name));
                    gdk_threads_leave();
                }
                duration += (vrfd->length / (float)(vrfd->rate * sizeof(int16_t) * vrfd->channels)) * 1000.0;
                if (duration < 10) {
                    continue;
                }
                recd->next += duration;
                recIter++;
                break;
              case V3_VRF_DATA_TEXT:
                gdk_threads_enter();
                children[recIter->first][recRecord.text] = c_to_ustring(vrfd->text);
                children[recIter->first][recRecord.status] = "*";
                gdk_threads_leave();
                recIter++;
                break;
              case V3_VRF_DATA_NULL:
              default:
                if (recd->outputAudio) {
                    recd->outputAudio->finish();
                }
                v3_vrf_data_destroy(vrfd);
                free(vrfd);
                delete recd;
                gdk_threads_enter();
                if (vrfh && children[recIter->first] && children[recIter->first][recRecord.status] == "Play") {
                    children[recIter->first][recRecord.status] = "*";
                }
                gdk_threads_leave();
                recData.erase(recIter);
                recIter = recData.begin();
                break;
            }
            duration = 0;
        }
        usleep(10000);
    }
    if (!player || player == self) {
        gdk_threads_enter();
        builder->get_widget("recStop", widget);
        widget->set_sensitive(false);
        gdk_threads_leave();
    }
}/*}}}*/

void
ManglerRecorder::recInfoDialog_cancel_clicked_cb(void) {/*{{{*/
    recInfoDialog->hide();
}/*}}}*/
void
ManglerRecorder::recInfoDialog_save_clicked_cb(void) {/*{{{*/
    if (vrfh) {
        v3_vrf_data vrfd;
        v3_vrf_data_init(&vrfd);
        builder->get_widget("recInfoByEntry", entry);
        strncpy(vrfd.username, ustring_to_c(entry->get_text()).c_str(), sizeof(vrfd.username));
        builder->get_widget("recInfoComment", textview);
        strncpy(vrfd.comment, ustring_to_c(textview->get_buffer()->get_text()).c_str(), sizeof(vrfd.comment));
        builder->get_widget("recInfoURL", textview);
        strncpy(vrfd.url, ustring_to_c(textview->get_buffer()->get_text()).c_str(), sizeof(vrfd.url));
        builder->get_widget("recInfoCopyright", textview);
        strncpy(vrfd.copyright, ustring_to_c(textview->get_buffer()->get_text()).c_str(), sizeof(vrfd.copyright));
        if (v3_vrf_put_info(vrfh, &vrfd) != V3_OK) {
            mangler->errorDialog(c_to_ustring(_v3_error(NULL)));
        }
    }
    recInfoDialog->hide();
}/*}}}*/

Glib::ustring
ManglerRecorder::float_to_ustring(float val, int precision) {/*{{{*/
    char tmp[128] = "";

    snprintf(tmp, sizeof(tmp) - 1, "%.*f", precision, val);

    return c_to_ustring(tmp);
}/*}}}*/

Glib::ustring
ManglerRecorder::bytes_to_readable(double size) {/*{{{*/
    static const char *const suffixes[] = {
        "bytes", "KB", "MB", "GB", "TB", "PB", "EB", "ZB", "YB"
    };
    int s;
    for (s = 0; size >= 1 << 10; s++) {
        size /= 1 << 10;
    }

    return float_to_ustring(size, 1) + " " + c_to_ustring(suffixes[s]);
}/*}}}*/

Glib::ustring
ManglerRecorder::msec_to_timestamp(uint32_t milliseconds) {/*{{{*/
    char timestamp[256] = "";
    uint32_t seconds = milliseconds / 1000;
    uint32_t minutes = seconds / 60;
    uint32_t hours   = minutes / 60;

    snprintf(timestamp, sizeof(timestamp) - 1, "%02u:%02u:%02u", hours, minutes % 60, seconds % 60);

    return c_to_ustring(timestamp);
}/*}}}*/

Glib::ustring
ManglerRecorder::timestamp(Glib::ustring format) {/*{{{*/
    char timestamp[256] = "";
    struct timeval tv;
    time_t t;
    struct tm *tm;

    gettimeofday(&tv, NULL);
    t = tv.tv_sec;
    tm = localtime(&t);
    if (!strftime(timestamp, sizeof(timestamp) - 1, format.c_str(), tm)) {
        snprintf(timestamp, sizeof(timestamp) - 1, "%lu", tv.tv_sec * 1000000 + tv.tv_usec);
    }

    return c_to_ustring(timestamp);
}/*}}}*/

