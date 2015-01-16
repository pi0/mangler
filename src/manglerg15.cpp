/*
 * vim: softtabstop=4 shiftwidth=4 cindent foldmethod=marker expandtab
 *
 * $LastChangedDate: 2012-06-16 23:40:54 +0430 (Sat, 16 Jun 2012) $
 * $Revision: 1176 $
 * $LastChangedBy: econnell $
 * $URL: http://svn.mangler.org/mangler/trunk/src/manglerg15.cpp $
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
#include "manglerg15.h"

#ifdef HAVE_G15

ManglerG15::ManglerG15()
{
    if ((fd = new_g15_screen(G15_G15RBUF)) > 0) {
        memset(&canvas, 0, sizeof(g15canvas));
        memcpy(&canvas.buffer, g15_blank, G15_BUFFER_LEN);
        g15r_renderString(&canvas, (unsigned char *)"Mangler", 0, G15_TEXT_LARGE, 50, G15_LCD_HEIGHT/2-5);
        g15_send(fd, (char *)&canvas.buffer, G15_BUFFER_LEN);
    }
}

void
ManglerG15::addevent(Glib::ustring text) {
    return;
    vector<Glib::ustring>::iterator it;
    int ctr;

    if (fd <= 0) {
        return;
    }
    if (events.size() > 5) {
        events.erase(events.begin());
    }
    events.push_back(text);
    memset(&canvas, 0, sizeof(g15canvas));
    memcpy(&canvas.buffer, g15_blank, G15_BUFFER_LEN);
    for (ctr = 1, it = events.begin(); it < events.end(); it++, ctr+=7) {
        g15r_renderString(&canvas, (unsigned char *)((Glib::ustring)*it).c_str(), 0, G15_TEXT_SMALL, 43, ctr);
        //fprintf(stderr, "%s\n", ((Glib::ustring)*it).c_str() );
    }
    g15_send(fd, (char *)&canvas.buffer, G15_BUFFER_LEN);
}

void
ManglerG15::update(Glib::ustring server, Glib::ustring lastXmit,  Glib::ustring serverJoin,  Glib::ustring chanJoin,  Glib::ustring ping) {
    char buf[130];
    if (fd <= 0) {
        return;
    }
    if (server != "") {
        this->server = server;
    }
    if (lastXmit != "") {
        this->lastXmit = lastXmit;
    }
    if (serverJoin != "") {
        this->serverJoin = serverJoin;
    }
    if (chanJoin != "") {
        this->chanJoin = chanJoin;
    }
    if (ping != "") {
        this->ping = ping;
    }
    memset(&canvas, 0, sizeof(g15canvas));
    memset(buf, 0, 130);
    sprintf(buf, "Mangler - %-25.25s     ", this->server.c_str());
    g15r_renderString(&canvas, (unsigned char *)buf, 0, G15_TEXT_SMALL, 0, 0);
    g15r_drawLine(&canvas, 0, 6, G15_LCD_WIDTH, 6, G15_COLOR_BLACK);
    sprintf(buf, "Xmit:%-32.32s", this->lastXmit.c_str());
    g15r_renderString(&canvas, (unsigned char *)buf, 0, G15_TEXT_LARGE, 6, 8);
    sprintf(buf, "ChanJoin: %-32.32s", this->chanJoin.c_str());
    g15r_renderString(&canvas, (unsigned char *)buf, 0, G15_TEXT_MED, 0, 18);
    sprintf(buf, "SrvrJoin: %-32.32s", this->serverJoin.c_str());
    g15r_renderString(&canvas, (unsigned char *)buf, 0, G15_TEXT_MED, 0, 26);
    sprintf(buf, "Ping    : %-32.32s", this->ping.c_str());
    g15r_renderString(&canvas, (unsigned char *)buf, 0, G15_TEXT_MED, 0, 34);
    g15_send(fd, (char *)&canvas.buffer, G15_BUFFER_LEN);
}


#endif

