/*
 * vim: softtabstop=4 shiftwidth=4 cindent foldmethod=marker expandtab
 *
 * $LastChangedDate: 2011-06-27 23:50:10 +0430 (Mon, 27 Jun 2011) $
 * $Revision: 1139 $
 * $LastChangedBy: econnell $
 * $URL: http://svn.mangler.org/mangler/trunk/src/manglerconfig.cpp $
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

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include "mangler.h"
#include "manglerconfig.h"
#include <gdk/gdkx.h>
#include "config.h"

#include "manglerserverlist.h"

#include <cstdlib>
#include <sys/types.h>
#include <dirent.h>

// only needed for ConvertOldConfig
#include <fstream>

// config directory, relative to $HOME
#define CONFIG_DIRECTORY ".mangler"

using namespace std;

class ManglerConfigDir {/*{{{*/
    public:
    static void dirInit();
    static string filename(const string &name);
    private:
    static bool &initialized();
    static string &confdir();
};/*}}}*/

bool &ManglerConfigDir::initialized() { static bool myInit( false ); return myInit; }
string &ManglerConfigDir::confdir() { static string myString; return myString; }

void ManglerConfigDir::dirInit() {/*{{{*/
    string homedir = getenv("HOME");
    if (! homedir.length() || homedir[homedir.length()-1] != '/') homedir += "/";
    confdir() = homedir + CONFIG_DIRECTORY;
    if (confdir()[confdir().length()-1] == '/') confdir().erase(confdir().length()-1, confdir().npos);
    DIR *confDIR = ::opendir(confdir().c_str());
    if (! confDIR) {
        if (::mkdir(confdir().c_str(), 0700)) {
            fprintf(stderr, "Unable to make directory '%s'\n", confdir().c_str());
            fprintf(stderr, "No configuration settings can be saved.\n");
            return;
        }
    }
    confdir() += "/";
    initialized() = true;
}/*}}}*/

string ManglerConfigDir::filename(const string &name) {/*{{{*/
    if (! initialized()) dirInit();
    return confdir() + name;
}/*}}}*/

ManglerConfig::ManglerConfig() /*{{{*/
    : config( ManglerConfigDir::filename("config.ini") )
    , servers( ManglerConfigDir::filename("servers.ini") ) {
    // this is where we check to see if config.ini was loaded
    if (! config.contains("mangler")) {
        struct stat statbuf;
        string oldfile = getenv("HOME");
        if (oldfile[oldfile.length()-1] != '/') oldfile += "/.manglerrc";
        else oldfile += ".manglerrc";
        if (stat(oldfile.c_str(), &statbuf) == 0) {
            ConvertOldConfig();
        } else {
            istringstream sin( DefaultConfiguration );
            config.load(sin);
        }
        save();
        // should have something now!!
        config.reload();
        servers.reload();
    }
}/*}}}*/

ManglerConfig::~ManglerConfig() {/*{{{*/
    save(); // might as well :)
}/*}}}*/

string ManglerConfig::confdir() {/*{{{*/
    string confdir = getenv("HOME");
    return confdir + "/" + CONFIG_DIRECTORY;
}/*}}}*/

