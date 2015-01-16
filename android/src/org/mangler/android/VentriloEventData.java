/*
 * Copyright 2010 Daniel Sloof <daniel@danslo.org>
 *
 * This file is part of Mangler.
 *
 * $LastChangedDate: 2011-04-10 09:53:30 +0430 (Sun, 10 Apr 2011) $
 * $Revision: 1120 $
 * $LastChangedBy: econnell $
 * $URL: http://svn.mangler.org/mangler/trunk/android/src/org/mangler/android/VentriloEventData.java $
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

public class VentriloEventData {

	public short type;
	public int ping;
	public int flags;
	
	public class _status {
		public byte percent;
		public byte[] message = new byte[256];
	}
	_status status = new _status();
	
	public class _error {
		public short code;
		public boolean disconnected;
		public byte[] message = new byte[512];
	}
	_error error = new _error();
	
	public class _pcm {
		public int length;
		public int rate;
		public short send_type;
		public byte channels;
	};
	_pcm pcm = new _pcm();
	
	public class _user {
		public short id;
		public short privchat_user1;
		public short privchat_user2;
	}
	_user user = new _user();
	
	public class _channel {
		public short id;
	}
	_channel channel = new _channel();
	
	public class _account {
		public short id;
		public short id2;
	}
	_account account = new _account();
	
	public class _text {
        public byte[] name 				= new byte[32];
        public byte[] password 			= new byte[32];
        public byte[] phonetic 			= new byte[32];
        public byte[] comment 			= new byte[128];
        public byte[] url 				= new byte[128];
        public byte[] integration_text 	= new byte[128];
        public short real_user_id;
	}
	_text text = new _text();
	
	public class _serverproperty {
		public short property;
		public byte value;
	}
	_serverproperty serverproperty = new _serverproperty();
	
	public class _data {
		public class _account {
			public byte[] username 		= new byte[32];
			public byte[] owner 		= new byte[32];
			public byte[] notes 		= new byte[256];
			public byte[] lock_reason 	= new byte[128];
			public short[] chan_auth 	= new short[32];
			public short[] chan_admin 	= new short[32];
			public int chan_admin_count;
			public int chan_auth_count;
		}
		_account account = new _account();
		
		public class _channel {
			public short id;
			public short parent;
			public short channel_codec;
			public short channel_format;
			public boolean password_protected;
		}
		_channel channel = new _channel();
		
		public class _rank {
			public short id;
			public short level;
		};
		_rank rank = new _rank();
		
		public byte[] sample		= new byte[32768];
		//public byte[] motd			= new byte[32768];
		public byte[] chatmessage	= new byte[256];
		public byte[] reason		= new byte[128];
	}
	_data data = new _data();
	
}