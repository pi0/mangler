/*
 * Copyright 2010 Daniel Sloof <daniel@danslo.org>
 *
 * This file is part of Mangler.
 *
 * $LastChangedDate: 2010-07-27 22:06:30 +0430 (Tue, 27 Jul 2010) $
 * $Revision: 1037 $
 * $LastChangedBy: ekilfoil $
 * $URL: http://svn.mangler.org/mangler/trunk/android/src/org/mangler/android/VentriloEvents.java $
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

public class VentriloEvents {
    public static final int V3_EVENT_STATUS 					= 1;
    public static final int V3_EVENT_PING 						= 2;
	public static final int V3_EVENT_USER_LOGIN 				= 3;
	public static final int V3_EVENT_USER_LOGOUT 				= 4;
	public static final int V3_EVENT_LOGIN_COMPLETE 			= 5;
	public static final int V3_EVENT_LOGIN_FAIL 				= 6;
	public static final int V3_EVENT_USER_CHAN_MOVE 			= 7;
	public static final int V3_EVENT_CHAN_MOVE					= 8;
	public static final int V3_EVENT_CHAN_ADD 					= 9;
	public static final int V3_EVENT_CHAN_MODIFY 				= 10;
	public static final int V3_EVENT_CHAN_REMOVE 				= 11;
	public static final int V3_EVENT_CHAN_BADPASS 				= 12;
	public static final int V3_EVENT_ERROR_MSG 					= 13;
	public static final int V3_EVENT_USER_TALK_START 			= 14;
	public static final int V3_EVENT_USER_TALK_END 				= 15;
	public static final int V3_EVENT_USER_TALK_MUTE				= 16;
	public static final int V3_EVENT_PLAY_AUDIO 				= 17;
	public static final int V3_EVENT_RECORD_UPDATE				= 18;
	public static final int V3_EVENT_DISPLAY_MOTD 				= 19;
	public static final int V3_EVENT_DISCONNECT 				= 20;
	public static final int V3_EVENT_USER_MODIFY 				= 21;
	public static final int V3_EVENT_CHAT_JOIN 					= 22;
	public static final int V3_EVENT_CHAT_LEAVE 				= 23;
	public static final int V3_EVENT_CHAT_MESSAGE 				= 24;
	public static final int V3_EVENT_ADMIN_AUTH 				= 25;
	public static final int V3_EVENT_CHAN_ADMIN_UPDATED 		= 26;
	public static final int V3_EVENT_PRIVATE_CHAT_MESSAGE 		= 27;
	public static final int V3_EVENT_PRIVATE_CHAT_START 		= 28;
	public static final int V3_EVENT_PRIVATE_CHAT_END 			= 29;
	public static final int V3_EVENT_PRIVATE_CHAT_AWAY 			= 30;
	public static final int V3_EVENT_PRIVATE_CHAT_BACK 			= 31;
	public static final int V3_EVENT_TEXT_TO_SPEECH_MESSAGE		= 32;
	public static final int V3_EVENT_PLAY_WAVE_FILE_MESSAGE		= 33;
	public static final int V3_EVENT_USERLIST_ADD 				= 34;
	public static final int V3_EVENT_USERLIST_MODIFY 			= 35;
	public static final int V3_EVENT_USERLIST_REMOVE 			= 36;
	public static final int V3_EVENT_USERLIST_CHANGE_OWNER 		= 37;
	public static final int V3_EVENT_USER_GLOBAL_MUTE_CHANGED 	= 38;
	public static final int V3_EVENT_USER_CHANNEL_MUTE_CHANGED 	= 39;
	public static final int V3_EVENT_PERMS_UPDATED 				= 40;
	public static final int V3_EVENT_USER_RANK_CHANGE 			= 41;
	public static final int V3_EVENT_SRV_PROP_RECV				= 42;
	public static final int V3_EVENT_SRV_PROP_SENT				= 43;
	public static final int V3_EVENT_ADMIN_BAN_LIST				= 44;

	public static final int V3_EVENT_USER_PAGE					= 63;
}