/*
 * vim: softtabstop=4 shiftwidth=4 cindent foldmethod=marker expandtab
 *
 * $LastChangedDate: 2011-06-27 23:50:10 +0430 (Mon, 27 Jun 2011) $
 * $Revision: 1139 $
 * $LastChangedBy: econnell $
 * $URL: http://svn.mangler.org/mangler/trunk/src/mangleraudio.h $
 *
 * Copyright 2009-2011 Eric Connell
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

#ifndef _MANGLERAUDIO_H
#define _MANGLERAUDIO_H

#include "config.h"

#ifdef HAVE_OSS
# include <fcntl.h>
# include <sys/ioctl.h>
# include <sys/soundcard.h>
#endif
#ifdef HAVE_ESPEAK
# include <espeak/speak_lib.h>
#endif

#include "manglerbackend.h"

#include <stdlib.h>
#include <string.h>
#include <errno.h>

#define AUDIO_CONTROL 0
#define AUDIO_INPUT   1
#define AUDIO_OUTPUT  2
#define AUDIO_NOTIFY  3

class ManglerPCM
{
    public:
        ManglerPCM(uint32_t length, uint8_t *sample) {
            this->length = length;
            this->sample = (uint8_t *)malloc(length); // I'm a C programmer... sue me
            memcpy(this->sample, sample, length);
        }
        ~ManglerPCM() {
            free(this->sample);
        }
        uint32_t            length;
        uint8_t             *sample;
};
class ManglerAudioDevice
{
    public:
        ManglerAudioDevice(uint32_t id, Glib::ustring name, Glib::ustring description) {
            this->id            = id;
            this->name          = name;
            this->description   = description;
        }
        ~ManglerAudioDevice() {
        }
        uint32_t            id;
        Glib::ustring       name;
        Glib::ustring       description;
};
class ManglerAudio
{
    public:
        ManglerAudio(int type, uint32_t rate = 0, uint8_t channels = 1, uint32_t pcm_framesize = 0, uint8_t buffer = 4, bool check_loggedin = true);
        ~ManglerAudio();

        ManglerBackend  *backend;
        int             type;
        uint32_t        rate;
        uint32_t        pcm_framesize;
        uint8_t         channels;
        uint8_t         buffer;
        bool            check_loggedin;
        GAsyncQueue*    pcm_queue;
        bool            outputStreamOpen;
        bool            inputStreamOpen;
        bool            stop_output;
        bool            stop_input;

        std::map<Glib::ustring, ManglerPCM *> sounds;

        std::vector<ManglerAudioDevice*> outputDevices;
        std::vector<ManglerAudioDevice*> inputDevices;

        bool            switchBackend(Glib::ustring audioSubsystem);
        bool            open(void);
        void            close(bool drain = false);
        void            queue(uint32_t length, uint8_t *sample);
        void            finish(bool drop = false);
        void            output(void);
        void            input(void);
        void            getDeviceList(Glib::ustring audioSubsystem);
        void            playNotification(Glib::ustring name);
        void            playText(Glib::ustring text);
};

int timeval_subtract(struct timeval *result, struct timeval *x, struct timeval *y);

#endif

