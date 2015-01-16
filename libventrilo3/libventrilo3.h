/*
 * vim: softtabstop=4 shiftwidth=4 cindent foldmethod=marker expandtab
 *
 * $LastChangedDate: 2012-05-24 22:31:51 +0430 (Thu, 24 May 2012) $
 * $Revision: 1166 $
 * $LastChangedBy: Haxar $
 * $URL: http://svn.mangler.org/mangler/trunk/libventrilo3/libventrilo3.h $
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

#ifndef _LIBVENTRILO3_H
#define _LIBVENTRILO3_H

#ifndef NO_AUTOMAKE
#include "config.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>

#ifndef _WIN32
#include <sys/time.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>
#else
#include <winsock2.h>
#include <windows.h>
#include <pthread.h>

#define SHUT_WR SD_SEND 
#endif

#define true  1
#define false 0

char *_v3_errors[] = {
    "Success",
    "Operation can not be performed on your self.",
    "Specified object no longer exists.",
    "Invalid password.",
    "You do not have server admin rights.",
    "You do not have channel admin rights.",
    "Bad name specified.",
    "Insufficient resources to complete operation.",
    "Specified name is already taken.",
    "You are banned from the specified channel.",
    "The channel is full.",
    "You do not have admin permission.",
    "You do not have root channel admin permission.",
    "State change does not apply in this instance.",
    "Max channel width exceeded.",
    "Max channel depth exceeded.",
    "Ban failed. Might already be banned by subnet.",
    "Unable to update the USR file.",
    "The server requires that you enable the 'Show login name in remote status request' of your server definition for this server.",
    "Maximum number of allowed guest logins has been reached. Try again later.",
    "Connection time limit (GUEST or INI) has been exceeded.",
    "You are not allowed to switch channels manually.",
    "You do not have sufficient rights to ban another user.",
    "You do not have sufficient rights to kick another user.",
    "You do not have sufficient rights to move another user to a different channel.",
    "You already have Server Admin rights.",
    "You do not have sufficient rights to issue RCon commands.",
    "You are not allowed to edit the regular MOTD.",
    "You are not allowed to edit the guest MOTD.",
    "You do not have sufficient rights to receive complaints.",
    "You do not have sufficient rights to send complaints.",
    "Another client is currently using the User Editor. See name: ",
    "You have not been given access rights to enter a User Authorization mode channel.",
    "Another client is currently editing the Rank list. See name: ",
    "Account locked. Reason: ",
    "Guest accounts are currently locked. Reason: ",
    "You do not have the 'Add user' right.",
    "You have been kicked off the server. Reason: ",
    "You have been kicked from the server for spamming.",
    "You have been kicked from the server. Duplicate names are not allowed.",
    "You have been kicked from the server. Duplicate IP's are not allowed for your account.",
    "You have been kicked from the server. Duplicate IP limit for your account has been reached.",
    "You have been kicked for inactivity.",
    "You have been kicked by the console.",
    "You have been kicked because your account on this server has been deleted.",
    "You have been kicked because you provided a bad Server Admin password. You will be automatically banned if you fail several more times, so be sure that you are entering the correct password before you try again. Be advised that your attempts to use the Server Admin password are being logged. If you are not authorized to be a Server Admin you may be subject to legal action.",
    "You have been banned from the server. Reason: ",
    "You have been banned from the server for spamming.",
    "You are temporarily banned.",
    "You are on the banned list.",
    "Guest accounts are not allowed to join the specified channel."
};

char *_v3_page_errors[] = {
    "User does not exist.",
    "User is not accepting pages.",
    "Users current channel does not allow for paging.",
    "Insufficient access rights."
};

char *_v3_move_errors[] = {
    "Admin rights.",
    "Invalid source object.",
    "Invalid target object.",
    "Not support in given mode.",
    "Target channel is full.",
    "Target channel does not allow phantoms, or source does not have proper access rights."
};

char *_v3_server_disabled_errors[] = {
    "This server has not been authenticated yet. Please try connecting again in a few minutes.",
    "This server has not been configured properly. Please contact your hosting company.",
    "This server has been remotely disabled.",
    "This server has been disabled by order of the Department of Homeland Security.",
    "This server has been disabled by order of the Federal Bureau of Investigation.",
    "This server has been disabled by order of the United States Government.",
    "This server has been disabled by order of the United States Secret Service.",
    "This server has been disabled by order of the goverment in which the server is hosted.",
    "This server has been disabled by order of the United States Military."
};

char *_v3_phantom_errors[] = {
    "Channel does not allow phantoms.",
    "Phantoms are restricted to USR file.",
    "Duplicate.",
    "Unknown channel.",
    "Not found.",
    "Not authenticated for specified channel.",
    "No slots available.",
    "Channel is full.",
    "Insufficient resources.",
    "Banned from channel.",
    "Insufficient access rights.",
};

char *_v3_permissions_errors[] = {
    "Specified ID is no longer valid.",
    "Duplicate name.",
    "Server reported an error.",
    "All user accounts must have a valid password.",
    "Missing required fields.",
    "Not allowed to delete Guest account.",
    "The specified login name is not allowed.",
    "The specified login name contains characters that are not allowed.",
    "Insufficient access rights to complete the request.",
    NULL,
    "Specified alternate ID is not valid.",
    "Specified new owner does not have 'Add Users' rights.",
    "The user account database is full. You must delete an existing account before you can add a new one.",
    "The Guest account can not be locked without having at least one other account defined and is not locked it self. Otherwise, you would be locking everyone out of the server.",
};

char *_v3_ranks_errors[] = {
    "Invalid ID specified.",
    "Duplicate Abbreviation.",
    "Duplicate Description.",
    "Insufficient access rights.",
    NULL,
    "Maximum number of allowed Ranks has been reached.",
    "Missing abbreviation or description.",
};

char *_v3_bitmasks[] = {
    "0.0.0.0",
    "128.0.0.0",
    "192.0.0.0",
    "224.0.0.0",
    "240.0.0.0",
    "248.0.0.0",
    "252.0.0.0",
    "254.0.0.0",
    "255.0.0.0",
    "255.128.0.0",
    "255.192.0.0",
    "255.224.0.0",
    "255.240.0.0",
    "255.248.0.0",
    "255.252.0.0",
    "255.254.0.0",
    "255.255.0.0",
    "255.255.128.0",
    "255.255.192.0",
    "255.255.224.0",
    "255.255.240.0",
    "255.255.248.0",
    "255.255.252.0",
    "255.255.254.0",
    "255.255.255.0",
    "255.255.255.128",
    "255.255.255.192",
    "255.255.255.224",
    "255.255.255.240",
    "255.255.255.248",
    "255.255.255.252",
    "255.255.255.254",
    "255.255.255.255",
    NULL
};

#define V3_OK        0
#define V3_MALFORMED 1
#define V3_NOTIMPL   2
#define V3_FAILURE   3

#define V3_NOT_WAITING   0
#define V3_EVENT_WAITING 1
#define V3_MSG_WAITING   2
#define V3_BOTH_WAITING  3

/*
 * This structure defines the bit number of each permission setting, the
 * internal name, and a name suitable for display to a user.
 */
