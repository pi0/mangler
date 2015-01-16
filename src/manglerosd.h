/*
 * vim: softtabstop=4 shiftwidth=4 cindent foldmethod=marker expandtab
 *
 * $LastChangedDate: 2011-06-27 23:50:10 +0430 (Mon, 27 Jun 2011) $
 * $Revision: 1139 $
 * $LastChangedBy: econnell $
 * $URL: http://svn.mangler.org/mangler/trunk/src/manglerosd.h $
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

#ifndef _MANGLEROSD_H
#define _MANGLEROSD_H

#include "config.h"

#ifdef HAVE_XOSD

#include <stdint.h>
#include <string>
#include <list>
#include <xosd.h>
extern "C" {
#include <ventrilo3.h>
}

class ManglerOsd {/*{{{*/
    private:
        xosd                             *osd;
        int                              osd_max_lines;
        std::list<std::string>           userList;
        void createOsd(void);

    public:
        ManglerOsd();
        void destroyOsd(void);
        bool checkOsdEnabled(void);
        void updateOsd(void);
        void addUser(uint32_t);
        void removeUser(uint32_t);

};/*}}}*/

#endif

#endif

