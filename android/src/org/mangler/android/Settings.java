/*
 * Copyright 2010-2011 Daniel Sloof <daniel@danslo.org>
 *
 * This file is part of Mangler.
 *
 * $LastChangedDate: 2011-07-13 09:50:43 +0430 (Wed, 13 Jul 2011) $
 * $Revision: 1145 $
 * $LastChangedBy: econnell $
 * $URL: http://svn.mangler.org/mangler/trunk/android/src/org/mangler/android/Settings.java $
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

import android.media.AudioManager;
import android.os.Bundle;
import android.preference.PreferenceActivity;
import android.preference.PreferenceManager;

public class Settings extends PreferenceActivity {
	@Override
	public void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		
		// Volume controls.
        setVolumeControlStream(AudioManager.STREAM_MUSIC);
        
		addPreferencesFromResource(R.xml.preferences);
	}
	
	@Override
	public void onResume() {
		super.onResume();
		setRequestedOrientation(Integer.parseInt(PreferenceManager.getDefaultSharedPreferences(getBaseContext()).getString("orientation", "4")));
	}
}
