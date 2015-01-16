/*
 * vim: softtabstop=4 shiftwidth=4 cindent foldmethod=marker expandtab
 *
 * $LastChangedDate: 2011-06-27 23:50:10 +0430 (Mon, 27 Jun 2011) $
 * $Revision: 1139 $
 * $LastChangedBy: econnell $
 * $URL: http://svn.mangler.org/mangler/trunk/src/mangleraudio.cpp $
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

#include "mangler.h"
#include "mangleraudio.h"
#include "mangler-sounds.h"
#include <sys/time.h>

#include "channeltree.h"
#include "manglersettings.h"

ManglerAudio::ManglerAudio(int type, uint32_t rate, uint8_t channels, uint32_t pcm_framesize, uint8_t buffer, bool check_loggedin) {/*{{{*/
    this->type           = type;
    this->rate           = rate;
    this->channels       = channels;
    this->pcm_framesize  = pcm_framesize;
    this->buffer         = buffer;
    this->check_loggedin = check_loggedin;
    outputStreamOpen = false;
    inputStreamOpen = false;
    backend = NULL;
    if (type < AUDIO_INPUT || !rate) {
        return;
    }
    if (type >= AUDIO_OUTPUT) {
        if (!open()) {
            return;
        }
        outputStreamOpen = true;
        stop_output = false;
        pcm_queue = g_async_queue_new();
        Glib::Thread::create(sigc::mem_fun(*this, &ManglerAudio::output), false);
    } else {
        if (!pcm_framesize) {
            fprintf(stderr, "pcm frame size not specified on input stream open; unsupported codec?\n");
            return;
        }
        if (!open()) {
            return;
        }
        inputStreamOpen = true;
        stop_input = false;
        Glib::Thread::create(sigc::mem_fun(*this, &ManglerAudio::input), false);
    }
}/*}}}*/
ManglerAudio::~ManglerAudio() {/*{{{*/
    if (backend) {
        delete backend;
    }
}/*}}}*/

bool
ManglerAudio::switchBackend(Glib::ustring audioSubsystem) {/*{{{*/
    ManglerBackend *backend;

    if (!(backend = ManglerBackend::getBackend(audioSubsystem, rate, channels, pcm_framesize))) {
        return false;
    }
    if (this->backend) {
        delete this->backend;
    }
    this->backend = backend;

    return true;
}/*}}}*/

bool
ManglerAudio::open(void) {/*{{{*/
    Glib::ustring direction;

    switch (type) {
      case AUDIO_INPUT:
        direction = "Input";
        break;
      default:
      case AUDIO_OUTPUT:
        direction = "Output";
        break;
      case AUDIO_NOTIFY:
        direction = "Notification";
        break;
    }
    Glib::ustring device = Mangler::config[direction + "DeviceName"].toUString();
    if (device == "Default") {
        device = "";
    } else if (device == "Custom") {
        device = Mangler::config[direction + "DeviceCustomName"].toUString();
    }
    close();
    if (!backend && !switchBackend(Mangler::config["AudioSubsystem"].toUString())) {
        return false;
    }
    if (!backend->open(type, device, rate, channels)) {
        return false;
    }

    return true;
}/*}}}*/

void
ManglerAudio::close(bool drain) {/*{{{*/
    if (backend) {
        backend->close(drain);
    }
}/*}}}*/

void
ManglerAudio::queue(uint32_t length, uint8_t *sample) {/*{{{*/
    if (!outputStreamOpen) {
        return;
    }
    g_async_queue_push(pcm_queue, new ManglerPCM(length, sample));
}/*}}}*/

void
ManglerAudio::finish(bool drop) {/*{{{*/
    if (outputStreamOpen) {
        stop_output = drop;
        g_async_queue_push(pcm_queue, new ManglerPCM(0, NULL));
    }
    if (inputStreamOpen) {
        stop_input = true;
    }
}/*}}}*/

