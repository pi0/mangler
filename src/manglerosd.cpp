/*
 * vim: softtabstop=4 shiftwidth=4 cindent foldmethod=marker expandtab
 *
 * $LastChangedDate: 2011-06-27 23:50:10 +0430 (Mon, 27 Jun 2011) $
 * $Revision: 1139 $
 * $LastChangedBy: econnell $
 * $URL: http://svn.mangler.org/mangler/trunk/src/manglerosd.cpp $
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
#include "manglerosd.h"

#ifdef HAVE_XOSD

using namespace std;

ManglerOsd::ManglerOsd() {/*{{{*/
    userList = list<string>();
    osd_max_lines = 10;
    osd = NULL;
}/*}}}*/

void
ManglerOsd::createOsd(void) {/*{{{*/
    if (osd) {
        destroyOsd();
    }
    osd = xosd_create(osd_max_lines);
    //xosd_set_pos(osd,XOSD_top);
    //xosd_set_align(osd,XOSD_center);
    xosd_set_pos(osd, (xosd_pos)(Mangler::config["OnScreenDisplayVerticalPosition"].toInt()));
    xosd_set_align(osd, (xosd_align)(Mangler::config["OnScreenDisplayHorizontalAlignment"].toInt()));
    if (Mangler::config["OnScreenDisplayFontSize"].toDouble() > 8.0) {
        Glib::ustring fontstr = Glib::ustring::compose( "-*-*-*-*-*-*-0-%1-*-*-*-*-*-*", (int)(Mangler::config["OnScreenDisplayFontSize"].toDouble() * 10.0));
        xosd_set_font(osd, fontstr.c_str());
    }
    if (Mangler::config["OnScreenDisplayColor"].length()) {
        xosd_set_colour(osd, Mangler::config["OnScreenDisplayColor"].toCString());
    }
}/*}}}*/

void
ManglerOsd::destroyOsd(void) {/*{{{*/
    userList.clear();
    if (osd) {
        xosd_destroy(osd);
        osd = NULL;
    }
}/*}}}*/

bool
ManglerOsd::checkOsdEnabled(void) {/*{{{*/
    if (!Mangler::config["OnScreenDisplayEnabled"].toBool()) {
        if (osd) {
            destroyOsd();
        }
        return false;
    } else if (!osd) {
        createOsd();
    }
    return true;
}/*}}}*/

void
ManglerOsd::updateOsd(void) {/*{{{*/
    if (!checkOsdEnabled()) {
        return;
    }
    if (userList.empty()) {
        xosd_hide(osd);
        return;
    }
    xosd_pos mosd_pos = (xosd_pos)(Mangler::config["OnScreenDisplayVerticalPosition"].toInt());

    int i = (mosd_pos == XOSD_bottom) ? osd_max_lines - 1 : 0;
    list<string>::iterator it;
    for(it=userList.begin(); it!= userList.end(); ++it) {
        xosd_display(osd,i,XOSD_string,it->c_str());
        i += (mosd_pos == XOSD_bottom) ? -1 : 1;
        if ( i >= osd_max_lines || i < 0) {
            break;
        }
    }
    
    while (i >= 0 && i < osd_max_lines) {
        xosd_display(osd,i,XOSD_string," ");
        i += (mosd_pos == XOSD_bottom) ? -1 : 1;
    }
}/*}}}*/

/*
 * Adds a user to the list of users talking
 */
void
ManglerOsd::addUser(uint32_t id) {/*{{{*/
    if (!checkOsdEnabled()) {
        return;
    }

    v3_user *u;
    list<string>::iterator it;
    if ((u = v3_get_user(id))) {
        /* Don't 're-add' existing names */
        for(it=userList.begin(); it!= userList.end(); ++it) {
            if (strcmp(it->c_str(),u->name)==0) {
                return;
            }
        }
        string s(u->name);
        userList.push_front(s);
        updateOsd();
        v3_free_user(u);
    }
}/*}}}*/


/*
 * Remove a user to the list of users talking
 */
void
ManglerOsd::removeUser(uint32_t id) {/*{{{*/
    if (!checkOsdEnabled()) {
        return;
    }

    v3_user *u;
    if ((u = v3_get_user(id))) {
        string s(u->name);
        userList.remove(s);
        updateOsd();
        v3_free_user(u);
    }
}/*}}}*/

#endif

