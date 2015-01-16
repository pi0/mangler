/*
 * vim: softtabstop=4 shiftwidth=4 cindent foldmethod=marker expandtab
 *
 * $LastChangedDate: 2012-05-28 01:45:00 +0430 (Mon, 28 May 2012) $
 * $Revision: 1173 $
 * $LastChangedBy: econnell $
 * $URL: http://svn.mangler.org/mangler/trunk/jukebox/jukebox.c $
 *
 * Copyright 2009-2010 Eric Connell
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
 *
 *
 ******************************************************************
 *         All hope abandon ye who open this source code.         *
 ******************************************************************
 *                                                                *
 *                           _,.-------.,_                        *
 *                       ,;~'             '~;,                    *
 *                     ,;                     ;,                  *
 *                    ;                         ;                 *
 *                   ,'                         ',                *
 *                  ,;                           ;,               *
 *                  ; ;      .           .      ; ;               *
 *                  | ;   ______       ______   ; |               *
 *                  |  `/~"     ~" . "~     "~\'  |               *
 *                  |  ~  ,-~~~^~, | ,~^~~~-,  ~  |               *
 *                   |   |        }:{        |   |                *
 *                   |   l       / | \       !   |                *
 *                   .~  (__,.--" .^. "--.,__)  ~.                *
 *                   |     ---;' / | \ `;---     |                *
 *                    \__.       \/^\/       .__/                 *
 *                     V| \                 / |V                  *
 *                      | |T~\___!___!___/~T| |                   *
 *                      | |`IIII_I_I_I_IIII'| |                   *
 *                      |  \,III I I I III,/  |                   *
 *                       \   `~~~~~~~~~~'    /                    *
 *                         \   .       .   /                      *
 *                           \.    ^    ./                        *
 *                             ^~~~^~~~^                          *
 *                                                                *
 ******************************************************************
  oh god this still exists :(
 */

#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <strings.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <getopt.h>
#include <pthread.h>
#include <libgen.h>
#include <math.h>

#include <ventrilo3.h>

#include <mpg123.h>

#if HAVE_VORBIS || HAVE_FLAC
static const char vorbis_artist[] = "ARTIST=";
static const char vorbis_title[] = "TITLE=";
static const char vorbis_album[] = "ALBUM=";

typedef struct {
    int16_t  buf[32768];
    uint32_t len;
} pcm_data_t;

pcm_data_t pcmd;
#endif

#if !HAVE_VORBIS
# warning "Vorbis support not enabled."
#else
# warning "Vorbis support enabled."
# include <vorbis/vorbisfile.h>
#endif

#if !HAVE_FLAC
# warning "FLAC support not enabled."
#else
# warning "FLAC support enabled."
# include <FLAC/stream_decoder.h>
# include <FLAC/metadata.h>

# define flac_dec_init                  FLAC__stream_decoder_new
# define flac_dec_destroy               FLAC__stream_decoder_delete
# define flac_dec_get_state             FLAC__stream_decoder_get_state
# define flac_dec_get_state_string      FLAC__stream_decoder_get_resolved_state_string
# define flac_dec_get_total_samples     FLAC__stream_decoder_get_total_samples
# define flac_dec_get_channels          FLAC__stream_decoder_get_channels
# define flac_dec_get_bits_per_sample   FLAC__stream_decoder_get_bits_per_sample
# define flac_dec_get_sample_rate       FLAC__stream_decoder_get_sample_rate
# define flac_dec_init_file             FLAC__stream_decoder_init_file
# define flac_dec_process_single        FLAC__stream_decoder_process_single
# define flac_dec_process_metadata      FLAC__stream_decoder_process_until_end_of_metadata

# define FLAC_DEC_INIT_OK               FLAC__STREAM_DECODER_INIT_STATUS_OK

# define FLAC_DEC_END_OF_STREAM         FLAC__STREAM_DECODER_END_OF_STREAM

# define flac_dec                       FLAC__StreamDecoder
# define flac_dec_write                 FLAC__StreamDecoderWriteStatus
# define flac_dec_error                 FLAC__StreamDecoderErrorStatus

# define FLAC_DEC_WRITE_OK              FLAC__STREAM_DECODER_WRITE_STATUS_CONTINUE
# define FLAC_DEC_WRITE_ABORT           FLAC__STREAM_DECODER_WRITE_STATUS_ABORT

# define flac_frame_t                   FLAC__Frame
# define flac_int32_t                   FLAC__int32
# define flac_metadata_t                FLAC__StreamMetadata

# define flac_metadata_get_tags         FLAC__metadata_get_tags
# define flac_metadata_destroy          FLAC__metadata_object_delete

# define FLAC_METADATA_STREAMINFO       FLAC__METADATA_TYPE_STREAMINFO
# define FLAC_METADATA_VORBIS_COMMENT   FLAC__METADATA_TYPE_VORBIS_COMMENT
#endif

#define false   0
#define true    1

// data types
struct _conninfo {
    char *server;
    char *username;
    char *password;
    char *channelid;
    char *path;
    float volume;
};

struct _musicfile {
    void *mh;
    char *filename;
    char vorbis;
    char flac;
    int rate;
    int channels;
    int invalid;
    char *path;
    char *title;
    char *artist;
    char *album;
    char *genre;
};

typedef struct _musicfile musicfile;

// global vars
int debug = 0;
int should_exit = false;
musicfile **musiclist;
int musicfile_count = 0;
int media_pathlen = 0;
int disable_stereo = false;

