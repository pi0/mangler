/*
 * Copyright 2010 Daniel Sloof <daniel@danslo.org>
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

#include <jni.h>
#include <stdint.h>
#include "ventrilo3.h"

inline jclass get_class(JNIEnv* env, jobject obj) {
	return (*env)->GetObjectClass(env, obj);
}

inline jobject get_object(JNIEnv* env, jobject parent_obj, jclass parent_cls, char* name, char* sig) {
	return (*env)->GetObjectField(env, parent_obj, (*env)->GetFieldID(env, parent_cls, name, sig));
}

inline char* get_string(JNIEnv* env, jstring str) {
	return (char*)(*env)->GetStringUTFChars(env, str, 0);
}

inline jbyte* get_byte_array(JNIEnv* env, jbyteArray pcm) {
	jboolean isCopy;
	return (*env)->GetByteArrayElements(env, pcm, &isCopy);
}

inline void release_byte_array(JNIEnv* env, jbyteArray pcm, jbyte* data) {
	(*env)->ReleaseByteArrayElements(env, pcm, data, 0);	
}

inline void release_string(JNIEnv* env, jstring str, char* _str) {
	(*env)->ReleaseStringUTFChars(env, str, _str);
}

inline void set_short(JNIEnv* env, jobject parent_obj, jclass parent_cls, char *name, jshort val) {
	(*env)->SetShortField(env, parent_obj, (*env)->GetFieldID(env, parent_cls, name, "S"), val);	
}

inline void set_byte(JNIEnv* env, jobject parent_obj, jclass parent_cls, char *name, jbyte val) {
	(*env)->SetByteField(env, parent_obj, (*env)->GetFieldID(env, parent_cls, name, "B"), val);	
}

inline void set_int(JNIEnv* env, jobject parent_obj, jclass parent_cls, char *name, jint val) {
	(*env)->SetIntField(env, parent_obj, (*env)->GetFieldID(env, parent_cls, name, "I"), val);	
}

inline void set_bool(JNIEnv* env, jobject parent_obj, jclass parent_cls, char *name, jboolean val) {
	(*env)->SetBooleanField(env, parent_obj, (*env)->GetFieldID(env, parent_cls, name, "Z"), val);	
}

inline void set_byte_array(JNIEnv* env, jobject parent_obj, jclass parent_cls, char* name, jbyteArray data, jint sz) {
	(*env)->SetByteArrayRegion(env, (*env)->GetObjectField(env, parent_obj, (*env)->GetFieldID(env, parent_cls, name, "[B")), 0, sz, data);	
}

JNIEXPORT jboolean JNICALL Java_org_mangler_android_VentriloInterface_recv() {
	_v3_net_message *msg;
	if ((msg = _v3_recv(V3_BLOCK))) {
		_v3_process_message(msg);
	}
	return msg && v3_is_loggedin();
}

JNIEXPORT jint JNICALL Java_org_mangler_android_VentriloInterface_pcmlengthforrate(JNIEnv* env, jobject obj, jint rate) {
	return v3_pcmlength_for_rate(rate);
}

JNIEXPORT void JNICALL Java_org_mangler_android_VentriloInterface_joinchat() {
	v3_join_chat();
}

JNIEXPORT void JNICALL Java_org_mangler_android_VentriloInterface_leavechat() {
	v3_leave_chat();
}

JNIEXPORT void JNICALL Java_org_mangler_android_VentriloInterface_logout() {
	v3_logout();
}

JNIEXPORT void JNICALL Java_org_mangler_android_VentriloInterface_phantomadd(JNIEnv* env, jobject obj, jchar channelid) {
	v3_phantom_add(channelid);
}

JNIEXPORT void JNICALL Java_org_mangler_android_VentriloInterface_phantomremove(JNIEnv* env, jobject obj, jchar channelid) {
	v3_phantom_remove(channelid);
}

JNIEXPORT jboolean JNICALL Java_org_mangler_android_VentriloInterface_isloggedin() {
	return v3_is_loggedin() != 0;
}

JNIEXPORT jchar JNICALL Java_org_mangler_android_VentriloInterface_getuserid() {
	return v3_get_user_id();
}

JNIEXPORT jboolean JNICALL Java_org_mangler_android_VentriloInterface_messagewaiting(JNIEnv* env, jobject obj, jboolean block) {
	return v3_message_waiting(block) != 0;
}

JNIEXPORT jint JNICALL Java_org_mangler_android_VentriloInterface_getmaxclients() {
	return v3_get_max_clients();
}

JNIEXPORT void JNICALL Java_org_mangler_android_VentriloInterface_clearevents() {
	v3_clear_events();
}

JNIEXPORT jint JNICALL Java_org_mangler_android_VentriloInterface_getcodecrate(JNIEnv* env, jobject obj, jchar codec, jchar format) {
	return v3_get_codec_rate(codec, format);
}

JNIEXPORT jchar JNICALL Java_org_mangler_android_VentriloInterface_getuserchannel(JNIEnv* env, jobject obj, jchar id) {
	return v3_get_user_channel(id);
}

JNIEXPORT jint JNICALL Java_org_mangler_android_VentriloInterface_channelrequirespassword(JNIEnv* env, jobject obj, jchar channelid) {
	return v3_channel_requires_password(channelid);
}

JNIEXPORT void JNICALL Java_org_mangler_android_VentriloInterface_startaudio(JNIEnv* env, jobject obj, jchar sendtype) {
	v3_start_audio(sendtype);
}

JNIEXPORT void JNICALL Java_org_mangler_android_VentriloInterface_stopaudio() {
	v3_stop_audio();
}

JNIEXPORT void JNICALL Java_org_mangler_android_VentriloInterface_setuservolume(JNIEnv* env, jobject obj, jshort id, jint level) {
	v3_set_volume_user(id, level);
}

JNIEXPORT void JNICALL Java_org_mangler_android_VentriloInterface_setxmitvolume(JNIEnv* env, jobject obj, jint level) {
	v3_set_volume_xmit(level);
}

JNIEXPORT jint JNICALL Java_org_mangler_android_VentriloInterface_usercount() {
	return v3_user_count();
}

JNIEXPORT jint JNICALL Java_org_mangler_android_VentriloInterface_channelcount() {
	return v3_channel_count();
}

JNIEXPORT int JNICALL Java_org_mangler_android_VentriloInterface_debuglevel(JNIEnv* env, jobject obj, jint level) {
	return v3_debuglevel(level);
}

JNIEXPORT int JNICALL Java_org_mangler_android_VentriloInterface_getchannelrate(JNIEnv* env, jobject obj, jchar channelid) {
	return v3_get_channel_codec(channelid)->rate;
}

JNIEXPORT jboolean JNICALL Java_org_mangler_android_VentriloInterface_getpermission(JNIEnv* env, jobject obj, jstring permname) {
	const v3_permissions *perms = v3_get_permissions();
	char *_permname = get_string(env, permname);
	int permval = 0;
	if (strcmp(_permname, "kickuser") == 0) {
		permval = perms->kick_user; 
	}
	if (strcmp(_permname, "banuser") == 0) {
		permval =  perms->ban_user; 
	}
	if (strcmp(_permname, "sendpage") == 0) {
		permval =  perms->send_page; 
	}
	if (strcmp(_permname, "addphantom") == 0) {
		permval =  perms->add_phantom; 
	}
	if (strcmp(_permname, "serveradmin") == 0) {
		permval =  perms->srv_admin; 
	}
	if (strcmp(_permname, "moveuser") == 0) {
		permval =  perms->move_user; 
	}
	release_string(env, permname, _permname);
	return permval;
}

JNIEXPORT void  JNICALL Java_org_mangler_android_VentriloInterface_forcechannelmove(JNIEnv* env, jobject obj, jshort userid, jshort channelid) {
	v3_force_channel_move(userid, channelid);
	return;
}

JNIEXPORT void  JNICALL Java_org_mangler_android_VentriloInterface_sendpage(JNIEnv* env, jobject obj, jshort userid) {
	v3_send_user_page(userid);
	return;
}

JNIEXPORT void  JNICALL Java_org_mangler_android_VentriloInterface_globalmute(JNIEnv* env, jobject obj, jshort userid) {
	v3_admin_global_mute(userid);
	return;
}

JNIEXPORT void JNICALL Java_org_mangler_android_VentriloInterface_kick(JNIEnv* env, jobject obj, jshort userid, jstring reason) {
	char *_reason = get_string(env, reason);
	v3_admin_boot(V3_BOOT_KICK, userid, _reason);
	return;
}

JNIEXPORT void JNICALL Java_org_mangler_android_VentriloInterface_ban(JNIEnv* env, jobject obj, jshort userid, jstring reason) {
	char *_reason = get_string(env, reason);
	v3_admin_boot(V3_BOOT_BAN, userid, _reason);
	return;
}


JNIEXPORT jboolean JNICALL Java_org_mangler_android_VentriloInterface_login(JNIEnv* env, jobject obj, jstring server, jstring username, jstring password, jstring phonetic) {
	char* _server = get_string(env, server);
	char* _username = get_string(env, username);
	char* _password = get_string(env, password);
	char* _phonetic = get_string(env, phonetic);
	v3_get_event(V3_NONBLOCK);
	v3_set_server_opts(V3_USER_ACCEPT_PAGES, 1);
	v3_set_server_opts(V3_USER_ACCEPT_U2U,   1);
	v3_set_server_opts(V3_USER_ACCEPT_CHAT,  0);
	v3_set_server_opts(V3_USER_ALLOW_RECORD, 1);
	jint ret = v3_login(_server, _username, _password, _phonetic);
	release_string(env, server, _server);
	release_string(env, username, _username);
	release_string(env, password, _password);
	release_string(env, phonetic, _phonetic);
	return ret;
}

JNIEXPORT void JNICALL Java_org_mangler_android_VentriloInterface_sendchatmessage(JNIEnv* env, jobject obj, jstring message) {
	char* _message = get_string(env, message);
	v3_send_chat_message(_message);
	release_string(env, message, _message);
}

JNIEXPORT void JNICALL Java_org_mangler_android_VentriloInterface_adminlogin(JNIEnv* env, jobject obj, jstring password) {
	char* _password = get_string(env, password);
	v3_admin_login(_password);
	release_string(env, password, _password);
}

JNIEXPORT void JNICALL Java_org_mangler_android_VentriloInterface_changechannel(JNIEnv* env, jobject obj, jchar channelid, jstring password) {
	char* _password = get_string(env, password);
	v3_change_channel(channelid, _password);
	release_string(env, password, _password);
}

JNIEXPORT void JNICALL Java_org_mangler_android_VentriloInterface_settext(JNIEnv* env, jobject obj, jstring comment, jstring url, jstring integrationtext, jboolean silent) {
	char* _comment = get_string(env, comment);
	char* _url = get_string(env, url);
	char* _integrationtext = get_string(env, integrationtext);
	v3_set_text(_comment, _url, _integrationtext, silent);
	release_string(env, comment, _comment);
	release_string(env, url, _url);
	release_string(env, integrationtext, _integrationtext);
}

JNIEXPORT void JNICALL Java_org_mangler_android_VentriloInterface_sendaudio(JNIEnv* env, jobject obj, jbyteArray pcm, jint size, jint rate) {
	jbyte *data = get_byte_array(env, pcm);
	v3_send_audio(V3_AUDIO_SENDTYPE_U2CCUR, rate, data, size, 0);
	release_byte_array(env, pcm, data);
}

JNIEXPORT void JNICALL Java_org_mangler_android_VentriloInterface_error(JNIEnv* env, jobject obj, jobject eventdata) {
	jclass  event_class = get_class(env, eventdata);
	jobject error = get_object(env, eventdata, event_class, "error", "Lorg/mangler/android/VentriloEventData$_error;");
	jclass  error_class = get_class(env, error);
	set_byte_array(env, error, error_class, "message", _v3_error(NULL), 512);
}

JNIEXPORT void JNICALL Java_org_mangler_android_VentriloInterface_getuser(JNIEnv* env, jobject obj, jobject eventdata, jshort userid) {
	v3_user *u = v3_get_user(userid);
	if(u) {
		jclass  event_class = get_class(env, eventdata);
		jobject text = get_object(env, eventdata, event_class, "text", "Lorg/mangler/android/VentriloEventData$_text;");
		jclass  text_class = get_class(env, text);
		set_byte_array(env, text, text_class, "name", u->name, 32);
		set_byte_array(env, text, text_class, "phonetic", u->phonetic, 32);
		set_byte_array(env, text, text_class, "comment", u->comment, 128);
		set_byte_array(env, text, text_class, "url", u->url, 128);
		set_byte_array(env, text, text_class, "integration_text", u->integration_text, 128);
		set_short(env, text, text_class, "real_user_id", u->real_user_id);
		v3_free_user(u);
	}
}

JNIEXPORT void JNICALL Java_org_mangler_android_VentriloInterface_getchannel(JNIEnv* env, jobject obj, jobject eventdata, jshort channelid) {
	v3_channel *c = v3_get_channel(channelid);
	if(c) {
		jclass  event_class = get_class(env, eventdata);
		jobject data = get_object(env, eventdata, event_class, "data", "Lorg/mangler/android/VentriloEventData$_data;");
		jclass  data_class = get_class(env, data);
		jobject channel = get_object(env, data, data_class, "channel", "Lorg/mangler/android/VentriloEventData$_data$_channel;");
		jclass  channel_class = get_class(env, channel);
		jobject text = get_object(env, eventdata, event_class, "text", "Lorg/mangler/android/VentriloEventData$_text;");
		jclass  text_class = get_class(env, text);
		set_short(env, channel, channel_class, "parent", c->parent);
		set_short(env, channel, channel_class, "channel_codec", c->channel_codec);
		set_short(env, channel, channel_class, "channel_format",c->channel_format);
		set_bool (env, channel, channel_class, "password_protected", c->password_protected != 0);
		set_byte_array(env, text, text_class, "name", c->name, 32);
		set_byte_array(env, text, text_class, "phonetic", c->phonetic, 32);
		set_byte_array(env, text, text_class, "comment", c->comment, 128);
		v3_free_channel(c);
	}
}

JNIEXPORT void JNICALL Java_org_mangler_android_VentriloInterface_getevent(JNIEnv* env, jobject obj, jobject eventdata) {
	v3_event *ev = v3_get_event(V3_BLOCK);
	if(ev != NULL) {
		jclass event_class = get_class(env, eventdata);
		set_short(env, eventdata, event_class, "type", ev->type);
		switch(ev->type) {
			case V3_EVENT_PLAY_AUDIO:
				{
					// PCM data.
					jobject pcm = get_object(env, eventdata, event_class, "pcm", "Lorg/mangler/android/VentriloEventData$_pcm;");
					jclass	pcm_class = get_class(env, pcm);
					set_int	 (env, pcm, pcm_class, "length", ev->pcm.length);
					set_int	 (env, pcm, pcm_class, "rate", ev->pcm.rate);
					set_short(env, pcm, pcm_class, "send_type", ev->pcm.send_type);
					set_byte (env, pcm, pcm_class, "channels", ev->pcm.channels);
					
					// User ID.
					jobject user = get_object(env, eventdata, event_class, "user", "Lorg/mangler/android/VentriloEventData$_user;");
					jclass  user_class = get_class(env, user);
					set_short(env, user, user_class, "id", ev->user.id);
					
					// Sample.
					jobject data = get_object(env, eventdata, event_class, "data", "Lorg/mangler/android/VentriloEventData$_data;");
					jclass  data_class = get_class(env, data);
					set_byte_array(env, data, data_class, "sample", ev->data->sample, ev->pcm.length);
				}
				break;
				
			case V3_EVENT_PING:
				{
					// Ping.
					set_int(env, eventdata, event_class, "ping", ev->ping);
				}
				break;
				
			case V3_EVENT_USER_TALK_END:
			case V3_EVENT_USER_TALK_MUTE:
			case V3_EVENT_CHAT_JOIN:
			case V3_EVENT_CHAT_LEAVE:
			case V3_EVENT_PRIVATE_CHAT_START:
			case V3_EVENT_PRIVATE_CHAT_END:
			case V3_EVENT_PRIVATE_CHAT_AWAY:
			case V3_EVENT_PRIVATE_CHAT_BACK:
			case V3_EVENT_USER_GLOBAL_MUTE_CHANGED:
			case V3_EVENT_USER_CHANNEL_MUTE_CHANGED:
			case V3_EVENT_USER_RANK_CHANGE:
				{
					// User ID.
					jobject user = get_object(env, eventdata, event_class, "user", "Lorg/mangler/android/VentriloEventData$_user;");
					jclass  user_class = get_class(env, user);
					set_short(env, user, user_class, "id", ev->user.id);
				}
				break;
				
			case V3_EVENT_USER_CHAN_MOVE:
			case V3_EVENT_USER_LOGOUT:
			case V3_EVENT_CHAN_REMOVE:
				{
					// User ID.
					jobject user = get_object(env, eventdata, event_class, "user", "Lorg/mangler/android/VentriloEventData$_user;");
					jclass  user_class = get_class(env, user);
					set_short(env, user, user_class, "id", ev->user.id);
					
					// Channel ID.
					jobject channel = get_object(env, eventdata, event_class, "channel", "Lorg/mangler/android/VentriloEventData$_channel;");
					jclass  channel_class = get_class(env, channel);
					set_short(env, channel, channel_class, "id", ev->channel.id);
				}
				break;
			
			case V3_EVENT_USER_MODIFY:
			case V3_EVENT_USER_LOGIN:
				{
					// User ID.
					jobject user = get_object(env, eventdata, event_class, "user", "Lorg/mangler/android/VentriloEventData$_user;");
					jclass  user_class = get_class(env, user);
					set_short(env, user, user_class, "id", ev->user.id);
					
					// Channel ID.
					jobject channel = get_object(env, eventdata, event_class, "channel", "Lorg/mangler/android/VentriloEventData$_channel;");
					jclass  channel_class = get_class(env, channel);
					set_short(env, channel, channel_class, "id", ev->channel.id);
					
					// Flags.
					set_int(env, eventdata, event_class, "flags", ev->flags);
				}
				break;
				
			case V3_EVENT_STATUS:
				{
					// Status message & percentage.
					jobject status = get_object(env, eventdata, event_class, "status", "Lorg/mangler/android/VentriloEventData$_status;");
					jclass  status_class = get_class(env, status);
					set_byte(env, status, status_class, "percent", ev->status.percent);
					set_byte_array(env, status, status_class, "message", ev->status.message, sizeof(ev->status.message));
				}
				break;
				
			case V3_EVENT_LOGIN_COMPLETE:
			case V3_EVENT_LOGIN_FAIL:
			case V3_EVENT_DISCONNECT:
			case V3_EVENT_ADMIN_AUTH:
			case V3_EVENT_CHAN_ADMIN_UPDATED:
			case V3_EVENT_PERMS_UPDATED:
				{
					// No event data for these types!
				}
				break;

			case V3_EVENT_PRIVATE_CHAT_MESSAGE:
				{
					// User IDs.
					jobject user = get_object(env, eventdata, event_class, "user", "Lorg/mangler/android/VentriloEventData$_user;");
					jclass  user_class = get_class(env, user);
					set_short(env, user, user_class, "privchat_user1", ev->user.privchat_user1);
					set_short(env, user, user_class, "privchat_user2", ev->user.privchat_user2);
					
					// Chat message.
					jobject data = get_object(env, eventdata, event_class, "data", "Lorg/mangler/android/VentriloEventData$_data;");
					jclass  data_class = get_class(env, data);
					set_byte_array(env, data, data_class, "chatmessage", ev->data->chatmessage, sizeof(ev->data->chatmessage));
					
					// Flags.
					set_int(env, eventdata, event_class, "flags", ev->flags);
				}
				break;
				
			case V3_EVENT_USERLIST_ADD:
			case V3_EVENT_USERLIST_MODIFY:
				{
					// Account fields.
					jobject data = get_object(env, eventdata, event_class, "data", "Lorg/mangler/android/VentriloEventData$_data;");
					jclass  data_class = get_class(env, data);
					jobject account = get_object(env, data, data_class, "account", "Lorg/mangler/android/VentriloEventData$_data$_account;");
					jclass  account_class = get_class(env, account);
					set_byte_array(env, account, account_class, "username", ev->data->account.username, sizeof(ev->data->account.username));
				}
				break;
			
			case V3_EVENT_CHAN_ADD:
			case V3_EVENT_CHAN_MODIFY:
				{
					// Channel id.
					jobject channel = get_object(env, eventdata, event_class, "channel", "Lorg/mangler/android/VentriloEventData$_channel;");
					jclass  channel_class = get_class(env, channel);
					set_short(env, channel, channel_class, "id", ev->channel.id);
				}
				break;
				
			case V3_EVENT_DISPLAY_MOTD:
				{
					/* don't deal with any MOTD events here because they're useless
					// MOTD.
					jobject data = get_object(env, eventdata, event_class, "data", "Lorg/mangler/android/VentriloEventData$_data;");
					jclass  data_class = get_class(env, data);
					set_byte_array(env, data, data_class, "motd", ev->data->motd, sizeof(ev->data->motd));
					
					// Flags.
					set_int(env, eventdata, event_class, "flags", ev->flags);
					*/
				}
				break;
				
			case V3_EVENT_CHAN_BADPASS:
				{
					// Channel ID.
					jobject channel = get_object(env, eventdata, event_class, "channel", "Lorg/mangler/android/VentriloEventData$_channel;");
					jclass  channel_class = get_class(env, channel);
					set_short(env, channel, channel_class, "id", ev->channel.id);
					
					// Error message.
					jobject error = get_object(env, eventdata, event_class, "error", "Lorg/mangler/android/VentriloEventData$_error;");
					jclass  error_class = get_class(env, error);
					set_byte_array(env, error, error_class, "message", ev->error.message, sizeof(ev->error.message));
				}
				break;
				
				
			case V3_EVENT_ERROR_MSG:
				{
					// Error message & disconnect flag.
					jobject error = get_object(env, eventdata, event_class, "error", "Lorg/mangler/android/VentriloEventData$_error;");
					jclass  error_class = get_class(env, error);
					set_byte_array(env, error, error_class, "message", ev->error.message, sizeof(ev->error.message));
					set_bool(env, error, error_class, "disconnected", ev->error.disconnected != 0);	
				}
				break;
				
			case V3_EVENT_USER_TALK_START:
				{
					// PCM data.
					jobject pcm = get_object(env, eventdata, event_class, "pcm", "Lorg/mangler/android/VentriloEventData$_pcm;");
					jclass  pcm_class = get_class(env, pcm);
					set_int  (env, pcm, pcm_class, "rate", ev->pcm.rate);
					set_short(env, pcm, pcm_class, "send_type", ev->pcm.send_type);
					
					// User ID.
					jobject user = get_object(env, eventdata, event_class, "user", "Lorg/mangler/android/VentriloEventData$_user;");
					jclass  user_class = get_class(env, user);
					set_short(env, user, user_class, "id", ev->user.id);
				}
				break;
				
			case V3_EVENT_CHAT_MESSAGE:
				{
					// User ID.
					jobject user = get_object(env, eventdata, event_class, "user", "Lorg/mangler/android/VentriloEventData$_user;");
					jclass  user_class = get_class(env, user);
					set_short(env, user,user_class, "id", ev->user.id);
					
					// Chat message.
					jobject data = get_object(env, eventdata, event_class, "data", "Lorg/mangler/android/VentriloEventData$_data;");
					jclass  data_class = get_class(env, data);
					set_byte_array(env, data, data_class, "chatmessage", ev->data->chatmessage, sizeof(ev->data->chatmessage));
				}
				break;
				
			case V3_EVENT_USERLIST_REMOVE:
				{
					// Account ID.
					jobject data = get_object(env, eventdata, event_class, "data", "Lorg/mangler/android/VentriloEventData$_data;");
					jclass  data_class = get_class(env, data);
					jobject account = get_object(env, data, data_class, "account", "Lorg/mangler/android/VentriloEventData$_data$_account;");
					jclass  account_class = get_class(env, account);
					set_short(env, account, account_class, "id", ev->account.id);
				}
				break;
			
			case V3_EVENT_USERLIST_CHANGE_OWNER:
				{
					// Account IDs.
					jobject data = get_object(env, eventdata, event_class, "data", "Lorg/mangler/android/VentriloEventData$_data;");
					jclass  data_class = get_class(env, data);
					jobject account = get_object(env, data, data_class, "account", "Lorg/mangler/android/VentriloEventData$_data$_account;");
					jclass  account_class = get_class(env, account);
					set_short(env, account, account_class, "id", ev->account.id);
					set_short(env, account, account_class, "id2", ev->account.id2);
				}
				break;
			case V3_EVENT_USER_PAGE:
				{
					// User ID.
					jobject user = get_object(env, eventdata, event_class, "user", "Lorg/mangler/android/VentriloEventData$_user;");
					jclass  user_class = get_class(env, user);
					set_short(env, user, user_class, "id", ev->user.id);
				}
				break;
		}
		v3_free_event(ev);
	}
}
