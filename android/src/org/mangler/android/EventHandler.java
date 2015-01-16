/*
 * Copyright 2010 Daniel Sloof <daniel@danslo.org>
 *
 * This file is part of Mangler.
 *
 * $LastChangedDate: 2010-11-24 17:13:36 +0330 (Wed, 24 Nov 2010) $
 * $Revision: 1108 $
 * $LastChangedBy: clearscreen $
 * $URL: http://svn.mangler.org/mangler/trunk/android/src/org/mangler/android/EventHandler.java $
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

import java.util.HashMap;

import android.util.Log;
import android.widget.Toast;

public class EventHandler {
	private ServerView sv;
	
	public EventHandler(ServerView sv) {
		this.sv = sv;
	}
	
	public void process() {
		
		VentriloEventData data;
		
		while ((data = EventService.getNext()) != null) {
			ChannelListEntity entity;
			//Log.d("mangler", "EventHandler: processing event type " + data.type);
			switch (data.type) {
				case VentriloEvents.V3_EVENT_ERROR_MSG:
					Toast.makeText(sv, ChannelListEntity.stringFromBytes(data.error.message), Toast.LENGTH_SHORT).show();
					break;
					
				case VentriloEvents.V3_EVENT_CHAT_MESSAGE:
					entity = new ChannelListEntity(ChannelListEntity.USER, data.user.id);
					sv.addChatMessage(entity.name, ChannelListEntity.stringFromBytes(data.data.chatmessage));
					break;
					
				case VentriloEvents.V3_EVENT_CHAT_JOIN:
					entity = new ChannelListEntity(ChannelListEntity.USER, data.user.id);
					sv.addChatUser(entity.name);
					break;
					
				case VentriloEvents.V3_EVENT_CHAT_LEAVE:
					entity = new ChannelListEntity(ChannelListEntity.USER, data.user.id);
					sv.removeChatUser(entity.name);
					break;
					
				case VentriloEvents.V3_EVENT_CHAN_ADD:
					entity = new ChannelListEntity(ChannelListEntity.CHANNEL, data.channel.id);
					ChannelList.add(entity);
					break;
					
				case VentriloEvents.V3_EVENT_USER_LOGIN:
					if (data.user.id != 0) {
						int flags = data.flags;
						entity = new ChannelListEntity(ChannelListEntity.USER, data.user.id);
						ChannelList.add(entity);
						if (entity.inMyChannel()) {
							UserList.addUser(entity);
						}
						sv.notifyAdaptersDataSetChanged();
						// user was added from userlist sent at login (existing user)
						// from lv3: #define V3_LOGIN_FLAGS_EXISTING (1 << 0)
						if ((flags & (1 << 0)) == 0) {
							sv.tts(sv.TTS_LOGIN, (entity.phonetic.length() > 0 ? entity.phonetic : entity.name) + " has logged in");
						}
						sv.setUserVolumeFromDatabase(entity);
					} else {
						// there should probably be a replace() function somewhere
						entity = new ChannelListEntity(ChannelListEntity.USER, (short)0);
						HashMap<String, Object> lobby = ChannelList.get(ChannelListEntity.CHANNEL, (short)0);
						lobby.put("name", entity.name);
						lobby.put("comment", entity.comment);
					}
					break;
					
				case VentriloEvents.V3_EVENT_USER_CHAN_MOVE:
					entity = new ChannelListEntity(ChannelListEntity.USER, data.user.id);
					if (entity.id == VentriloInterface.getuserid()) {
						UserList.populate(entity.parentid);
						UserList.addUser(entity);
						Player.clear();
						Recorder.rate(VentriloInterface.getchannelrate(data.channel.id));
						if (data.channel.id == 0) {
							Toast.makeText(sv, "Changed to Lobby", Toast.LENGTH_SHORT).show();
						} else {
							entity = new ChannelListEntity(ChannelListEntity.CHANNEL, data.channel.id);
							Toast.makeText(sv, "Changed to " + entity.name, Toast.LENGTH_SHORT).show();
						}
					} else {
						if (entity.inMyChannel()) {
							sv.tts(sv.TTS_CHANNEL, (entity.phonetic.length() > 0 ? entity.phonetic : entity.name) + " has joined the channel");
							UserList.addUser(entity);
						} else if (UserList.getChannel(data.user.id) == VentriloInterface.getuserchannel(VentriloInterface.getuserid())) {
							sv.tts(sv.TTS_CHANNEL, (entity.phonetic.length() > 0 ? entity.phonetic : entity.name) + " has left the channel");
							UserList.delUser(data.user.id);
						}
						Player.close(data.user.id);
					}
					entity = new ChannelListEntity(ChannelList.get(ChannelListEntity.USER, data.user.id));
					entity.parentid = data.channel.id;
					ChannelList.remove(entity.id);
					ChannelList.add(entity);
					sv.notifyAdaptersDataSetChanged();
					break;
					
				case VentriloEvents.V3_EVENT_USER_LOGOUT:
					entity = new ChannelListEntity(ChannelList.get(ChannelListEntity.USER, data.user.id));
					Player.close(data.user.id);
					UserList.delUser(data.user.id);
					ChannelList.remove(data.user.id);
					sv.tts(sv.TTS_LOGIN, (entity.phonetic.length() > 0 ? entity.phonetic : entity.name) + " has logged out");
					sv.notifyAdaptersDataSetChanged();
					break;
					
				case VentriloEvents.V3_EVENT_PLAY_AUDIO:
					UserList.updateStatus(data.user.id, R.drawable.xmit_on);
					ChannelList.updateStatus(data.user.id, R.drawable.xmit_on);
					sv.notifyAdaptersDataSetChanged();
					break;
					
				case VentriloEvents.V3_EVENT_USER_TALK_START:
					UserList.updateStatus(data.user.id, R.drawable.xmit_init);
					ChannelList.updateStatus(data.user.id, R.drawable.xmit_init);
					sv.notifyAdaptersDataSetChanged();
					break;
					
				case VentriloEvents.V3_EVENT_USER_TALK_END:
				case VentriloEvents.V3_EVENT_USER_TALK_MUTE:
				case VentriloEvents.V3_EVENT_USER_GLOBAL_MUTE_CHANGED:
				case VentriloEvents.V3_EVENT_USER_CHANNEL_MUTE_CHANGED:
					UserList.updateStatus(data.user.id, R.drawable.xmit_off);
					ChannelList.updateStatus(data.user.id, R.drawable.xmit_off);
					sv.notifyAdaptersDataSetChanged();
					break;
					
				case VentriloEvents.V3_EVENT_PING:
					sv.ping = data.ping;
					sv.setTitle();
					break;
					
				case VentriloEvents.V3_EVENT_USER_MODIFY:
					entity = new ChannelListEntity(ChannelListEntity.USER, data.user.id);
					ChannelList.remove(entity.id);
					ChannelList.add(entity);
					if (entity.inMyChannel()) {
						UserList.delUser(entity.id);
						UserList.addUser(entity);
					}
					sv.notifyAdaptersDataSetChanged();
					break;
					
				case VentriloEvents.V3_EVENT_USER_PAGE:
					entity = new ChannelListEntity(ChannelListEntity.USER, data.user.id);
					sv.tts(sv.TTS_PAGE, "You have been paged by " + (entity.phonetic.length() > 0 ? entity.phonetic : entity.name));
					break;
					
				case VentriloEvents.V3_EVENT_LOGIN_COMPLETE:
				case VentriloEvents.V3_EVENT_PERMS_UPDATED:
					if (VentriloInterface.getpermission("serveradmin")) {
						sv.setTitle();
						sv.setIsAdmin(true);
					} else {
						sv.setIsAdmin(false);
					}
					break;
					
				case VentriloEvents.V3_EVENT_DISCONNECT:
					EventService.clearEvents();
					sv.finish();
					break;
					
				default:
					Log.d("mangler", "Unhandled event type: " + Integer.toString(data.type));
					break;
			}
		}
	}
}