// prototypes
void usage(char *argv[]);
void ctrl_c(int signum);
void *jukebox_player(void *connptr);
void *open_file(musicfile *musicfile, const v3_codec *codec);
int get_pcm_frame(musicfile *musicfile, int channels, int16_t *buf, int *pcmread);
uint64_t timediff(const struct timeval *left, const struct timeval *right);
void close_file(musicfile *musicfile);
int get_id3_info(musicfile *musicfile);
char *id3strdup(mpg123_string *inlines);
int get_random_number(int min, int max);
void send_now_playing(int filenum);
int select_channel(void);
void shuffle_musiclist(void);
char *strip_pathelem(char *src);

void ctrl_c(int signum) {
    fprintf(stderr, "disconnecting... ");
    v3_logout();
    fprintf(stderr, "done\n");
}

void usage(char *argv[]) {
    fprintf(stderr, "usage: %s -h hostname:port -u username [-p password] [-c channelid] [-v volume_multipler] [-s disable stereo for opus] [-n don't shuffle] /path/to/music\n", argv[0]);
    exit(EXIT_FAILURE);
}

void *jukebox_player(void *connptr) {
    struct _conninfo *conninfo = connptr;
    musicfile *musicfile = NULL;
    int connected = false;
    int playing = false;
    int stopped = true;
    int filenum = -1;
    int playonce = false;
    int16_t sendbuf[16384];
    v3_event *ev;
    const v3_codec *codec;
    v3_user *user;
    struct timeval tm_start, tm_end;
    uint64_t audio_dur, code_dur;
    int pcmread;
    uint8_t channels;
    int ctr;
    struct timeval last_audio, now;
    char playonly[64];
    int politeness = 0; // only slightly polite :)
    float multiplier;

    gettimeofday(&last_audio, NULL);
    while (!should_exit && v3_is_loggedin()) {
        if ((ev = v3_get_event(stopped ? V3_BLOCK : V3_NONBLOCK))) {
            if (debug) {
                fprintf(stderr, "jukebox: got event type %d\n", ev->type);
            }
            switch (ev->type) {
                case V3_EVENT_DISCONNECT:
                    v3_free_event(ev);
                    should_exit = true;
                    continue;
                case V3_EVENT_LOGIN_COMPLETE:
                    codec = v3_get_channel_codec(0);
                    if (debug) {
                        fprintf(stderr, "login complete...\n");
                    }
                    fprintf(stderr, "server default codec rate is %u\n", codec->rate);
                    if (!conninfo->channelid) {
                        v3_change_channel(select_channel(), "");
                    } else {
                        v3_change_channel(atoi(conninfo->channelid), "");
                    }
                    v3_join_chat();
                    connected = true;
                    break;
                case V3_EVENT_USER_CHAN_MOVE:
                    if (ev->user.id == v3_get_user_id()) {
                        codec = v3_get_channel_codec(ev->channel.id);
                        fprintf(stderr, "channel codec rate is %u\n", codec->rate);
                    }
                    break;
                case V3_EVENT_PLAY_AUDIO:
                    //last_audio = now;
                    gettimeofday(&last_audio, NULL);
                    break;
                case V3_EVENT_CHAT_MESSAGE:
                    if (ev->user.id == v3_get_user_id()) {
                        // ignore commands from ourself
                        break;
                    }
                    if (strcmp(ev->data->chatmessage, "!play worst band in the world") == 0) {
                        v3_send_chat_message("We don't have any Creed songs...");
                    } else if (strcasecmp(ev->data->chatmessage, "!play creed") == 0) {
                        v3_send_chat_message("No.");
                    } else if (strcasecmp(ev->data->chatmessage, "!help") == 0) {
                        v3_send_chat_message("!start -- start playing music");
                        v3_send_chat_message("!stop -- stop playing music");
                        v3_send_chat_message("!next -- play a new random track");
                        v3_send_chat_message("!move -- move to your channel");
                        v3_send_chat_message("!play [song/artist/file name] -- search for a song by filename and play the first random match");
                        v3_send_chat_message("!playonce [song/artist/file name] -- play a single file and stop");
                        v3_send_chat_message("!playonly [song/artist/file name] -- don't play anything that doesn't match the given string");
                        v3_send_chat_message("!vol [0-100] -- set the volume to the specified level: ex: !vol 50");
                        v3_send_chat_message("!polite [off|0-60] -- pauses playing when audio is received for the specified duration");
                        break;
                    } else if (strncmp(ev->data->chatmessage, "!shuffle", 8) == 0) {
                        if (musicfile) {
                            v3_stop_audio();
                            close_file(musicfile);
                            musicfile = NULL;
                        }
                        playing = false;
                        v3_send_chat_message("Playlist has been re-shuffled");
                    } else if (strncmp(ev->data->chatmessage, "!polite", 7) == 0) {
                        if (ev->data->chatmessage[8]) {
                            char *level = ev->data->chatmessage + 8;
                            int newpol = -1;
                            if (strncmp(level, "off", 3) == 0) {
                                politeness = -1;
                            } else {
                                newpol = atoi(level);
                                if (newpol > -1 && newpol < 61) {
                                    politeness = newpol;
                                }
                            }
                        } else {
                            politeness = -1;
                        }
                        if (politeness > -1) {
                            char chat_msg[50];
                            sprintf(chat_msg, "politeness is now %d seconds", politeness);
                            v3_send_chat_message(chat_msg);
                        } else {
                            v3_send_chat_message("politeness is now off");
                        }
                        break;
                    } else if (! stopped && strncmp(ev->data->chatmessage, "!vol ", 5) == 0) {
                        char *volume = ev->data->chatmessage + 5;
                        int vol = atoi(volume);
                        if (vol < 0) {
                            vol = 0;
                        } else if (vol > 100) {
                            vol = 100;
                        }
                        conninfo->volume = vol / 100.0;
                        char chat_msg[64];
                        snprintf(chat_msg, 63, "volume is now %d%%", vol);
                        v3_send_chat_message(chat_msg);
                    } else if (
                            strncmp(ev->data->chatmessage, "!play ", 6) == 0
                            || strncmp(ev->data->chatmessage, "!playonly ", 10) == 0
                            || strncmp(ev->data->chatmessage, "!playonce ", 10) == 0
                        ) {
                        char *searchspec;
                        int ctr;
                        int found = false;
                        if (strncmp(ev->data->chatmessage, "!playonly ", 10) == 0) {
                            strncpy(playonly, ev->data->chatmessage + 10, 64);
                            searchspec = playonly;
                        } else if (strncmp(ev->data->chatmessage, "!playonce ", 10) == 0) {
                            playonce = true;
                            searchspec = ev->data->chatmessage + 10;
                            playonly[0] = 0;
                        } else {
                            searchspec = ev->data->chatmessage + 6;
                            playonly[0] = 0;
                        }
                        for (ctr = 0; ctr < musicfile_count; ctr++) {
                            // make sure we have at least 1 thing that matches
                            // so  we don't end up in an endless loop
                            if (strcasestr(musiclist[ctr]->path, searchspec)) { 
                                found = true;
                                break;
                            }
                        }
                        if (! found) {
                            v3_send_chat_message("no songs matched your request");
                            playonly[0] = 0;
                        } else {
                            int attempts = 0;
                            if (playing || musicfile) {
                                playing = false;
                                v3_stop_audio();
                                close_file(musicfile);
                                musicfile = NULL;
                            }
                            // we have SOMETHING in the filelist that matches, but no guarantee that it's a song... try 10
                            // different matches before giving up
                            for (attempts = 0; attempts < 20; attempts++) {
                                filenum++;
                                if (filenum >= musicfile_count) filenum = 0;
                                if (debug) {
                                    fprintf(stderr, "checking for %s: %s\n", searchspec, musiclist[filenum]->path);
                                }
                                if (strcasestr(musiclist[filenum]->path, searchspec) == 0) {
                                    // this file didn't match, so just get a new random file and don't count this
                                    // attempt
                                    attempts--;
                                    continue;
                                }
                                if (debug) {
                                    fprintf(stderr, "found %s in %s\n", searchspec, musiclist[filenum]->path);
                                }
                                if ((musicfile = open_file(musiclist[filenum], codec))) {
                                    send_now_playing(filenum);
                                    playing = true;
                                    stopped = false;
                                    v3_start_audio(V3_AUDIO_SENDTYPE_U2CCUR);
                                    break;
                                } else {
                                    if (debug) {
                                        fprintf(stderr, "could not open: %s\n", musiclist[filenum]->path);
                                    }
                                    attempts = 20;
                                }
                            }
                            if (attempts > 20) {
                                // give up and just pick a random song
                                v3_send_chat_message("Apparently something matched but it didn't appear to be a song, so I fail.  Here's something else...");
                                stopped = false;
                                playonce = false;
                            }
                        }
                    } else if (! stopped && strcmp(ev->data->chatmessage, "!next") == 0) {
                        if (musicfile) {
                            v3_stop_audio();
                            close_file(musicfile);
                            musicfile = NULL;
                        }
                        playing = false;
                    } else if (strcmp(ev->data->chatmessage, "!move") == 0) {
                        user = v3_get_user(ev->user.id);
                        v3_change_channel(user->channel, "");
                        v3_free_user(user);
                    } else if (stopped && strcmp(ev->data->chatmessage, "!start") == 0) {
                        stopped = false;
                        v3_send_chat_message("Starting jukebox...");
                    } else if (! stopped && strcmp(ev->data->chatmessage, "!stop") == 0) {
                        v3_send_chat_message("Stopping jukebox...");
                        if (musicfile) {
                            v3_stop_audio();
                            close_file(musicfile);
                            musicfile = NULL;
                        }
                        v3_set_text("", "", "", true);
                        playing = false;
                        stopped = true;
                    } else {
                        fprintf(stderr, "chat message: '%s'\n", ev->data->chatmessage);
                    }
                    break;
            }
            v3_free_event(ev);
        }
        gettimeofday(&now, NULL);
        if (connected && ! stopped && (politeness < 0 || timediff(&last_audio, &now) >= politeness * 1000000 + 500000 )) {
            if (! playing) {
                while (! musicfile) {
                    filenum++;
                    if (filenum >= musicfile_count) filenum = 0;
                    if (strlen(playonly) && !strcasestr(musiclist[filenum]->path, playonly)) {
                        continue;
                    }
                    if (!(musicfile = open_file(musiclist[filenum], codec))) {
                        if (debug) {
                            fprintf(stderr, "could not open: %s\n", musiclist[filenum]->path);
                        }
                    }
                }
                if (debug) {
                    fprintf(stderr, "playing: %s\n", musiclist[filenum]->path);
                }
                gettimeofday(&tm_start, NULL);
                send_now_playing(filenum);
                v3_start_audio(V3_AUDIO_SENDTYPE_U2CCUR);
                playing = true;
            }
            if (! musicfile) {
                fprintf(stderr, "musicfile is NULL?  unpossible!\n");
                exit(1);
            }
            channels = 1;
            switch (codec->codec) {
              case 1:
              case 2:
                channels = (disable_stereo || musiclist[filenum]->channels != 2) ? 1 : 2;
                break;
            }
            pcmread = v3_max_pcm_frames(codec) * codec->pcmframesize * channels;
            if (debug) {
                fprintf(stderr, "want to read %d pcm bytes\n", pcmread);
            }
            if (get_pcm_frame(musicfile, (channels == 2) ? 1 : 2, sendbuf, &pcmread)) {
                if (conninfo->volume != 1.0) {
                    multiplier = tan(conninfo->volume * M_PI * 25 / 100.0);
                    for (ctr = 0; ctr < pcmread / sizeof(int16_t); ctr++) {
                        sendbuf[ctr] *= multiplier;
                    }
                }
                v3_send_audio(V3_AUDIO_SENDTYPE_U2CCUR, musiclist[filenum]->rate, (uint8_t *)sendbuf, pcmread, (channels == 2));
                audio_dur = (pcmread / (double)(musiclist[filenum]->rate * sizeof(int16_t) * channels)) * 1000000.0;
                gettimeofday(&tm_end, NULL);
                code_dur = timediff(&tm_start, &tm_end);
                if (code_dur < audio_dur) {
                    usleep(audio_dur - code_dur);
                }
                gettimeofday(&tm_start, NULL);
            } else {
                if (musicfile) {
                    v3_stop_audio();
                    close_file(musicfile);
                    musicfile = NULL;
                }
                if (playonce) {
                    stopped = true;
                    playonce = false;
                }
                playing = false;
            }
        }
    }

    pthread_detach(pthread_self());
    pthread_exit(NULL);
}

