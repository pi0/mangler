/*
 * Copyright 2010-2011 Daniel Sloof <daniel@danslo.org>
 *
 * This file is part of Mangler.
 *
 * $LastChangedDate: 2011-06-29 17:53:20 +0430 (Wed, 29 Jun 2011) $
 * $Revision: 1141 $
 * $LastChangedBy: econnell $
 * $URL: http://svn.mangler.org/mangler/trunk/android/src/org/mangler/android/ChannelList.java $
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

public class ChannelList {

	public static ArrayList<HashMap<String, Object>> data = new ArrayList<HashMap<String, Object>>();
	
	public static void add(ChannelListEntity entity) {
		String indent = "";
		if (entity.id == 0) {
			entity.parentid = -1;
		}
		for (int ctr = 0; ctr < getDepth(entity.parentid)+1 && entity.id != 0; ctr++) {
			indent = "    " + indent;
		}
		if (entity.type == ChannelListEntity.USER) {
			entity.xmitStatus = R.drawable.xmit_off;
		} else {
			entity.xmitStatus = R.drawable.xmit_clear;
		}
		entity.indent = indent;
		if (entity.type == ChannelListEntity.USER && entity.realUserId != 0) {
			entity.name = "[P] " + entity.name;
		}
		//Log.d("mangler", "adding entity id " + entity.id + " (" + entity.name + ") at location " + getLocation(entity.parentid));
		if (entity.id == 0) {
			entity.type = ChannelListEntity.CHANNEL;
			data.add(entity.toHashMap());
		} else {
			int location;
			if (entity.type == ChannelListEntity.USER) {
				location = getLocation(entity.parentid) + 1;
			} else {
				location = getLocation(entity.parentid) + getChildCount(entity.parentid) + 1;
			}
			while(location > data.size()) {
				data.add(null);
			}
			data.add(location, entity.toHashMap());
		}
	}
	
	public static boolean isChannel(int position) {
		if (Integer.parseInt(data.get(position).get("type").toString()) == ChannelListEntity.USER) {
			return false;
		}
		return true;
	}
	
	private static int getDepth(short channelid) {
		int depth = 0;
		short p;
		while ((p = getParentId(channelid)) >= 0) {
			channelid = p;
			depth++;
		}
		return depth;
	}
	
	private static int getChildCount(short channelid) {
		int childcount = 0;
		for(ListIterator<HashMap<String, Object>> iterator = data.listIterator(); iterator.hasNext(); ) {
			if(((Short)iterator.next().get("parentid")).equals(channelid)) {	
				childcount++;
			}
		}
		return childcount;
	}
	
	public static int getLocation(short id) {
		int ctr = 0;
		for(ListIterator<HashMap<String, Object>> iterator = data.listIterator(); iterator.hasNext(); ) {
			short listid =  ((Short)iterator.next().get("id"));
			if(listid == id) {
				return ctr;
			}
			ctr++;
		}
		return ctr;
	}
	
	private static short getParentId(short channelid) {
		for(ListIterator<HashMap<String, Object>> iterator = data.listIterator(); iterator.hasNext(); ) {
			if(((Short)iterator.next().get("id")).equals(channelid)) {	
				return (short)Integer.parseInt(iterator.previous().get("parentid").toString());
			}
		}
		return -1;
	}
	
	public static HashMap<String, Object> get(int type, short id) {
		for(ListIterator<HashMap<String, Object>> iterator = data.listIterator(); iterator.hasNext(); ) {
			HashMap<String, Object> entity = iterator.next();
			if ((Integer)entity.get("type") == type && (Short)entity.get("id") == id) {
				return data.get(iterator.previousIndex());
			}
		}
		return null;
	}
	
	public static void remove(short id) {
		//Log.d("mangler", "removing entity with id " + id + " at location " + getLocation(id));
		data.remove(getLocation(id));
	}
	
	public static void clear() {
		data.clear();
	}
	
	public static void updateStatus(short id, int status) {
		for(Iterator<HashMap<String, Object>> iterator = data.iterator(); iterator.hasNext(); ) {
			HashMap<String, Object> data = iterator.next();
			if((Short)data.get("id") == id) {
				data.put("xmitStatus", status);
				return;
			}
		}
	}

}
