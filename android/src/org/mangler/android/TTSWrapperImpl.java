/*
 * Copyright 2010 Daniel Sloof <daniel@danslo.org>
 *
 * This file is part of Mangler.
 *
 * $LastChangedDate: 2010-07-27 22:17:53 +0430 (Tue, 27 Jul 2010) $
 * $Revision: 1038 $
 * $LastChangedBy: ekilfoil $
 * $URL: http://svn.mangler.org/mangler/trunk/android/src/org/mangler/android/TTSWrapperImpl.java $
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

import android.content.Context;
import android.speech.tts.TextToSpeech;

public class TTSWrapperImpl extends TTSWrapper{
	private static TextToSpeech ttsInstance = null;
	
	public TTSWrapperImpl(Context context) {
		ttsInstance = new TextToSpeech(context, null);
	}

	public void shutdown() {
		ttsInstance.shutdown();
	}

	public void speak(String message) {
		ttsInstance.speak(message, TextToSpeech.QUEUE_ADD, null);	
	}
}