void send_now_playing(int filenum) {
    musicfile *musicfile = musiclist[filenum];
    char msgbuf[255] = "";

    if (!musicfile->vorbis && !musicfile->flac && !get_id3_info(musicfile)) {
        if (debug) {
            fprintf(stderr, "no valid id3 tag: %s\n", musicfile->path);
        }
    }
    if (musicfile->artist || musicfile->title || musicfile->album) {
        if (musicfile->artist && strlen(musicfile->artist)) {
            strncat(msgbuf, musicfile->artist, 254);
        }
        if (musicfile->title && strlen(musicfile->title)) {
            strncat(msgbuf, " - \"", 254);
            strncat(msgbuf, musicfile->title, 254);
            strncat(msgbuf, "\"", 254);
        }
        if (musicfile->album && strlen(musicfile->album)) {
            strncat(msgbuf, " from ", 254);
            strncat(msgbuf, musicfile->album, 254);
        }
        v3_set_text("", "", msgbuf, true);
    } else {
        v3_set_text("", "", "", true);
        strncat(msgbuf, musicfile->path+media_pathlen, 254);
    }
    v3_send_chat_message(msgbuf);
}

void read_playlist_file(char *path) {
    char *basepath, *pathcp, *temp;
    char buf[4096];
    struct stat s;
    FILE *f;
    int i;

    pathcp = strdup(path);
    basepath = dirname(pathcp);

    if (! (f = fopen(path, "r"))) {
        fprintf(stderr, "could not open playlist file: %s\n", path);
        exit(1);
    }
    
    while (fgets(buf, 4096, f)) {
        if (buf[0] == '#') continue;
        i = strlen(buf) - 1;
        while (i > 0 && (buf[i] == '\n' || buf[i] == '\t' || buf[i] == '\r' || buf[i] == ' ')) --i;
        if (i < 1) continue;
        buf[i+1] = '\0';
        i = 0;
        while (i < 4096 && (buf[i] == ' ' || buf[i] == '\t' || buf[i] == '\r')) ++i;
        if (i) {
            temp = strdup(buf+i);
            strcpy(buf, temp);
            free(temp);
        }
        if (buf[0] != '/') {
            // contains relative path
            temp = strdup(buf);
            strcpy(buf, basepath);
            i = strlen(buf);
            buf[i] = '/';
            i++;
            strncpy(buf + i, temp, 4096 - i);
            free(temp);
        }
        
        if (stat(buf, &s)) {
            if (debug) fprintf(stderr, "could not stat '%s', skipping.\n", buf);
            continue;
        }
        
        if (! S_ISREG(s.st_mode)) {
            if (debug) fprintf(stderr, "'%s' is not a regular file, skipping.\n", buf);
            continue;
        }

        musiclist = realloc(musiclist, (musicfile_count+1) * sizeof(musicfile *));
        musiclist[musicfile_count] = malloc(sizeof(musicfile));
        memset(musiclist[musicfile_count], 0, sizeof(musicfile));
        musiclist[musicfile_count]->path = strdup(buf);
        musicfile_count++;
        if (debug) {
            fprintf(stderr, "added file #%d: %s\n", musicfile_count, buf);
        }
    }

    fclose(f);
    free(pathcp);
}