void
ManglerAudio::output(void) {/*{{{*/
    ManglerPCM *queuedpcm = NULL;

    if (!pcm_queue) {
        return;
    }
    g_async_queue_ref(pcm_queue);
    for (;;) {
        if (stop_output || (check_loggedin && !v3_is_loggedin())) {
            close();
            break;
        }
        if (buffer) {
            buffer--;
            ManglerPCM *bufferpcm = (ManglerPCM *)g_async_queue_pop(pcm_queue);
            if (bufferpcm && bufferpcm->length) {
                uint32_t prelen = (queuedpcm) ? queuedpcm->length : 0;
                uint32_t buflen = prelen + bufferpcm->length;
                uint8_t *bufpcm = (uint8_t *)malloc(buflen);
                memcpy(bufpcm + prelen, bufferpcm->sample, bufferpcm->length);
                if (queuedpcm) {
                    memcpy(bufpcm, queuedpcm->sample, queuedpcm->length);
                    delete queuedpcm;
                }
                delete bufferpcm;
                queuedpcm = new ManglerPCM(buflen, bufpcm);
                free(bufpcm);
                continue;
            } else {
                buffer = 0;
                finish();
            }
        }
        if (!queuedpcm) {
            queuedpcm = (ManglerPCM *)g_async_queue_pop(pcm_queue);
        }
        // finish() queues a 0 length packet to notify us that we're done
        if (!queuedpcm->length) {
            close(true);
            break;
        }
        if (mangler->muteSound) {
            delete queuedpcm;
            queuedpcm = NULL;
            continue;
        }
        if (Mangler::config["AudioSubsystem"].toLower() != backend->getAudioSubsystem()) {
            if (!switchBackend(Mangler::config["AudioSubsystem"].toLower()) || !open()) {
                break;
            }
        }
        if (!backend->write(queuedpcm->sample, queuedpcm->length, channels)) {
            close();
            break;
        }
        delete queuedpcm;
        queuedpcm = NULL;
    }
    outputStreamOpen = false;
    close();
    while (queuedpcm || (queuedpcm = (ManglerPCM *)g_async_queue_try_pop(pcm_queue))) {
        delete queuedpcm;
        queuedpcm = NULL;
    }
    g_async_queue_unref(pcm_queue);
    delete this;
    return;
}/*}}}*/

