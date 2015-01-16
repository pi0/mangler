/*
 * Copyright 2010-2011 Daniel Sloof <daniel@danslo.org>
 *
 * This file is part of Mangler.
 *
 * $LastChangedDate: 2011-06-30 02:59:47 +0430 (Thu, 30 Jun 2011) $
 * $Revision: 1143 $
 * $LastChangedBy: econnell $
 * $URL: http://svn.mangler.org/mangler/trunk/android/src/org/mangler/android/ChannelListEntity.java $
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

import java.io.UnsupportedEncodingException;
import java.util.ArrayList;
import java.util.HashMap;

public class ChannelListEntity {
	
	static final int USER = 1;
	static final int CHANNEL = 2;
	
	public short id;
	public String name = "";
	public String phonetic = "";
	public String comment = "";
	public String url = "";
	public String indent = "";
	public int type = -1;
	public int passwordProtected = 0;
	public short parentid = 0;
	public int xmitStatus = R.drawable.xmit_off;
	public short realUserId = 0;
	
	public ChannelListEntity(HashMap<String, Object> entity) {
		type = Integer.parseInt(entity.get("type").toString());
		id = Short.parseShort(entity.get("id").toString());
		name = entity.get("name").toString();
		phonetic = entity.get("phonetic").toString();
		url = entity.get("url").toString();
		comment = entity.get("comment").toString();
		indent = entity.get("indent").toString();
		realUserId = Short.parseShort(entity.get("realUserId").toString());
		passwordProtected = Integer.parseInt(entity.get("passwordProtected").toString());
		parentid = Short.parseShort(entity.get("parentid").toString());
		xmitStatus = Integer.parseInt(entity.get("xmitStatus").toString());
	}
	
	public ChannelListEntity(int type, short id) {
		VentriloEventData data = new VentriloEventData();
		switch (type) {
			case USER:
				VentriloInterface.getuser(data, id);
				this.type = type;
				this.id = id;
				name = stringFromBytes(data.text.name);
				phonetic = stringFromBytes(data.text.phonetic);
				comment = stringFromBytes(data.text.comment);
				url = stringFromBytes(data.text.url);
				parentid = VentriloInterface.getuserchannel(id);
				realUserId = data.text.real_user_id;
				break;
			case CHANNEL:
				VentriloInterface.getchannel(data, id);
				this.type = type;
				this.id = id;
				name = stringFromBytes(data.text.name);
				comment = stringFromBytes(data.text.comment);
				parentid = data.data.channel.parent;
				passwordProtected = VentriloInterface.channelrequirespassword(id);
				break;
		}
	}
	
	public ChannelListEntity(ArrayList<String> a) {
		type = Integer.parseInt(a.get(0));
		id = Short.parseShort(a.get(1));
		name = a.get(2);
		phonetic = a.get(3);
		url = a.get(4);
		comment = a.get(5);
		indent = a.get(6);
		passwordProtected = Integer.parseInt(a.get(7));
		parentid = Short.parseShort(a.get(8));
		xmitStatus = Integer.parseInt(a.get(9));
		realUserId = Short.parseShort(a.get(10));
	}
	
	public ChannelListEntity() {
	}
	
	public static String stringFromBytes(byte[] bytes) {
		try {
			return new String(bytes, 0, (new String(bytes).indexOf(0)), ServerList.characterEncoding);
		} catch (UnsupportedEncodingException e) {
			e.printStackTrace();
			return new String(bytes, 0, (new String(bytes).indexOf(0)));
		}
	}

	public HashMap<String, Object> toHashMap() {
		HashMap<String, Object> entity = new HashMap<String, Object>();
		entity.put("type", type);
		entity.put("id", id);
		entity.put("name", name);
		entity.put("phonetic", phonetic);
		entity.put("comment", comment);
		entity.put("url", url);
		entity.put("indent", indent);
		entity.put("passwordProtected", passwordProtected);
		entity.put("parentid", parentid);
		entity.put("xmitStatus", xmitStatus);
		entity.put("realUserId", realUserId);
		return entity;
	}
	
	public ArrayList<String> serialize() {
		ArrayList<String> a = new ArrayList<String>();
		a.add(String.valueOf(type));
		a.add(String.valueOf(id));
		a.add(name);
		a.add(phonetic);
		a.add(url);
		a.add(comment);
		a.add(indent);
		a.add(String.valueOf(passwordProtected));
		a.add(String.valueOf(parentid));
		a.add(String.valueOf(xmitStatus));
		a.add(String.valueOf(realUserId));
		return a;
	}
	
	public boolean inMyChannel() {
		short channelid = VentriloInterface.getuserchannel(VentriloInterface.getuserid());
		if (parentid == channelid) {
			return true;
		}
		return false;
	}
}