void scan_media_path(char *path) {
    DIR *dir;
    struct dirent *ent;
    char namebuf[2048];
    char *cptr;
    struct stat s;
    
    if (stat(path, &s)) {
        fprintf(stderr, "could not stat: %s\n", path);
        exit(1);
    }

    if (S_ISREG(s.st_mode)) {
        read_playlist_file(path);
        return;
    }

    if (! (dir = opendir(path))) {
        fprintf(stderr, "could not open diretory: %s\n", path);
        exit(1);
    }
    while ((ent = readdir(dir))) {
        if (strcmp(ent->d_name, ".") == 0 || strcmp(ent->d_name, "..") == 0) {
            continue;
        }
        snprintf(namebuf, 2047, "%s/%s", path, ent->d_name);
        if (stat(namebuf, &s) != 0) {
            continue;
        }
        if (S_ISDIR(s.st_mode)) {
            if (debug) {
                fprintf(stderr, "found directory: %s\n", namebuf);
            }
            scan_media_path(namebuf);
        } else {
            cptr = namebuf + strlen(namebuf);
            if (!strcasecmp(cptr-4, ".mp3")
#if HAVE_VORBIS
            || !strcasecmp(cptr-4, ".ogg")
#endif
#if HAVE_FLAC
            || !strcasecmp(cptr-5, ".flac")
#endif
            ) {
                musiclist = realloc(musiclist, (musicfile_count+1) * sizeof(musicfile *));
                musiclist[musicfile_count] = malloc(sizeof(musicfile));
                memset(musiclist[musicfile_count], 0, sizeof(musicfile));
                musiclist[musicfile_count]->path = strdup(namebuf);
#if HAVE_VORBIS
                musiclist[musicfile_count]->vorbis = !strcasecmp(cptr-4, ".ogg");
#endif
#if HAVE_FLAC
                musiclist[musicfile_count]->flac = !strcasecmp(cptr-5, ".flac");
#endif
                /*
                   if (!get_id3_info(musiclist[musicfile_count])) {
                   free(musiclist[musicfile_count]);
                   continue;
                   }
                 */
                musicfile_count++;
                if (debug) {
                    fprintf(stderr, "found file #%d: %s\n", musicfile_count, namebuf);
                }
            }
        }
    }
    closedir(dir);
}