void ManglerConfig::ConvertOldConfig() {/*{{{*/
    string buf;
    config.clear();
    servers.clear();
    istringstream sin( DefaultConfiguration );
    config.load(sin);
    map<int, string> serv_names;
    string oldconf = getenv("HOME");
    if (! oldconf.length() || oldconf[oldconf.length() - 1] != '/') oldconf += "/";
    oldconf += ".manglerrc";
    ifstream fin( oldconf.c_str() );
    if (! fin) {
        cerr << "unable to open old .manglerrc file for reading.  can't convert." << endl;
    }
    for (int cnt = 1;;++cnt) {
        getline(fin, buf);
        if (fin.eof()) break;
        unsigned eq_pos = 0;
        while (eq_pos < buf.length() && buf[eq_pos] != '=') ++eq_pos;
        if (eq_pos == buf.length()) {
            cerr << "error parsing .manglerrc: line " << cnt << endl;
            continue;
        }
        string var = buf.substr(0, eq_pos);
        string val = buf.substr(eq_pos + 1, buf.npos);
        if (var.substr(0, 13) == "notification.") {
            var.erase(12, 1);
            var[0] = 'N';
            var[12] = (char)toupper(var[12]);
        } else if (var.substr(0, 7) == "window.") {
            if (var.substr(7, 5) == "width" || var.substr(7, 6) == "height") {
                var.erase(6, 1);
                var[0] = 'W';
                var[6] = (char)toupper(var[6]);
            } else {
                var.erase(0, 7);
                var[0] = (char)toupper(var[0]);
            }
        } else if (var.substr(0, 11) == "serverlist.") {
            int serv_id = 0;
            string serv_var = "";
            unsigned i = 11;
            while (i < var.length() && var[i] != '.') ++i;
            if (i == var.length()) {
                cerr << "error parsing .manglerrc: line " << cnt << ": serverlist entry without id" << endl;
                continue;
            }
            serv_id = atoi(var.substr(11, i - 11).c_str());
            serv_var = var.substr(i+1, var.npos);
            if (! serv_var.length()) {
                cerr << "error parsing .manglerrc: line " << cnt << ": serverlist line looks mangled ;)" << endl;
                continue;
            }
            if (serv_var == "name") {
                serv_names[serv_id] = val;
                continue;
            } else if (serv_var == "accept_u2u") {
                serv_var = "AcceptU2U";
            } else if (serv_var == "accept_pages") {
                serv_var = "AcceptPages";
            } else if (serv_var == "accept_privchat") {
                serv_var = "AcceptPrivateChat";
            } else if (serv_var == "allow_recording") {
                serv_var = "AllowRecording";
            } else if (serv_var == "persistent_connection") {
                serv_var = "PersistentConnection";
            } else if (serv_var == "persistent_comments") {
                serv_var = "PersistentComments";
            } else if (serv_var == "motdhash") {
                serv_var = "MotdHash";
            } else if (serv_var == "defaultchannelid") {
                serv_var = "DefaultChannel";
            } else if (serv_var == "defaultchannel") {
                continue;
            } else if (serv_var.substr(0, 7) == "volume.") {
                string vol_usr = serv_var.substr(7, serv_var.npos);
                serv_var = "UserVolume[" + vol_usr + "]";
            } else if (serv_var.substr(0, 12) == "channelpass.") {
                string chan_pass = serv_var.substr(12, serv_var.npos);
                serv_var = "ChannelPassword[" + chan_pass + "]";
            } else {
                serv_var[0] = (char)toupper(serv_var[0]);
            }
            string serv_name = serv_names[serv_id];
            if (serv_name.length()) {
                servers[serv_name][serv_var] = val;
                continue;
            } else {
                cerr << "error parsing .manglerrc: line " << cnt << ": unknown server id" << endl;
                continue;
            }
        } else if (var.substr(0, 13) != "qc_lastserver" && var.substr(0, 14) != "lv3_debuglevel") {
            var[0] = (char)toupper(var[0]);
        }
        // if we get here, it's a main config option
        config["mangler"][var] = val;
    }
    fin.close();
    int lcs_id = config["mangler"]["LastConnectedServerID"].toInt();
    string lcs_name = serv_names[lcs_id];
    config["mangler"]["LastConnectedServerName"] = lcs_name;
    config["mangler"].erase("LastConnectedServerID");
}/*}}}*/

void ManglerConfig::save() {/*{{{*/
    //mutex.lock();
    config.save();
    servers.save();
    //mutex.unlock();
}/*}}}*/

