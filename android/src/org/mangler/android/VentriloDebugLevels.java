/*
 * Copyright 2010 Daniel Sloof <daniel@danslo.org>
 *
 * This file is part of Mangler.
 *
 * $LastChangedDate: 2010-07-26 09:45:54 +0430 (Mon, 26 Jul 2010) $
 * $Revision: 1024 $
 * $LastChangedBy: ekilfoil $
 * $URL: http://svn.mangler.org/mangler/trunk/android/src/org/mangler/android/VentriloDebugLevels.java $
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

package org.mangler.android;

public class VentriloDebugLevels {
	public static final int V3_DEBUG_NONE               = 0;
	public static final int V3_DEBUG_STATUS             = 1;
	public static final int V3_DEBUG_ERROR              = 1 << 2;
	public static final int V3_DEBUG_STACK              = 1 << 3;
	public static final int V3_DEBUG_INTERNAL           = 1 << 4;
	public static final int V3_DEBUG_PACKET             = 1 << 5;
	public static final int V3_DEBUG_PACKET_PARSE       = 1 << 6;
	public static final int V3_DEBUG_PACKET_ENCRYPTED   = 1 << 7;
	public static final int V3_DEBUG_MEMORY             = 1 << 8;
	public static final int V3_DEBUG_SOCKET             = 1 << 9;
	public static final int V3_DEBUG_NOTICE             = 1 << 10;
	public static final int V3_DEBUG_INFO               = 1 << 11;
	public static final int V3_DEBUG_MUTEX              = 1 << 12;
	public static final int V3_DEBUG_EVENT              = 1 << 13;
	public static final int V3_DEBUG_ALL                = 65535;
}