int get_id3_info(musicfile *musicfile) {
    void *mh = NULL;
    mpg123_id3v1 *v1;
    mpg123_id3v2 *v2;
    int meta;

    if (! musicfile) {
        return 0;
    }
    mpg123_init();
    if (debug) {
        fprintf(stderr, "scanning file %s\n", musicfile->path);
    }
    mh = mpg123_new(NULL, NULL);
    if(mpg123_open(mh, musicfile->path) != MPG123_OK) {
        if (debug) {
            fprintf(stderr, "cannot open %s\n", musicfile->path);
        }
        mpg123_exit();
        return 0;
    }
    if (mpg123_scan(mh) != MPG123_OK) {
        if (debug) {
            fprintf(stderr, "cannot scan %s\n", musicfile->path);
        }
        mpg123_close(mh);
        mpg123_delete(mh);
        mpg123_exit();
        return 0;
    }
    meta = mpg123_meta_check(mh);
    if (meta & MPG123_ID3 && mpg123_id3(mh, &v1, &v2) == MPG123_OK) {
        if(v2 != NULL) {
            musicfile->title  = id3strdup(v2->title);
            musicfile->artist = id3strdup(v2->artist);
            musicfile->album  = id3strdup(v2->album);
            musicfile->genre  = id3strdup(v2->genre);
            if (debug) {
                fprintf(stderr, "found an id3 tag on %s\n", musicfile->path);
            }
            /*
               fprintf(stderr, "title : %s\n", musicfile->title);
               fprintf(stderr, "artist: %s\n", musicfile->artist);
               fprintf(stderr, "album : %s\n", musicfile->album);
               fprintf(stderr, "genre : %s\n", musicfile->genre);
             */
        } else {
            if (debug) {
                fprintf(stderr, "no id3 tag on %s\n", musicfile->path);
            }
        }
    }
    mpg123_close(mh);
    mpg123_delete(mh);
    mpg123_exit();
    return 1;
}

char *id3strdup(mpg123_string *inlines) {
    size_t i;
    int hadcr = 0, hadlf = 0;
    char *lines = NULL;
    char *line  = NULL;
    size_t len = 0;
    char *ret;

    if (inlines != NULL && inlines->fill) {
        lines = inlines->p;
        len   = inlines->fill;
    } else {
        return NULL;
    }

    line = lines;
    for(i=0; i<len; ++i) {
        if(lines[i] == '\n' || lines[i] == '\r' || lines[i] == 0) {
            char save = lines[i]; /* saving, changing, restoring a byte in the data */
            if(save == '\n') ++hadlf;
            if(save == '\r') ++hadcr;
            if((hadcr || hadlf) && hadlf % 2 == 0 && hadcr % 2 == 0) line = "";

            if(line) {
                lines[i] = 0;
                ret = strdup(line);
                return ret;
                // whatever else it's doing, we don't care...
                line = NULL;
                lines[i] = save;
            }
        } else {
            hadlf = hadcr = 0;
            if(line == NULL) line = lines+i;
        }
    }
    return NULL;
}

#if HAVE_FLAC
flac_dec_write
flac_write(const flac_dec *dec, const flac_frame_t *frame, const flac_int32_t *const buf[], void *data) {
    (void)data;
    uint8_t channels = flac_dec_get_channels(dec);
    uint32_t ctr;

    if (frame->header.blocksize * channels > sizeof(pcmd.buf)) {
        fprintf(stderr, "error: flac_write: blocksize * channels %i > buffer %lu bytes\n", frame->header.blocksize * channels, sizeof(pcmd.buf));
        return FLAC_DEC_WRITE_ABORT;
    }
    for (ctr = 0, pcmd.len = 0; ctr < frame->header.blocksize * channels; ctr++) {
        pcmd.buf[pcmd.len++] = buf[0][ctr];
        if (channels == 2) {
            pcmd.buf[pcmd.len++] = buf[1][ctr];
        }
    }

    return FLAC_DEC_WRITE_OK;
}

void
flac_metadata(const flac_dec *dec, const flac_metadata_t *metadata, void *data) {
    (void)dec;
    musicfile *musicfile = data;

    if (metadata->type == FLAC_METADATA_STREAMINFO) {
        musicfile->rate = metadata->data.stream_info.sample_rate;
        musicfile->channels = metadata->data.stream_info.channels;
    }
}