struct _v3_perm_info v3_perm_info[]  = {
    { 1,      "V3_PERM_LOCK_ACCT",               "Lock Account"                             },
    { 2,      "V3_PERM_DFL_CHAN",                "Access Default Channel"                   },
    { 3,      "V3_PERM_DUPE_IP",                 "DUPE_IP"                                  },
    { 4,      "V3_PERM_SWITCH_CHAN",             "Switch Channels"                          },
    { 5,      "V3_PERM_IN_RESERVE_LIST",         "IN_RESERVE_LIST"                          },
    { 6,      "V3_PERM_RECV_BCAST",              "Receive Broadcasts"                       },
    { 7,      "V3_PERM_ADD_PHANTOM",             "ADD_PHANTOM"                              },
    { 8,      "V3_PERM_RECORD",                  "Record Audio"                             },
    { 9,      "V3_PERM_RECV_COMPLAINT",          "Receive Complaint"                        },
    { 10,     "V3_PERM_SEND_COMPLAINT",          "Send Complaint"                           },
    { 11,     "V3_PERM_INACTIVE_EXEMPT",         "Exempt from Inactivity Timer"             },
    { 12,     "V3_PERM_SRV_ADMIN",               "Server Admin"                             },
    { 13,     "V3_PERM_ADD_USER",                "Add Users"                                },
    { 14,     "V3_PERM_DEL_USER",                "Delete Users"                             },
    { 15,     "V3_PERM_BAN_USER",                "Ban a User"                               },
    { 16,     "V3_PERM_KICK_USER",               "Kick a User"                              },
    { 17,     "V3_PERM_MOVE_USER",               "Move a User's Channel"                    },
    { 18,     "V3_PERM_ASSIGN_CHAN_ADMIN",       "Assign Channel Admins"                    },
    { 19,     "V3_PERM_EDIT_RANK",               "Edit User Ranks"                          },
    { 20,     "V3_PERM_EDIT_MOTD",               "Edit the MOTD"                            },
    { 21,     "V3_PERM_EDIT_GUEST_MOTD",         "Edit the Guest MOTD"                      },
    { 22,     "V3_PERM_ISSUE_RCON_CMD",          "ISSUE_RCON_CMD"                           },
    { 23,     "V3_PERM_EDIT_VOICE_TARGET",       "EDIT_VOICE_TARGET"                        },
    { 24,     "V3_PERM_EDIT_COMMAND_TARGET",     "EDIT_COMMAND_TARGET"                      },
    { 25,     "V3_PERM_ASSIGN_RANK",             "Assign a Rank"                            },
    { 26,     "V3_PERM_ASSIGN_RESERVED",         "ASSIGN_RESERVED"                          },
    { 27,     "V3_PERM_BCAST",                   "Send Broadcasts"                          },
    { 28,     "V3_PERM_BCAST_LOBBY",             "Send Broadcast to Lobby"                  },
    { 29,     "V3_PERM_BCAST_USER",              "Send Broadcast to User"                   },
    { 30,     "V3_PERM_BCAST_X_CHAN",            "Send Cross Channel Broadcasts"            },
    { 31,     "V3_PERM_SEND_TTS_BIND",           "SEND_TTS_BIND"                            },
    { 32,     "V3_PERM_SEND_WAV_BIND",           "SEND_WAV_BIND"                            },
    { 33,     "V3_PERM_SEND_PAGE",               "Send Pages"                               },
    { 34,     "V3_PERM_SEND_COMMENT",            "Send Comments"                            },
    { 35,     "V3_PERM_SET_PHON_NAME",           "Set Phonetic Name"                        },
    { 36,     "V3_PERM_GEN_COMMENT_SNDS",        "GEN_COMMENT_SNDS"                         },
    { 37,     "V3_PERM_EVENT_SNDS",              "EVENT_SNDS"                               },
    { 38,     "V3_PERM_MUTE_GLBL",               "MUTE_GLBL"                                },
    { 39,     "V3_PERM_MUTE_OTHER",              "MUTE_OTHER"                               },
    { 40,     "V3_PERM_GLBL_CHAT",               "GLBL_CHAT"                                },
    { 41,     "V3_PERM_START_PRIV_CHAT",         "START_PRIV_CHAT"                          },
    { 42,     "V3_PERM_EQ_OUT",                  "EQ_OUT"                                   },
    { 43,     "V3_PERM_SEE_GUEST",               "See Guests"                               },
    { 44,     "V3_PERM_SEE_NONGUEST",            "See Registered Useres"                    },
    { 45,     "V3_PERM_SEE_MOTD",                "See Message of the Day"                   },
    { 46,     "V3_PERM_SEE_SRV_COMMENT",         "See Server Comment"                       },
    { 47,     "V3_PERM_SEE_CHAN_LIST",           "See Channel List"                         },
    { 48,     "V3_PERM_SEE_CHAN_COMMENT",        "See Channel Comments"                     },
    { 49,     "V3_PERM_SEE_USER_COMMENT",        "See User Comments"                        }
};

