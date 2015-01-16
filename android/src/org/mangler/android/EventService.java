/*
 * Copyright 2010-2011 Daniel Sloof <daniel@danslo.org>
 *
 * This file is part of Mangler.
 *
 * $LastChangedDate: 2011-07-13 09:50:43 +0430 (Wed, 13 Jul 2011) $
 * $Revision: 1145 $
 * $LastChangedBy: econnell $
 * $URL: http://svn.mangler.org/mangler/trunk/android/src/org/mangler/android/EventService.java $
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
import java.util.concurrent.ConcurrentLinkedQueue;

import android.app.Service;
import android.content.Intent;
import android.os.Binder;
import android.os.IBinder;

public class EventService extends Service {

	static {
		// Let's try loading the library here too... maybe this will fix the
		// unsatisfied link problems?
		System.loadLibrary("ventrilo_interface");
	}
	
	private final IBinder binder = new EventBinder();
	private boolean running = false;
	private static ConcurrentLinkedQueue<VentriloEventData> eventQueue;

	public class EventBinder extends Binder {
		EventService getService() {
			return EventService.this;
		}
	}

	@Override
	public IBinder onBind(Intent intent) {
		return binder;
	}

	@Override
	public void onCreate() {
		eventQueue = new ConcurrentLinkedQueue<VentriloEventData>();
		running = true;
		Thread t = new Thread(eventRunnable);
		t.start();
	}

	@Override
	public void onDestroy() {
		running = false;
		eventQueue = null;
	}

	public static String StringFromBytes(byte[] bytes) {
		return new String(bytes, 0, (new String(bytes).indexOf(0)));
	}

	private Runnable eventRunnable = new Runnable() {

		public void run() {
			boolean forwardToUI = true;
			final int INIT = 0;
			final int ON = 1;
			final int OFF = 2;
			HashMap<Short, Integer> talkState = new HashMap<Short, Integer>();
			while (running) {
				forwardToUI = true;

				VentriloEventData data = new VentriloEventData();
				VentriloInterface.getevent(data);

				// Process audio packets here and let everything else queue up
				// for the UI thread
				switch (data.type) {
					case VentriloEvents.V3_EVENT_USER_LOGOUT:
						Player.close(data.user.id);
						talkState.put(data.user.id, OFF);
						break;

					case VentriloEvents.V3_EVENT_LOGIN_COMPLETE:
						Recorder.rate(VentriloInterface.getchannelrate(VentriloInterface.getuserchannel(VentriloInterface.getuserid())));
						break;

					case VentriloEvents.V3_EVENT_USER_TALK_START:
						talkState.put(data.user.id, INIT);
						break;

					case VentriloEvents.V3_EVENT_PLAY_AUDIO:
						Player.write(data.user.id, data.pcm.rate, data.pcm.channels, data.data.sample, data.pcm.length);
						// Only forward the first play audio event to the UI
						if (talkState.get(data.user.id) != ON) {
							talkState.put(data.user.id, ON);
						} else {
						forwardToUI = false;
						}
						break;

					case VentriloEvents.V3_EVENT_USER_TALK_END:
					case VentriloEvents.V3_EVENT_USER_TALK_MUTE:
					case VentriloEvents.V3_EVENT_USER_GLOBAL_MUTE_CHANGED:
					case VentriloEvents.V3_EVENT_USER_CHANNEL_MUTE_CHANGED:
						Player.close(data.user.id);
						talkState.put(data.user.id, OFF);
						break;

					case VentriloEvents.V3_EVENT_USER_CHAN_MOVE:
						if (data.user.id == VentriloInterface.getuserid()) {
							Player.clear();
							Recorder.rate(VentriloInterface.getchannelrate(data.channel.id));
							talkState.put(data.user.id, OFF);
						} else {
							Player.close(data.user.id);
							talkState.put(data.user.id, OFF);
						}
						break;
				}
				if (forwardToUI) {
					// In order to conserve memory, let the consumer catch up
					// before putting too many objects in the event queue
					while (eventQueue.size() > 25) {
						try {
							this.wait(10);
						} catch (Exception e) {
						}
					}
					eventQueue.add(data);
					sendBroadcast(new Intent(ServerView.EVENT_ACTION));
				}
			}
			Player.clear();
			Recorder.stop();
		}
	};

	public static VentriloEventData getNext() {
		if (eventQueue == null) {
			return null;
		}
		return eventQueue.poll();
	}
	
	public static void clearEvents() {
		eventQueue.clear();
	}
}