void
flac_error(const flac_dec *dec, flac_dec_error status, void *data) {
    (void)status, (void)data;
    fprintf(stderr, "error: flac_error: %s\n", flac_dec_get_state_string(dec));
}
#endif

void *open_file(musicfile *musicfile, const v3_codec *codec) {
    int err = MPG123_OK;
    long int rate = 0;
    int channels = 0;
    int encoding = 0;
    mpg123_pars *mp;
    int result;

#if HAVE_VORBIS
    if (musicfile->vorbis) {
        vorbis_info *vi;
        vorbis_comment *vc;
        char *comment;
        int ctr;

        pcmd.len = 0;
        musicfile->mh = malloc(sizeof(OggVorbis_File));
        if ((err = ov_fopen(musicfile->path, musicfile->mh))) {
            fprintf(stderr, "error: ov_fopen: %i\n", err);
            close_file(musicfile);
            return NULL;
        }
        if (!(vi = ov_info(musicfile->mh, -1))) {
            fprintf(stderr, "error: ov_info: corrupted stream?\n");
            close_file(musicfile);
            return NULL;
        }
        musicfile->rate = vi->rate;
        musicfile->channels = vi->channels;
        if (!(vc = ov_comment(musicfile->mh, -1))) {
            fprintf(stderr, "error: ov_comment: corrupted stream?\n");
            close_file(musicfile);
            return NULL;
        }
        for (ctr = 0; ctr < vc->comments; ctr++) {
            comment = vc->user_comments[ctr];
            if (!strncasecmp(comment, vorbis_artist, sizeof(vorbis_artist) - 1)) {
                if (musicfile->artist) {
                    free(musicfile->artist);
                }
                musicfile->artist = strdup(comment + sizeof(vorbis_artist) - 1);
            } else if (!strncasecmp(comment, vorbis_title, sizeof(vorbis_title) - 1)) {
                if (musicfile->title) {
                    free(musicfile->title);
                }
                musicfile->title = strdup(comment + sizeof(vorbis_title) - 1);
            } else if (!strncasecmp(comment, vorbis_album, sizeof(vorbis_album) - 1)) {
                if (musicfile->album) {
                    free(musicfile->album);
                }
                musicfile->album = strdup(comment + sizeof(vorbis_album) - 1);
            }
        }

        return musicfile;
    }
#endif
#if HAVE_FLAC
    if (musicfile->flac) {
        flac_metadata_t *tags;
        void *comment;
        uint32_t ctr;

        if (!(musicfile->mh = flac_dec_init())) {
            fprintf(stderr, "error: flac_dec_init: init failed\n");
            return NULL;
        }
        if (flac_dec_init_file(musicfile->mh, musicfile->path, flac_write, flac_metadata, flac_error, musicfile) != FLAC_DEC_INIT_OK) {
            fprintf(stderr, "error: flac_dec_init_file: %s\n", flac_dec_get_state_string(musicfile->mh));
            close_file(musicfile);
            return NULL;
        }
        if (!flac_dec_process_metadata(musicfile->mh)) {
            fprintf(stderr, "error: flac_dec_process_metadata: %s\n", flac_dec_get_state_string(musicfile->mh));
            close_file(musicfile);
            return NULL;
        }
        if (!musicfile->channels || !musicfile->rate) {
            fprintf(stderr, "error: flac_dec_process_metadata: flac metadata not found\n");
            close_file(musicfile);
            return NULL;
        }
        if (flac_metadata_get_tags(musicfile->path, &tags)) {
            for (ctr = 0; ctr < tags->data.vorbis_comment.num_comments; ctr++) {
                comment = tags->data.vorbis_comment.comments[ctr].entry;
                if (!strncasecmp(comment, vorbis_artist, sizeof(vorbis_artist) - 1)) {
                    if (musicfile->artist) {
                        free(musicfile->artist);
                    }
                    musicfile->artist = strdup(comment + sizeof(vorbis_artist) - 1);
                } else if (!strncasecmp(comment, vorbis_title, sizeof(vorbis_title) - 1)) {
                    if (musicfile->title) {
                        free(musicfile->title);
                    }
                    musicfile->title = strdup(comment + sizeof(vorbis_title) - 1);
                } else if (!strncasecmp(comment, vorbis_album, sizeof(vorbis_album) - 1)) {
                    if (musicfile->album) {
                        free(musicfile->album);
                    }
                    musicfile->album = strdup(comment + sizeof(vorbis_album) - 1);
                }
            }
            flac_metadata_destroy(tags);
        }

        return musicfile;
    }
#endif

    err = mpg123_init();
    mp = mpg123_new_pars(&result);

    /*
    if (codec->codec == 0) {
        //mpg123_par(mp, MPG123_DOWN_SAMPLE, downsample, 0);
        mpg123_par(mp, MPG123_DOWN_SAMPLE, 3 - codec->format, 0);
    } else {
        mpg123_par(mp, MPG123_DOWN_SAMPLE, 0, 0);
    }
    */
    if (err != MPG123_OK) {
        fprintf(stderr, "error: mpg123_init: trouble with mpg123: %s\n", mpg123_plain_strerror(err));
        return NULL;
    }
    //if ((mh = mpg123_new(NULL, &err)) == NULL) {
    if (!(musicfile->mh = mpg123_parnew(mp, NULL, &err))) {
        fprintf(stderr, "error: mpg123_parnew: could not create mpg123 handle\n");
        return NULL;
    }
    if (mpg123_open(musicfile->mh, musicfile->path) != MPG123_OK) {
        fprintf(stderr, "error: mpg123_open: could not open %s\n", musicfile->path);
        close_file(musicfile);
        return NULL;
    }
    if (mpg123_getformat(musicfile->mh, &rate, &channels, &encoding) != MPG123_OK) {
        fprintf(stderr, "error: mpg123_getformat: could not get format\n");
        close_file(musicfile);
        return NULL;
    }
    /*
    if ((codec->codec == 0 && rate != codec->rate) || (codec->codec == 3 && rate != 44100)) {
        fprintf( stderr, "error: sample rate %lu not supported\n", rate);
        close_file(musicfile);
        return NULL;
    }
    */
    if (encoding != MPG123_ENC_SIGNED_16) {
        fprintf(stderr, "error: mpg123_getformat: unknown encoding: 0x%02x\n", encoding);
        close_file(musicfile);
        return NULL;
    }
    mpg123_format_none(musicfile->mh);
    mpg123_format(musicfile->mh, (musicfile->rate = rate), (musicfile->channels = channels), encoding);

    return musicfile;
}