uint32_t _v3_hash_table[] =
{
    0x00000000, 0x77073096, 0xee0e612c, 0x990951ba,
    0x076dc419, 0x706af48f, 0xe963a535, 0x9e6495a3,
    0x0edb8832, 0x79dcb8a4, 0xe0d5e91e, 0x97d2d988,
    0x09b64c2b, 0x7eb17cbd, 0xe7b82d07, 0x90bf1d91,
    0x1db71064, 0x6ab020f2, 0xf3b97148, 0x84be41de,
    0x1adad47d, 0x6ddde4eb, 0xf4d4b551, 0x83d385c7,
    0x136c9856, 0x646ba8c0, 0xfd62f97a, 0x8a65c9ec,
    0x14015c4f, 0x63066cd9, 0xfa0f3d63, 0x8d080df5,
    0x3b6e20c8, 0x4c69105e, 0xd56041e4, 0xa2677172,
    0x3c03e4d1, 0x4b04d447, 0xd20d85fd, 0xa50ab56b,
    0x35b5a8fa, 0x42b2986c, 0xdbbbc9d6, 0xacbcf940,
    0x32d86ce3, 0x45df5c75, 0xdcd60dcf, 0xabd13d59,
    0x26d930ac, 0x51de003a, 0xc8d75180, 0xbfd06116,
    0x21b4f4b5, 0x56b3c423, 0xcfba9599, 0xb8bda50f,
    0x2802b89e, 0x5f058808, 0xc60cd9b2, 0xb10be924,
    0x2f6f7c87, 0x58684c11, 0xc1611dab, 0xb6662d3d,
    0x76dc4190, 0x01db7106, 0x98d220bc, 0xefd5102a,
    0x71b18589, 0x06b6b51f, 0x9fbfe4a5, 0xe8b8d433,
    0x7807c9a2, 0x0f00f934, 0x9609a88e, 0xe10e9818,
    0x7f6a0dbb, 0x086d3d2d, 0x91646c97, 0xe6635c01,
    0x6b6b51f4, 0x1c6c6162, 0x856530d8, 0xf262004e,
    0x6c0695ed, 0x1b01a57b, 0x8208f4c1, 0xf50fc457,
    0x65b0d9c6, 0x12b7e950, 0x8bbeb8ea, 0xfcb9887c,
    0x62dd1ddf, 0x15da2d49, 0x8cd37cf3, 0xfbd44c65,
    0x4db26158, 0x3ab551ce, 0xa3bc0074, 0xd4bb30e2,
    0x4adfa541, 0x3dd895d7, 0xa4d1c46d, 0xd3d6f4fb,
    0x4369e96a, 0x346ed9fc, 0xad678846, 0xda60b8d0,
    0x44042d73, 0x33031de5, 0xaa0a4c5f, 0xdd0d7cc9,
    0x5005713c, 0x270241aa, 0xbe0b1010, 0xc90c2086,
    0x5768b525, 0x206f85b3, 0xb966d409, 0xce61e49f,
    0x5edef90e, 0x29d9c998, 0xb0d09822, 0xc7d7a8b4,
    0x59b33d17, 0x2eb40d81, 0xb7bd5c3b, 0xc0ba6cad,
    0xedb88320, 0x9abfb3b6, 0x03b6e20c, 0x74b1d29a,
    0xead54739, 0x9dd277af, 0x04db2615, 0x73dc1683,
    0xe3630b12, 0x94643b84, 0x0d6d6a3e, 0x7a6a5aa8,
    0xe40ecf0b, 0x9309ff9d, 0x0a00ae27, 0x7d079eb1,
    0xf00f9344, 0x8708a3d2, 0x1e01f268, 0x6906c2fe,
    0xf762575d, 0x806567cb, 0x196c3671, 0x6e6b06e7,
    0xfed41b76, 0x89d32be0, 0x10da7a5a, 0x67dd4acc,
    0xf9b9df6f, 0x8ebeeff9, 0x17b7be43, 0x60b08ed5,
    0xd6d6a3e8, 0xa1d1937e, 0x38d8c2c4, 0x4fdff252,
    0xd1bb67f1, 0xa6bc5767, 0x3fb506dd, 0x48b2364b,
    0xd80d2bda, 0xaf0a1b4c, 0x36034af6, 0x41047a60,
    0xdf60efc3, 0xa867df55, 0x316e8eef, 0x4669be79,
    0xcb61b38c, 0xbc66831a, 0x256fd2a0, 0x5268e236,
    0xcc0c7795, 0xbb0b4703, 0x220216b9, 0x5505262f,
    0xc5ba3bbe, 0xb2bd0b28, 0x2bb45a92, 0x5cb36a04,
    0xc2d7ffa7, 0xb5d0cf31, 0x2cd99e8b, 0x5bdeae1d,
    0x9b64c2b0, 0xec63f226, 0x756aa39c, 0x026d930a,
    0x9c0906a9, 0xeb0e363f, 0x72076785, 0x05005713,
    0x95bf4a82, 0xe2b87a14, 0x7bb12bae, 0x0cb61b38,
    0x92d28e9b, 0xe5d5be0d, 0x7cdcefb7, 0x0bdbdf21,
    0x86d3d2d4, 0xf1d4e242, 0x68ddb3f8, 0x1fda836e,
    0x81be16cd, 0xf6b9265b, 0x6fb077e1, 0x18b74777,
    0x88085ae6, 0xff0f6a70, 0x66063bca, 0x11010b5c,
    0x8f659eff, 0xf862ae69, 0x616bffd3, 0x166ccf45,
    0xa00ae278, 0xd70dd2ee, 0x4e048354, 0x3903b3c2,
    0xa7672661, 0xd06016f7, 0x4969474d, 0x3e6e77db,
    0xaed16a4a, 0xd9d65adc, 0x40df0b66, 0x37d83bf0,
    0xa9bcae53, 0xdebb9ec5, 0x47b2cf7f, 0x30b5ffe9,
    0xbdbdf21c, 0xcabac28a, 0x53b39330, 0x24b4a3a6,
    0xbad03605, 0xcdd70693, 0x54de5729, 0x23d967bf,
    0xb3667a2e, 0xc4614ab8, 0x5d681b02, 0x2a6f2b94,
    0xb40bbe37, 0xc30c8ea1, 0x5a05df1b, 0x2d02ef8d
};

