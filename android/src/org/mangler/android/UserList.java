/*
 * Copyright 2010 Daniel Sloof <daniel@danslo.org>
 *
 * This file is part of Mangler.
 *
 * $LastChangedDate: 2012-05-26 06:05:03 +0430 (Sat, 26 May 2012) $
 * $Revision: 1171 $
 * $LastChangedBy: Haxar $
 * $URL: http://svn.mangler.org/mangler/trunk/android/src/org/mangler/android/UserList.java $
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

import java.util.ArrayList;
import java.util.HashMap;
import java.util.Iterator;
import java.util.ListIterator;

public class UserList {
	
	public static ArrayList<HashMap<String, Object>> data = new ArrayList<HashMap<String, Object>>();
	
	public static void delUser(short userid) {
		for (Iterator<HashMap<String, Object>> iterator = data.iterator(); iterator.hasNext();) {
			if (((Short) iterator.next().get("userid")).equals(userid)) {
				iterator.remove();
				return;
			}
		}
	}
	
	public static void clear() {
		data.clear();
	}
	
	public static void addUser(ChannelListEntity entity) {
			HashMap<String, Object> user = new HashMap<String, Object>();
			user.put("userstatus", R.drawable.xmit_off);
			user.put("userid", entity.id);
			user.put("username", entity.name);
			user.put("channelid", entity.parentid);
			user.put("comment", (entity.comment.length() > 0) ? "(" + ((entity.url.length() > 0) ? "U: " : "") + entity.comment + ")" : "");
			data.add(user);
	}
	
	public static void populate(short channelid) {
		clear();
		for(ListIterator<HashMap<String, Object>> iterator = ChannelList.data.listIterator(); iterator.hasNext(); ) {
			ChannelListEntity entity = new ChannelListEntity(iterator.next());
			if(entity.type == ChannelListEntity.USER && entity.parentid == channelid) {	
				addUser(entity);
			}
		}
	}
	
	public static void updateStatus(short userid, int status) {
		for (Iterator<HashMap<String, Object>> iterator = data.iterator(); iterator.hasNext();) {
			HashMap<String, Object> data = iterator.next();
			if ((Short) data.get("userid") == userid) {
				data.put("userstatus", status);
				return;
			}
		}
	}
	
	public static short getChannel(short userid) {
		for (Iterator<HashMap<String, Object>> iterator = data.iterator(); iterator.hasNext();) {
			HashMap<String, Object> data = iterator.next();
			if ((Short) data.get("userid") == userid) {
				short channelid = (Short) data.get("channelid");
				return (short) channelid;
			}
		}
		return -1;
	}
	
	public static int getChannelListLocation(short userid) {
		for (Iterator<HashMap<String, Object>> iterator = data.iterator(); iterator.hasNext();) {
			short id = (Short) iterator.next().get("userid");
			if (id == userid) {
				return ChannelList.getLocation(id);
			}
		}
		return 0;
	}
}