int get_pcm_frame(musicfile *musicfile, int channels, int16_t *buf, int *pcmread) {
    unsigned char readbuffer[65536];
    int16_t *readptr;
    size_t numdecoded;
    int err;
    int ctr;

    if (musicfile->channels == 1) {
        channels = 1;
    }
#if HAVE_VORBIS
    if (musicfile->vorbis) {
        int ret;

        while (pcmd.len < channels**pcmread && (ret = ov_read(musicfile->mh, (void *)readbuffer, sizeof(pcmd.buf) - pcmd.len, 0, 2, 1, NULL)) > 0) {
            if (debug) {
                fprintf(stderr, "got %i pcm bytes from vorbis\n", ret);
            }
            memcpy((void *)pcmd.buf + pcmd.len, readbuffer, ret);
            pcmd.len += ret;
        }
        if (debug) {
            fprintf(stderr, "%u pcm bytes in buffer\n", pcmd.len);
        }
        if (!pcmd.len) {
            return false;
        }
        if (channels == 1) {
            memcpy(buf, pcmd.buf, *pcmread);
        } else for (ctr = 0; ctr < *pcmread/2; ctr++) {
            buf[ctr] = (pcmd.buf[ctr*2] + pcmd.buf[ctr*2+1]) / 2;
        }
        if (pcmd.len < channels**pcmread) {
            pcmd.len = 0;
        } else {
            memmove(pcmd.buf, (void *)pcmd.buf + channels**pcmread, pcmd.len - channels**pcmread);
            pcmd.len -= channels**pcmread;
        }
        return true;
    }
#endif
#if HAVE_FLAC
    if (musicfile->flac) {
        pcmd.len = 0;
        while (flac_dec_get_state(musicfile->mh) != FLAC_DEC_END_OF_STREAM && flac_dec_process_single(musicfile->mh)) {
            if (debug) {
                fprintf(stderr, "got %u pcm bytes from flac\n", pcmd.len);
                fprintf(stderr, "flac state: %s\n", flac_dec_get_state_string(musicfile->mh));
            }
            if (!pcmd.len) {
                continue;
            }
            if (channels == 1) {
                memcpy(buf, pcmd.buf, pcmd.len);
            } else for (ctr = 0; ctr < pcmd.len/2; ctr++) {
                buf[ctr] = (pcmd.buf[ctr*2] + pcmd.buf[ctr*2+1]) / 2;
            }
            *pcmread = pcmd.len / channels;
            return true;
        }
        return false;
    }
#endif
    memset(readbuffer, 0, *pcmread);
    if (debug) {
        fprintf(stderr, "getting %d*%d pcm bytes from mp3\n", channels, *pcmread);
    }
    if ((err = mpg123_read(musicfile->mh, readbuffer, channels**pcmread, &numdecoded)) != MPG123_DONE) {
        if (err != MPG123_OK) {
            fprintf(stderr, "got error : %d!\n", err);
            return false;
        }
        readptr = (int16_t *)readbuffer;
        if (channels == 1) { // no channels to mix; this is for opus stereo mode
            memcpy(buf, readbuffer, *pcmread);
        } else for (ctr = 0; ctr < *pcmread/2; ctr++) {
            buf[ctr] = (readptr[ctr*2] + readptr[ctr*2+1]) / 2;
        }
        return true;
    }
    return false;
}

void close_file(musicfile *musicfile) {
    if (!musicfile || !musicfile->mh) {
        return;
    }
#if HAVE_VORBIS
    if (musicfile->vorbis) {
        ov_clear(musicfile->mh);
        free(musicfile->mh);
        musicfile->mh = NULL;
        return;
    }
#endif
#if HAVE_FLAC
    if (musicfile->flac) {
        flac_dec_destroy(musicfile->mh);
        musicfile->mh = NULL;
        return;
    }
#endif
    mpg123_close(musicfile->mh);
    mpg123_delete(musicfile->mh);
    musicfile->mh = NULL;
    mpg123_exit();
}

int get_random_number(int min, int max) {
    max = max - min + 1;
    return min + (int)( ((float)max) * rand() / ( RAND_MAX + 1.0 ) );
}

void shuffle_musiclist(void) {
    musicfile *temp;
    int i, j;
    for (i = 0; i < musicfile_count; ++i) {
        j = get_random_number(0, musicfile_count-1);
        if (i != j) {
            temp = musiclist[i];
            musiclist[i] = musiclist[j];
            musiclist[j] = temp;
        }
    }
}