/*
 * Global Variables
 */
_v3_server  v3_server  = { 0, 0, 0, 0, 0, NULL, NULL, NULL, NULL, NULL, NULL, NULL, -1, { 0, 0 }, 0, 0, { "", 0, 0 }, { "", 0, 0 }, NULL, NULL, {0, 0},  0, 0, 0, 0, false, false, false};
_v3_luser   v3_luser   = { -1, NULL, NULL, NULL, NULL, NULL, NULL, 0, 0, 0, 0, 0, {  } };

v3_channel              *v3_channel_list = NULL;
int                     _v3_channel_count;

v3_user                 *v3_user_list = NULL;
int                     _v3_user_count;

v3_rank                 *v3_rank_list = NULL;
int                     _v3_rank_count;

v3_account              *v3_account_list = NULL;
int                     _v3_account_count;

v3_server_prop          _v3_server_prop = { 0 };
uint16_t                _v3_server_prop_tid = 0;

char                    _v3_error_text[256];
char                    _v3_status_text[256];
int                     _v3_sockd = -1;
uint32_t                _v3_debuglevel = V3_DEBUG_NONE;
uint32_t                _v3_user_loggedin = 0;

pthread_mutex_t         *userlist_mutex = NULL;
pthread_mutex_t         *channellist_mutex = NULL;
pthread_mutex_t         *ranklist_mutex = NULL;
pthread_mutex_t         *accountlist_mutex = NULL;
pthread_mutex_t         *server_mutex = NULL;
pthread_mutex_t         *luser_mutex = NULL;
pthread_mutex_t         *sendq_mutex = NULL;

