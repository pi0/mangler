/*
 * Copyright 2010-2011 Daniel Sloof <daniel@danslo.org>
 *
 * This file is part of Mangler.
 *
 * $LastChangedDate: 2011-06-29 17:53:20 +0430 (Wed, 29 Jun 2011) $
 * $Revision: 1141 $
 * $LastChangedBy: econnell $
 * $URL: http://svn.mangler.org/mangler/trunk/android/src/org/mangler/android/TTSWrapper.java $
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

import java.lang.reflect.Constructor;

import android.content.Context;

public abstract class TTSWrapper {
	@SuppressWarnings("unchecked")
	public static TTSWrapper getInstance(Context context) {
		try {
			@SuppressWarnings("unused")
			Class tts = Class.forName("android.speech.tts.TextToSpeech");
			Class impl = Class.forName("org.mangler.android.TTSWrapperImpl");
			Constructor c = impl.getConstructor(Context.class);
			return (TTSWrapper) c.newInstance(context);
		} catch (Exception e) {
			return null;
		}
	}

	public abstract void shutdown();

	public abstract void speak(String message);
}