typedef struct _channel_node {
    char name[40];
    uint16_t id;
    struct _channel_node *children[200];
    int childcount;
} channel_node;

channel_node *find_channel(channel_node *node, uint16_t id) {
    int i = 0;
    channel_node *p = NULL;
    if (node->id == id) return node;
    while (i < node->childcount) {
        p = find_channel(node->children[i], id);
        if (p) return p;
        ++i;
    }
    return NULL;
}

void add_channel(channel_node *lobby_node, v3_channel *chan) {
    channel_node *p = lobby_node;
    if (chan->parent) {
        p = find_channel(lobby_node, chan->parent);
        if (!p) p = lobby_node;
    }
    if (p) {
        p->children[p->childcount] = malloc(sizeof(channel_node));
        memset(p->children[p->childcount], 0, sizeof(channel_node));
        if (p->children[p->childcount]) {
            strncpy(p->children[p->childcount]->name, chan->name, 40);
            p->children[p->childcount]->id = chan->id;
            ++p->childcount;
        }
    } else {
        //if (debug)
        fprintf(stderr, "error adding channel %u to tree.\n", chan->id);
    }
}

void print_channels(channel_node *node, int indentlevel) {
    int i;
    for (i = 0; i < indentlevel; ++i) fprintf(stdout, "    ");
    fprintf(stdout, "%d: %s\n", node->id, node->name);
    for (i = 0; i < node->childcount; ++i) print_channels(node->children[i], indentlevel+1);
}

void free_channels(channel_node *node) {
    int i;
    for (i = 0; i < node->childcount; ++i) free_channels(node->children[i]);
    free(node);
}

int select_channel(void) {
    int ctr;
    v3_channel *c;
    char buf[16];
    channel_node *chantree = malloc(sizeof(channel_node));
    memset(chantree, 0, sizeof(channel_node));
    strcpy(chantree->name, "(Lobby)");
    chantree->id = 0;
    chantree->childcount = 0;

    for (ctr = 0; ctr < 32768; ctr++) {
        if ((c = v3_get_channel(ctr))) {
            //fprintf(stdout, "%d: %s\n", c->id, c->name);
            add_channel(chantree, c);
            v3_free_channel(c);
        }
    }
    print_channels(chantree, 0);
    free_channels(chantree);
    fprintf(stdout, "Enter a channel id: ");
    fgets(buf, 15, stdin);
    return atoi(buf);
}

int
main(int argc, char **argv) {
    int opt;
    pthread_t player;
    struct _conninfo conninfo;
    int shuffle = true;
    _v3_net_message *msg;

    memset(&conninfo, 0, sizeof(conninfo));
    conninfo.volume = 1;
    while ((opt = getopt(argc, argv, "dh:p:u:c:nsv:")) != -1) {
        switch (opt) {
            case 'd':
                debug++;
                break;
            case 'h':
                conninfo.server = optarg;
                break;
            case 'u':
                conninfo.username = optarg;
                break;
            case 'c':
                conninfo.channelid = optarg;
                break;
            case 'v':
                opt = atoi(optarg);
                if (opt < 0) {
                    opt = 0;
                } else if (opt > 100) {
                    opt = 100;
                }
                conninfo.volume = opt / 100.0;
                break;
            case 'p':
                conninfo.password = optarg;
                break;
            case 'n':
                shuffle = false;
                break;
            case 's':
                disable_stereo = true;
                break;
        }
    }
    if (! conninfo.server)  {
        fprintf(stderr, "error: server hostname (-h) was not specified\n");
        usage(argv);
    }
    if (! conninfo.username)  {
        fprintf(stderr, "error: username (-u) was not specified\n");
        usage(argv);
    }
    if (! conninfo.password) {
        conninfo.password = "";
    }
    if (optind >= argc) {
        fprintf(stderr, "error: path to music library not specified\n");
        usage(argv);
    }
    conninfo.path = argv[argc-1];
    media_pathlen = strlen(conninfo.path);
    fprintf(stderr, "server: %s\nusername: %s\nmedia path: %s\n", conninfo.server, conninfo.username, conninfo.path);
    scan_media_path(conninfo.path);
    fprintf(stderr, "found %d files in music path\n", musicfile_count);
    if (!musicfile_count) {
        return EXIT_FAILURE;
    }
    if (shuffle) {
        srand(time(NULL));
        shuffle = get_random_number(2, 5);
        while (shuffle--) {
            shuffle_musiclist();
        }
    } else {
        fprintf(stderr, "not shuffling musiclist\n");
    }
    if (!disable_stereo) {
        fprintf(stderr, "will use 2 channels for the Opus codec (replacing channel codecs truespeech and lernout & hauspie)\n");
    }
    if (debug >= 2) {
        v3_debuglevel(V3_DEBUG_ALL);
    }
    v3_get_event(V3_NONBLOCK);
    if (!v3_login(conninfo.server, conninfo.username, conninfo.password, "")) {
        fprintf(stderr, "failed to login to server: %s\n", _v3_error(NULL));
        return EXIT_FAILURE;
    }
    signal(SIGINT, ctrl_c);
    pthread_create(&player, NULL, jukebox_player, &conninfo);
    while ((msg = _v3_recv(V3_BLOCK))) {
        _v3_process_message(msg);
    }

    return EXIT_SUCCESS;
}

uint64_t timediff(const struct timeval *left, const struct timeval *right) {
    int64_t ret, lval, rval;
    lval = left->tv_sec * 1000000 + left->tv_usec;
    rval = right->tv_sec * 1000000 + right->tv_usec;
    ret = rval - lval;
    return ret;
}