pthread_mutex_t         *eventq_mutex = NULL;
pthread_cond_t          *eventq_cond = NULL;

v3_event                *_v3_eventq = NULL;

#if HAVE_SPEEX_DSP
pthread_mutex_t         *audioq_mutex = NULL;

void                    *_v3_resampler = NULL;
uint32_t                _v3_in_rate = 0;
uint32_t                _v3_out_rate = 0;
uint8_t                 _v3_channels = 0;
int                     _v3_pcm_pipe[2] = { -1, -1 };
uint32_t                _v3_pcm_write = 0;
#endif

v3_codec v3_codecs[] = {
#if HAVE_GSM
    { 0, 0, 640, 8000, -1, "GSM 6.10 8kHz" },
    { 0, 1, 640, 11025, -1, "GSM 6.10 11kHz" },
    { 0, 2, 640, 22050, -1, "GSM 6.10 22kHz" },
    { 0, 3, 640, 44100, -1, "GSM 6.10 44kHz" },
#endif
#if HAVE_OPUS
    { 1, 0, 960*2, 48000, -1, "Opus 48kHz" },
    { 2, 0, 960*2, 48000, -1, "Opus 48kHz" },
#endif
#if HAVE_SPEEX
    { 3, 0, 320, 8000, 0, "Speex 8kHz Quality 0" },
    { 3, 1, 320, 8000, 1, "Speex 8kHz Quality 1" },
    { 3, 2, 320, 8000, 2, "Speex 8kHz Quality 2" },
    { 3, 3, 320, 8000, 3, "Speex 8kHz Quality 3" },
    { 3, 4, 320, 8000, 4, "Speex 8kHz Quality 4" },
    { 3, 5, 320, 8000, 5, "Speex 8kHz Quality 5" },
    { 3, 6, 320, 8000, 6, "Speex 8kHz Quality 6" },
    { 3, 7, 320, 8000, 7, "Speex 8kHz Quality 7" },
    { 3, 8, 320, 8000, 8, "Speex 8kHz Quality 8" },
    { 3, 9, 320, 8000, 9, "Speex 8kHz Quality 9" },
    { 3, 10, 320, 8000, 10, "Speex 8kHz Quality 10" },
    { 3, 11, 640, 16000, 0, "Speex 16kHz Quality 0" },
    { 3, 12, 640, 16000, 1, "Speex 16kHz Quality 1" },
    { 3, 13, 640, 16000, 2, "Speex 16kHz Quality 2" },
    { 3, 14, 640, 16000, 3, "Speex 16kHz Quality 3" },
    { 3, 15, 640, 16000, 4, "Speex 16kHz Quality 4" },
    { 3, 16, 640, 16000, 5, "Speex 16kHz Quality 5" },
    { 3, 17, 640, 16000, 6, "Speex 16kHz Quality 6" },
    { 3, 18, 640, 16000, 7, "Speex 16kHz Quality 7" },
    { 3, 19, 640, 16000, 8, "Speex 16kHz Quality 8" },
    { 3, 20, 640, 16000, 9, "Speex 16kHz Quality 9" },
    { 3, 21, 640, 16000, 10, "Speex 16kHz Quality 10" },
    { 3, 22, 1280, 32000, 0, "Speex 32kHz Quality 0" },
    { 3, 23, 1280, 32000, 1, "Speex 32kHz Quality 1" },
    { 3, 24, 1280, 32000, 2, "Speex 32kHz Quality 2" },
    { 3, 25, 1280, 32000, 3, "Speex 32kHz Quality 3" },
    { 3, 26, 1280, 32000, 4, "Speex 32kHz Quality 4" },
    { 3, 27, 1280, 32000, 5, "Speex 32kHz Quality 5" },
    { 3, 28, 1280, 32000, 6, "Speex 32kHz Quality 6" },
    { 3, 29, 1280, 32000, 7, "Speex 32kHz Quality 7" },
    { 3, 30, 1280, 32000, 8, "Speex 32kHz Quality 8" },
    { 3, 31, 1280, 32000, 9, "Speex 32kHz Quality 9" },
    { 3, 32, 1280, 32000, 10, "Speex 32kHz Quality 10" },
#endif
    { -1, -1, 0, 0, -1, "Unsupported Codec" },
};