void
ManglerAudio::input(void) {/*{{{*/
    uint8_t *buf = NULL;
    struct timeval start, vastart, now, diff;
    int ctr;
    bool drop, xmit = false;
    float seconds = 0;
    register float pcmpeak = 0;
    float midpeak = 0;
    float vasilencedur, vasilenceelapsed;
    uint8_t vapercent;

    for (;;) {
        if (stop_input) {
            break;
        }
        gettimeofday(&start, NULL);
        ctr = 0;
        seconds = 0;
        pcmpeak = 0;
        midpeak = 0;
        // need to send ~0.115 seconds of audio for each packet
        for (;;) {
            /*
            if (seconds >= 0.115) {
                //fprintf(stderr, "0.115 seconds of real time has elapsed\n");
                break;
            }
            */
            if (pcm_framesize * ctr > rate * sizeof(int16_t) * 0.115) {
                //fprintf(stderr, "we have 0.115 seconds of audio in %d iterations\n", ctr);
                break;
            }
            if ((pcm_framesize*(ctr+1)) > 16384) {
                fprintf(stderr, "audio frames are greater than buffer size.  dropping audio frames after %f seconds\n", seconds);
                drop = true;
                break;
            }
            drop = false;
            //fprintf(stderr, "reallocating %d bytes of memory\n", pcm_framesize*(ctr+1));
            buf = (uint8_t *)realloc(buf, pcm_framesize*(ctr+1));
            //fprintf(stderr, "reading %d bytes of memory to %lu\n", pcm_framesize, buf+(pcm_framesize*ctr));
            if (Mangler::config["AudioSubsystem"].toLower() != backend->getAudioSubsystem()) {
                if (!switchBackend(Mangler::config["AudioSubsystem"].toLower()) || !open()) {
                    stop_input = true;
                    break;
                }
            }
            if (!backend->read(buf+(pcm_framesize*ctr))) {
                close();
                stop_input = true;
                break;
            }
            gettimeofday(&now, NULL);
            timeval_subtract(&diff, &now, &start);
            seconds = (float)diff.tv_sec + ((float)diff.tv_usec / 1000000.0);
            //fprintf(stderr, "iteration after %f seconds with %d bytes\n", seconds, pcm_framesize*ctr);
            for (int16_t *pcmptr = (int16_t *)(buf+(pcm_framesize*ctr)); pcmptr < (int16_t *)(buf+(pcm_framesize*(ctr+1))); pcmptr++) {
                pcmpeak = abs(*pcmptr) > pcmpeak ? abs(*pcmptr) : pcmpeak;
            }
            if (seconds >= 0.115 / 2 && !midpeak && pcmpeak) {
                midpeak = log10(((pcmpeak / 0x7fff) * 9) + 1);
                midpeak = (midpeak > 1) ? 1 : midpeak;
                gdk_threads_enter();
                mangler->inputvumeter->set_fraction(midpeak);
                gdk_threads_leave();
            }
            ctr++;
        }
        if (!stop_input) {
            pcmpeak = log10(((pcmpeak / 0x7fff) * 9) + 1);
            pcmpeak = (pcmpeak > 1) ? 1 : pcmpeak;
            gdk_threads_enter();
            if (Mangler::config["VoiceActivationEnabled"].toBool() &&
                    !mangler->isTransmittingKey &&
                    !mangler->isTransmittingMouse &&
                    !mangler->isTransmittingButton) {
                vasilencedur = Mangler::config["VoiceActivationSilenceDuration"].toInt();
                vapercent = Mangler::config["VoiceActivationSensitivity"].toUInt();
                if (pcmpeak * 100 >= vapercent) {
                    gettimeofday(&vastart, NULL);
                    if (!xmit) {
                        xmit = true;
                        mangler->audioControl->playNotification("talkstart");
                        mangler->statusIcon->set(mangler->icons["tray_icon_green"]);
                        v3_start_audio(V3_AUDIO_SENDTYPE_U2CCUR);
                    }
                    mangler->channelTree->setUserIcon(v3_get_user_id(), "green", true);
                } else if (xmit) {
                    gettimeofday(&now, NULL);
                    timeval_subtract(&diff, &now, &vastart);
                    vasilenceelapsed = (float)diff.tv_sec + ((float)diff.tv_usec / 1000000.0);
                    if (vasilenceelapsed * 1000 >= vasilencedur) {
                        xmit = false;
                        v3_stop_audio();
                        mangler->channelTree->setUserIcon(v3_get_user_id(), "orange", true);
                        mangler->statusIcon->set(mangler->icons["tray_icon_yellow"]);
                        mangler->audioControl->playNotification("talkend");
                    } else {
                        mangler->channelTree->setUserIcon(v3_get_user_id(), "yellow", true);
                    }
                }
            } else {
                if (!xmit) {
                    xmit = true;
                    mangler->audioControl->playNotification("talkstart");
                    v3_start_audio(V3_AUDIO_SENDTYPE_U2CCUR);
                }
                mangler->channelTree->setUserIcon(v3_get_user_id(), "green", true);
                mangler->statusIcon->set(mangler->icons["tray_icon_green"]);
            }
            mangler->inputvumeter->set_fraction(pcmpeak);
            gdk_threads_leave();
            if (!drop && xmit) {
                //fprintf(stderr, "sending %d bytes of audio\n", pcm_framesize * ctr);
                // TODO: hard coding user to channel for now, need to implement U2U
                uint32_t ret;
                if ((ret = v3_send_audio(V3_AUDIO_SENDTYPE_U2CCUR, rate, buf, pcm_framesize * ctr, false)) != rate) {
                    if (!(rate = ret) || !open()) {
                        stop_input = true;
                    }
                }
            }
        }
        free(buf);
        buf = NULL;
    }
    inputStreamOpen = false;
    close();
    gdk_threads_enter();
    if (xmit) {
        xmit = false;
        v3_stop_audio();
        mangler->audioControl->playNotification("talkend");
    }
    if (v3_is_loggedin()) {
        mangler->channelTree->setUserIcon(v3_get_user_id(), "red");
        mangler->statusIcon->set(mangler->icons["tray_icon_red"]);
    }
    mangler->inputvumeter->set_fraction(0);
    if (mangler->inputAudio == this) {
        mangler->inputAudio = NULL;
    }
    gdk_threads_leave();
    delete this;
    return;
}/*}}}*/

