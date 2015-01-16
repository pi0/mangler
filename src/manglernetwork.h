/*
 * vim: softtabstop=4 shiftwidth=4 cindent foldmethod=marker expandtab
 *
 * $LastChangedDate: 2011-06-27 23:50:10 +0430 (Mon, 27 Jun 2011) $
 * $Revision: 1139 $
 * $LastChangedBy: econnell $
 * $URL: http://svn.mangler.org/mangler/trunk/src/manglernetwork.h $
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

#ifndef _MANGLERNETWORK_H
#define _MANGLERNETWORK_H

class ManglerNetwork
{
    public:
        ManglerNetwork(Glib::RefPtr<Gtk::Builder> builder);
        void connect(Glib::ustring hostname, Glib::ustring port, Glib::ustring username, Glib::ustring password, Glib::ustring phonetic);
        void disconnect(void);
        Gtk::Button *button;
        Gtk::Label *label;
        Gtk::ComboBox *combobox;
        Gtk::Statusbar *statusbar;
        Glib::RefPtr<Gtk::Builder> builder;
};

#endif