typedef struct __v3_decoders {
#if HAVE_GSM
    void *gsm;
#endif
#if HAVE_SPEEX
    uint32_t speexrate;
    void *speex;
#endif
#if HAVE_OPUS
    void *opus;
#endif
    int _pad_;
} _v3_decoders;

_v3_decoders v3_decoders[65535];

uint8_t _v3_user_volumes[65535];
uint8_t _v3_master_volume = 79;
uint8_t _v3_xmit_volume = 79;

// TODO: it's too messy to have this here. move into new lv3 repo.
/*{{{*/
#define V3_VRF_HEADID       "VENTRECD"
#define V3_VRF_TEMPID       "TEMPRECD"

#define V3_VRF_VERSION      0x01

#define V3_VRF_HEADLEN      0x28
#define V3_VRF_INFOLEN      0x2910
#define V3_VRF_TEXTLEN      0x100

#define V3_VRF_TYPE_AUDIO   0x01
#define V3_VRF_TYPE_TEXT    0x02
#define V3_VRF_TYPE_EXT     0x03

#define V3_VRF_MAX_FRAGLEN  1 << 11 // 1105

enum _v3_vrf_events {
    V3_VRF_EVENT_DATA_FLUSH = 1,
    V3_VRF_EVENT_AUDIO_DATA,
    V3_VRF_EVENT_AUDIO_STOP,
    V3_VRF_EVENT_TEXT_DATA
};

typedef struct _v3_vrf_header   _v3_vrf_header;
typedef struct _v3_vrf_segment  _v3_vrf_segment;
typedef struct _v3_vrf_audio    _v3_vrf_audio;
typedef struct _v3_vrf_fragment _v3_vrf_fragment;
typedef struct _v3_vrf_rec      _v3_vrf_rec;
typedef struct _v3_vrf          _v3_vrf;

struct _v3_vrf_header {         // 10552
    char     headid[8];         // 0    - header id (VENTRECD)
    uint32_t size;              // 8    - file size
    uint32_t headlen;           // 12   - header length (0x28 / 40)
    uint32_t unknown1;          // 16   - unused
    uint32_t segtable;          // 20   - segment table offset
    uint32_t segcount;          // 24   - segment count
    uint32_t vrfversion;        // 28   - vrf version
    uint32_t unknown2;          // 32   - unused
    uint32_t unknown3;          // 36   - unused
    uint32_t infolen;           // 40   - info length (0x2910 / 10512)
    uint32_t codec;             // 44   - codec
    uint32_t codecformat;       // 48   - codec format
    uint32_t unknown4;          // 52   - unused
    char     platform[64];      // 56   - platform
    char     version[64];       // 120  - client version
    char     username[128];     // 184  - user name
    char     comment[8192];     // 312  - info comment
    char     url[1024];         // 8504 - info url
    char     copyright[1024];   // 9528 - info copyright
} __attribute__ ((__packed__));

