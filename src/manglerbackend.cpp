/*
 * vim: softtabstop=4 shiftwidth=4 cindent foldmethod=marker expandtab
 *
 * $LastChangedDate: 2011-06-27 23:50:10 +0430 (Mon, 27 Jun 2011) $
 * $Revision: 1139 $
 * $LastChangedBy: econnell $
 * $URL: http://svn.mangler.org/mangler/trunk/src/manglerbackend.cpp $
 *
 * Copyright 2009-2011 Eric Connell
 * Copyright 2010-2011 Roman Tetelman
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

#include "mangler.h"
#include "mangleraudio.h"
#include "manglerbackend.h"
#include "manglerpulse.h"
#include "mangleralsa.h"
#include "mangleross.h"

ManglerBackend* 
ManglerBackend::getBackend(Glib::ustring audioSubsystem, uint32_t rate, uint8_t channels, uint32_t pcm_framesize) {
#ifdef HAVE_PULSE
    if (audioSubsystem == "pulse") {
        return new ManglerPulse(rate, channels, pcm_framesize);
    }
#endif
#ifdef HAVE_ALSA
    if (audioSubsystem == "alsa") {
        return new ManglerAlsa(rate, channels, pcm_framesize);
    }
#endif
#ifdef HAVE_OSS
    if (audioSubsystem == "oss") {
        return new ManglerOSS(rate, channels, pcm_framesize);
    }
#endif
    if (audioSubsystem == "openal") {
      fprintf(stderr, "no mac users\n");
      return NULL;
    }
    fprintf(stderr, "unrecognized audio subsystem \"%s\"\n", audioSubsystem.c_str());
    return NULL;
}

void
ManglerBackend::getDeviceList(Glib::ustring audioSubsystem, std::vector<ManglerAudioDevice*>& input, std::vector<ManglerAudioDevice*>& output) {
#ifdef HAVE_PULSE
    if (audioSubsystem == "pulse") {
        ManglerPulse::getDeviceList(input, output);
    }
#endif
#ifdef HAVE_ALSA
    if (audioSubsystem == "alsa") {
        ManglerAlsa::getDeviceList(input, output);
    }
#endif
#ifdef HAVE_OSS
    if (audioSubsystem == "oss") {
        ManglerOSS::getDeviceList(input, output);
    }
#endif
}

ManglerBackend::~ManglerBackend() {
}