std::vector<int> ManglerConfig::PushToTalkXKeyCodes() const {/*{{{*/
    int begin = 0;
    uint32_t ctr;
    GdkWindow   *rootwin = gdk_get_default_root_window();
    vector<int> ret;
    if (! config.contains("mangler") || ! config.at("mangler").contains("PushToTalkKeyValue"))
        return ret;
    Glib::ustring pttString = config.at("mangler").at("PushToTalkKeyValue").toUString();
    Glib::ustring keyname;
    for (ctr = 0; ctr < pttString.length(); ctr++) {
        if (pttString[ctr] == '+') {
            keyname = pttString.substr(begin, ctr-begin);
            begin = ctr+1;
            if (keyname[0] == '<' && keyname[keyname.length()-1] == '>') {
                keyname = keyname.substr(1, keyname.length() - 2);
            }
            int keycode = XKeysymToKeycode(GDK_WINDOW_XDISPLAY(rootwin), XStringToKeysym(keyname.c_str()));
            ret.push_back(keycode);
        }
    }
    keyname = pttString.substr(begin, ctr-begin);
    if (keyname[0] == '<' && keyname[keyname.length()-1] == '>') {
        keyname = keyname.substr(1, keyname.length() - 2);
    }
    int keycode = XKeysymToKeycode(GDK_WINDOW_XDISPLAY(rootwin), XStringToKeysym(keyname.c_str()));
    ret.push_back(keycode);
    return ret;
}/*}}}*/

iniValue &ManglerConfig::operator[](const string &configVar) {/*{{{*/
    return config["mangler"][configVar];
}/*}}}*/

bool ManglerConfig::hasUserVolume(const string &server, const string &user) const {/*{{{*/
    string varname = string( "UserVolume[" ) + user + string( "]" );
    return servers.contains(server) && servers.at(server).contains(varname);
}/*}}}*/

iniValue &ManglerConfig::UserVolume(const string &server, const string &user) {/*{{{*/
    string varname = string( "UserVolume[" ) + user + string( "]" );
    return servers[server][varname];
}/*}}}*/

bool ManglerConfig::hasUserMuted(const string &server, const string &user) const {/*{{{*/
    string varname = string( "UserMuted[" ) + user + string( "]" );
    return servers.contains(server) && servers.at(server).contains(varname);
}/*}}}*/

iniValue &ManglerConfig::UserMuted(const string &server, const string &user) {/*{{{*/
    string varname = string( "UserMuted[" ) + user + string( "]" );
    return servers[server][varname];
}/*}}}*/

iniValue &ManglerConfig::ChannelPassword(const string &server, uint16_t channel) {/*{{{*/
    string varname = string( "ChannelPassword[" ) + iniVariant( channel ).toString() + string( "]" );
    return servers[server][varname];
}/*}}}*/

const char *ManglerConfig::DefaultConfiguration = "[mangler]\n"
"PushToTalkKeyEnabled=0\n"
"PushToTalkKeyValue=\n"
"PushToTalkMouseEnabled=0\n"
"PushToTalkMouseValue=\n"
"AudioIntegrationEnabled=0\n"
"AudioIntegrationPlayer=0\n"
"VoiceActivationEnabled=0\n"
"VoiceActivationSilenceDuration=2000\n"
"VoiceActivationSensitivity=25\n"
"InputDeviceName=\n"
"InputDeviceCustomName=\n"
"OutputDeviceName=\n"
"OutputDeviceCustomName=\n"
"NotificationDeviceName=\n"
"NotificationDeviceCustomName=\n"
"NotificationLoginLogout=1\n"
"NotificationChannelEnterLeave=1\n"
"NotificationTransmitStartStop=1\n"
"NotificationTextToSpeech=1\n"
"MouseDeviceName=\n"
#ifdef HAVE_PULSE
"AudioSubsystem=pulse\n"
#elif HAVE_ALSA
"AudioSubsystem=alsa\n"
#elif HAVE_OSS
"AudioSubsystem=oss\n"
#endif
"qc_lastserver.hostname=\n"
"qc_lastserver.port=\n"
"qc_lastserver.username=\n"
"qc_lastserver.password=\n"
"LastConnectedServerId=0\n"
"lv3_debuglevel=0\n"
"MasterVolumeLevel=79\n"
"InputGainLevel=79\n"
"WindowWidth=400\n"
"WindowHeight=525\n"
"ButtonsHidden=0\n"
"ServerInfoHidden=0\n"
"GuestFlagHidden=0\n"
"ChatTimestamps=1\n";