struct _v3_vrf_segment {        // 64
    uint32_t headlen;           // 0  - header length (0x40 / 64)
    uint32_t type;              // 4  - audio type (0x01: v2.x audio | 0x02: text bind | 0x03: v3.x extended audio)
    uint32_t valid;             // 8  - valid
    uint32_t offset;            // 12 - audio offset
    uint32_t time;              // 16 - audio time
    uint32_t duration;          // 20 - audio duration
    uint32_t unknown1;          // 24 - unused
    uint32_t unknown2;          // 28 - unused
    char     username[32];      // 32 - user name
} __attribute__ ((__packed__));

struct _v3_vrf_audio {          // 32
    uint32_t headlen;           // 0  - header length (0x20 / 32)
    uint32_t type;              // 4  - audio type (0x01: v2.x audio | 0x02: text bind | 0x03: v3.x extended audio)
    uint32_t unknown1;          // 8  - unused
    uint32_t index;             // 12 - segment index
    uint32_t fragcount;         // 16 - fragment count
    uint32_t unknown2;          // 20 - unused
    uint32_t unknown3;          // 24 - unused
    uint32_t offset;            // 28 - unused
} __attribute__ ((__packed__));

struct _v3_vrf_fragment {       // 24
    uint32_t headlen;           // 0  - header length (0x01: 0x10 / 16 | 0x02: variable length | 0x03: 0x18 / 24)
    uint32_t fraglen;           // 4  - fragment length
    struct {
        uint32_t pcmlen;        // 8  - pcm buffer length
        uint32_t unknown1;      // 12 - unused
        struct {
            uint16_t codec;       // 16 - codec
            uint16_t codecformat; // 18 - codec format
            uint32_t unknown2;    // 20 - unused
        } __attribute__ ((__packed__)) ext;
    } __attribute__ ((__packed__)) audio;
} __attribute__ ((__packed__));

struct _v3_vrf_rec {
    uint32_t        user_id;

    _v3_vrf_audio   audio;
    _v3_vrf_segment segment;

    void            *data;
    uint32_t        datalen;

    uint8_t         stopped;

    _v3_vrf_rec     *next;
};

struct _v3_vrf {
    int             file;
    char            *filename;
    uint32_t        filelen;

    pthread_mutex_t mutex;

    _v3_vrf_header  header;
    _v3_vrf_segment *table;
    uint32_t        tablesize;

    struct timeval  start;
    _v3_vrf_rec     queue;

    uint8_t         stopped;
};

void     _v3_vrf_lock(_v3_vrf *vrfh);
void     _v3_vrf_unlock(_v3_vrf *vrfh);
void     _v3_vrf_print_header(const _v3_vrf_header *header);
void     _v3_vrf_print_info(const _v3_vrf_header *header);
void     _v3_vrf_print_segment(uint32_t id, const _v3_vrf_segment *segment);
void     _v3_vrf_print_audio(const _v3_vrf_audio *audio);
void     _v3_vrf_print_fragment(uint32_t type, const _v3_vrf_fragment *fragment);
int      _v3_vrf_get_header(_v3_vrf *vrfh);
int      _v3_vrf_get_table(_v3_vrf *vrfh);
int      _v3_vrf_check_table(_v3_vrf *vrfh);
int      _v3_vrf_get_segment(_v3_vrf *vrfh, uint32_t id, _v3_vrf_segment **segment);
int      _v3_vrf_get_audio(_v3_vrf *vrfh, uint32_t offset, _v3_vrf_audio *audio);
int      _v3_vrf_get_fragment(_v3_vrf *vrfh, uint32_t type, uint32_t *offset, _v3_vrf_fragment *fragment, uint32_t *fraglen, void **fragdata);
int      _v3_vrf_put_header(_v3_vrf *vrfh);
uint32_t _v3_vrf_put_segment(uint32_t id, const _v3_vrf_segment *segment, void *offset);
uint32_t _v3_vrf_put_audio(const _v3_vrf_audio *audio, void *offset);
uint32_t _v3_vrf_put_fragment(uint32_t type, const _v3_vrf_fragment *fragment, void *offset);
void     _v3_vrf_put_record(uint32_t user_id, uint32_t index, uint32_t type, const char *username, _v3_vrf_rec *rec);
void     _v3_vrf_record_event(
                int type,
                uint16_t user_id,
                uint16_t codec,
                uint16_t codecformat,
                uint32_t pcmlen,
                uint32_t datalen,
                const void *data);
void     _v3_vrf_record_finish(_v3_vrf *vrfh, uint32_t segtable);
int      _v3_vrf_recover(_v3_vrf *vrfh);
/*}}}*/

_v3_vrf                 *v3_vrfh = NULL;

pthread_mutex_t         *vrfh_mutex = NULL;

/*
 * Functions in ventrilo3_algo.c
 */