void
ManglerAudio::getDeviceList(Glib::ustring audioSubsystem) {/*{{{*/
    outputDevices.clear();
    inputDevices.clear();

    ManglerBackend::getDeviceList(audioSubsystem, inputDevices, outputDevices);
}/*}}}*/

void
ManglerAudio::playNotification(Glib::ustring name) {/*{{{*/
    if (mangler->muteSound) {
        return;
    }
    if ((name == "talkstart" || name == "talkend") && !Mangler::config["NotificationTransmitStartStop"].toBool()) {
        return;
    }
    if ((name == "channelenter" || name == "channelleave") && !Mangler::config["NotificationChannelEnterLeave"].toBool()) {
        return;
    }
    if ((name == "login" || name == "logout") && !Mangler::config["NotificationLoginLogout"].toBool()) {
        return;
    }
    if (sounds.empty()) {
        sounds["talkstart"]    = new ManglerPCM(sizeof(sound_talkstart),    sound_talkstart);
        sounds["talkend"]      = new ManglerPCM(sizeof(sound_talkend),      sound_talkend);
        sounds["channelenter"] = new ManglerPCM(sizeof(sound_channelenter), sound_channelenter);
        sounds["channelleave"] = new ManglerPCM(sizeof(sound_channelleave), sound_channelleave);
        sounds["login"]        = new ManglerPCM(sizeof(sound_login),        sound_login);
        sounds["logout"]       = new ManglerPCM(sizeof(sound_logout),       sound_logout);
    }
    ManglerAudio *notify = new ManglerAudio(AUDIO_NOTIFY, 44100, 1, 0, 0, false);
    notify->queue(sounds[name]->length, sounds[name]->sample);
    notify->finish();
}/*}}}*/

#ifdef HAVE_ESPEAK
int
espeak_synth_cb(short *wav, int numsamples, espeak_EVENT *events) {/*{{{*/
    ManglerAudio *tts = (ManglerAudio *)events->user_data;

    if (!numsamples) {
        if (tts) {
            tts->finish();
            events->user_data = NULL;
        }
        return 1;
    }
    tts->queue(numsamples * sizeof(short), (uint8_t *)wav);

    return 0;
}/*}}}*/
#endif

void
ManglerAudio::playText(Glib::ustring text) {/*{{{*/
    if (!text.length() || !Mangler::config["NotificationTextToSpeech"].toBool()) {
        return;
    }
#ifdef HAVE_ESPEAK
    espeak_SetSynthCallback(espeak_synth_cb);
    ManglerAudio *tts = new ManglerAudio(AUDIO_NOTIFY, mangler->espeakRate, 1, 0, 0, false);
    if (espeak_Synth(text.c_str(), text.length() + 1, 0, POS_CHARACTER, 0, espeakCHARS_AUTO, NULL, tts) != EE_OK) {
        tts->finish();
        fprintf(stderr, "espeak: synth error\n");
        return;
    }
#endif
}/*}}}*/

int
timeval_subtract(struct timeval *result, struct timeval *x, struct timeval *y) {/*{{{*/
    /* Perform the carry for the later subtraction by updating y. */
    if (x->tv_usec < y->tv_usec) {
        int nsec = (y->tv_usec - x->tv_usec) / 1000000 + 1;
        y->tv_usec -= 1000000 * nsec;
        y->tv_sec += nsec;
    }
    if (x->tv_usec - y->tv_usec > 1000000) {
        int nsec = (x->tv_usec - y->tv_usec) / 1000000;
        y->tv_usec += 1000000 * nsec;
        y->tv_sec -= nsec;
    }

    /* Compute the time remaining to wait.
       tv_usec is certainly positive. */
    result->tv_sec = x->tv_sec - y->tv_sec;
    result->tv_usec = x->tv_usec - y->tv_usec;

    /* Return 1 if result is negative. */
    return x->tv_sec < y->tv_sec;
}/*}}}*/