int                     ventrilo_read_keys(ventrilo_key_ctx *client, ventrilo_key_ctx *server, uint8_t *data, int size
#ifdef VENTRILO_ALGO_PROTOCOL
        , int protocol
#endif
        );
void                    ventrilo_first_dec(uint8_t *data, int size);
void                    ventrilo_first_enc(uint8_t *data, int size);
void                    ventrilo_dec(ventrilo_key_ctx *ctx, uint8_t *data, int size);
void                    ventrilo_enc(ventrilo_key_ctx *ctx, uint8_t *data, int size);
int                     ventrilo3_handshake(uint32_t ip, uint16_t port, uint8_t *handshake, uint32_t *handshake_num, uint8_t *handshake_key);
void                    ventrilo3_algo_scramble(ventrilo_key_ctx *ctx, uint8_t *v3key);
int                     _v3_logout(void);

/*
 * Internal functions
 */
void                    _v3_debug(uint32_t level, const char *format, ...);
char *                  _v3_status(uint8_t percent, const char *format, ...);
void                    _v3_hexdump(int level, const char *data, int len);
void                    _v3_net_message_dump_raw(const char *data, int len);
void                    _v3_net_message_dump(_v3_net_message *msg);

int                     _v3_login_connect(struct in_addr *srvip, uint16_t srvport);
int                     _v3_server_auth(struct in_addr *srvip, uint16_t srvport);

int                     _v3_evpipe_write(int fd, v3_event *ev);

int                     _v3_send(_v3_net_message *);
_v3_net_message *       _v3_recv(int block);
_v3_net_message *       _v3_create_message(_v3_net_message *msg, uint16_t type, uint16_t len, char *data);

int                     _v3_server_key_exchange(void);
int                     _v3_send_enc_msg(char *data, int len);
int                     _v3_recv_enc_msg(char *data);
int                     _v3_process_message(_v3_net_message *msg);
void                    _v3_destroy_packet(_v3_net_message *msg);
int                     _v3_update_channel(v3_channel *channel);
void                    _v3_copy_channel(v3_channel *dest, v3_channel *src);
int                     _v3_update_user(v3_user *user);
void                    _v3_copy_user(v3_user *dest, v3_user *src);
v3_user *               _v3_get_user(uint16_t id);
int                     _v3_update_rank(v3_rank *rank);
void                    _v3_copy_rank(v3_rank *dest, v3_rank *src);
int                     _v3_update_account(v3_account *account);
int                     _v3_remove_account(uint16_t id);
void                    _v3_copy_account(v3_account *dest, v3_account *src);
void                    _v3_destroy_userlist(void);
void                    _v3_destroy_channellist(void);
void                    _v3_destroy_ranklist(void);
void                    _v3_destroy_accountlist(void);
void                    _v3_lock_userlist(void);
void                    _v3_unlock_userlist(void);
void                    _v3_lock_channellist(void);
void                    _v3_unlock_channellist(void);
void                    _v3_lock_ranklist(void);
void                    _v3_unlock_ranklist(void);
void                    _v3_lock_accountlist(void);
void                    _v3_unlock_accountlist(void);
void                    _v3_lock_luser(void);
void                    _v3_unlock_luser(void);
void                    _v3_lock_sendq(void);
void                    _v3_unlock_sendq(void);
void                    _v3_lock_server(void);
void                    _v3_unlock_server(void);
int                     v3_queue_event(v3_event *ev);
v3_event                *_v3_get_last_event(int *len);
v3_event                *_v3_create_event(uint16_t event);

#ifdef _WIN32
// TODO: May want to move these function definitions to a windows specific header file.
int                     mingw_inet_aton(const char *cp, struct in_addr *addr);
#endif

#if HAVE_SPEEX_DSP
void _v3_lock_audioq(void);
void _v3_unlock_audioq(void);
int  _v3_nonblock(int pipefd[2]);
#endif
void _v3_init_decoders(void);
void _v3_destroy_decoder(_v3_decoders *decoder);
void _v3_destroy_decoders(void);
uint8_t *_v3_audio_encode(
                /* pcm input */
                uint8_t *sample,
                uint32_t pcmlen,
                /* encoded output */
                const v3_codec *codec,
                uint16_t *datalen,
                /* optional args */
                uint8_t channels);
int _v3_audio_decode(
                /* encoded input */
                const v3_codec *codec,
                _v3_decoders *decoder,
                uint8_t *data,
                uint16_t datalen,
                /* pcm output */
                uint8_t *sample,
                uint32_t *pcmlen,
                /* optional args */
                uint8_t *channels);

#endif // _LIBVENTRILO3_H

