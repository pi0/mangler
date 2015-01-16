/*
 * vim: softtabstop=4 shiftwidth=4 cindent foldmethod=marker expandtab
 *
 * $LastChangedDate: 2011-06-27 23:50:10 +0430 (Mon, 27 Jun 2011) $
 * $Revision: 1139 $
 * $LastChangedBy: econnell $
 * $URL: http://svn.mangler.org/mangler/trunk/libventrilo3/libventrilo3_message.h $
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

#ifndef _LIBVENTRILO3_MESSAGE_H
#define _LIBVENTRILO3_MESSAGE_H

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ventrilo3.h"

#define V3_CLIENT_VERSION   "3.0.5"
#define V3_PROTO_VERSION    "3.0.0"

/*
 * This file contains structures for each message type received by the Ventrilo3
 * protocol.  Each type is broken up into subtypes where applicable.  For
 * example:
 *
 * _v3_net_message_0x5d_0x04 is message type 0x5d subtype 0x04
 *
 * Where further breakdown is necessary (for instance user and channel
 * information messages), structures are defined specifically for that data.
 * That data (so far) is identical for any subtype.  Therefore, a user
 * definition in message type 0x5d subtype 0x04 is identical to a user
 * definition in type 0x5d subtype 0x01.
 *
 * To parse a packet, a smaller structure (i.e. a "type") will be recast as a
 * larger structure depending on the subtype.
 *
 * In cases where a particular value is unknown, it is marked by a name of
 * (unknown_#).  The number is arbitrary and subject to change.  If an unknown
 * value is identified, it should be pulled out of the unknown pool and named
 * appropriately.
 *
 * For questions or comments, join irc://irc.freenode.net channel #mangler
 *
 * Better documentation on these messages can be found at our wiki:
 * http://www.mangler.org/trac/wiki/ProtocolDocumentation
 */

typedef struct _v3_net_message_0x00 {/*{{{*/
    uint32_t type;
    char     version[16];
    char     salt1[32];
    char     salt2[32];
} __attribute__ ((__packed__)) _v3_msg_0x00;
_v3_net_message *_v3_put_0x00(void);/*}}}*/
typedef struct _v3_net_message_0x06 {/*{{{*/
    uint32_t type;              // 0
    uint16_t unknown_1;         // 4
    uint16_t error_id;          // 6
    uint32_t subtype;           // 8
    uint8_t  unknown_2;         // 12 - variable length starts here
    uint8_t *encryption_key;
} __attribute__ ((__packed__)) _v3_msg_0x06;
int _v3_get_0x06(_v3_net_message *msg);/*}}}*/
typedef struct _v3_net_message_0x33 {/*{{{*/
    uint32_t type;              // 0
    uint8_t  unkonwn[48];       // 4
    uint16_t channel_id_count;  // 52
    uint16_t *channel_ids;      // 54 - array of uint16_t - variable length starts list
} __attribute__ ((__packed__)) _v3_msg_0x33;
int _v3_get_0x33(_v3_net_message *msg);
int _v3_destroy_0x33(_v3_net_message *msg);/*}}}*/
typedef struct _v3_net_message_0x36 {/*{{{*/
    uint32_t type;              // 0
    uint16_t subtype;           // 4
    uint16_t error_id;          // 6
    uint16_t unknown_1;         // 8
    uint16_t rank_count;        // 10
    uint16_t unknown_2;         // 12
    uint16_t unknown_3;         // 14
    _v3_msg_rank *rank_list;
} __attribute__ ((__packed__)) _v3_msg_0x36;
int _v3_get_0x36(_v3_net_message *msg);
_v3_net_message *_v3_put_0x36(uint16_t subtype, v3_rank *rank);
int _v3_destroy_0x36(_v3_net_message *msg);/*}}}*/
typedef struct _v3_net_message_0x37 {/*{{{*/
    uint32_t type;              // 0
    uint16_t user_id;           // 4
    uint16_t sequence;          // 6
    uint16_t ping;              // 8
    uint16_t inactivity;        // 10
} __attribute__ ((__packed__)) _v3_msg_0x37;
int _v3_get_0x37(_v3_net_message *msg);
_v3_net_message *_v3_put_0x37(int sequence);/*}}}*/
typedef struct _v3_net_message_0x3a {/*{{{*/
    uint32_t type;              // 0
    uint32_t user_id;           // 4
    uint16_t msglen;            // 8
    char *   msg;               // 10 - variable length starts here
} __attribute__ ((__packed__)) _v3_msg_0x3a;/*}}}*/
int _v3_get_0x3a(_v3_net_message *msg);
_v3_net_message *_v3_put_0x3a(char *message);
typedef struct _v3_net_message_0x3b {/*{{{*/
    uint32_t type;              // 0
    uint16_t id;                // 4
    uint16_t channel_id;        // 6
    uint32_t error_id;          // 8
} __attribute__ ((__packed__)) _v3_msg_0x3b;
int _v3_get_0x3b(_v3_net_message *msg);
_v3_net_message *_v3_put_0x3b(uint16_t id, uint16_t channel_id);/*}}}*/
typedef struct _v3_net_message_0x3c {/*{{{*/
    uint32_t type;              // 0
    uint8_t  unknown1[4];       // 4
    uint16_t codec;             // 8
    uint16_t codec_format;      // 10
    uint8_t  unknown2[12];      // 12
} __attribute__ ((__packed__)) _v3_msg_0x3c;
int _v3_get_0x3c(_v3_net_message *msg);/*}}}*/
typedef struct _v3_net_message_0x3f {/*{{{*/
    uint32_t type;              // 0
    uint32_t user_id;           // 4
    uint16_t msglen;            // 8
    char *   msg;               // 10 - variable length starts here
} __attribute__ ((__packed__)) _v3_msg_0x3f;/*}}}*/
int _v3_get_0x3f(_v3_net_message *msg);
_v3_net_message *_v3_put_0x3f(char *message);
typedef struct _v3_net_message_0x42 {/*{{{*/
    uint32_t type;              // 0
    uint16_t user_id;           // 4
    uint16_t subtype;           // 6
    uint32_t unknown;           // 8
    uint16_t msglen;            // 12 - variable length starts here
    char *   msg;               // 14
} __attribute__ ((__packed__)) _v3_msg_0x42;
int _v3_get_0x42(_v3_net_message *msg);
_v3_net_message *_v3_put_0x42(uint16_t subtype, uint16_t user_id, char *message);/*}}}*/
typedef struct _v3_net_message_0x46 {/*{{{*/
    uint32_t type;              // 0
    uint16_t user_id;           // 4
    uint16_t setting;           // 6
    uint32_t value;             // 8
} __attribute__ ((__packed__)) _v3_msg_0x46;
int _v3_get_0x46(_v3_net_message *msg);
_v3_net_message *_v3_put_0x46(uint16_t user_id, uint16_t setting, uint16_t value);/*}}}*/
typedef struct _v3_net_message_0x48 {/*{{{*/
    uint32_t type;              // 0
    uint32_t subtype;           // 4
    uint32_t unknown_1;         // 8
    uint32_t server_ip;         // 12
    uint16_t portmask;          // 16
    uint16_t show_login_name;   // 18
    uint16_t unknown_2;         // 20
    uint16_t auth_server_index; // 22
    char     handshake[16];     // 24
    char     client_version[16];// 40
    uint8_t  unknown_3[48];     // 56
    char     proto_version[16]; // 104
    char     password_hash[32]; // 120
    char     username[32];      // 152
    char     phonetic[32];      // 184
    char     os[64];            // 216
} __attribute__ ((__packed__)) _v3_msg_0x48;
int _v3_get_0x48(_v3_net_message *msg);
_v3_net_message *_v3_put_0x48(void);/*}}}*/
typedef struct _v3_net_message_0x49 {/*{{{*/
    uint32_t type;              // 0
    uint16_t user_id;           // 4
    uint16_t subtype;           // 6
    uint8_t  hash_password[32]; // 8
    v3_channel *channel;        // 40 - variable length starts here
} __attribute__ ((__packed__)) _v3_msg_0x49;
int _v3_get_0x49(_v3_net_message *msg);
_v3_net_message *_v3_put_0x49(uint16_t subtype, uint16_t user_id, char *channel_password, _v3_msg_channel *channel);/*}}}*/
typedef struct _v3_net_message_0x4a {/*{{{*/
    uint32_t type;              // 0
    uint16_t subtype;           // 4
    uint16_t error_id;          // 6
    uint16_t unknown_1;         // 8
    uint16_t count;             // 10
    uint16_t start_id;          // 12
    uint16_t end_id;            // 14
    uint32_t unknown_2;         // 16
} __attribute__ ((__packed__)) _v3_msg_0x4a;
typedef struct _v3_net_message_0x4a_account {
    _v3_msg_0x4a header;        // 0
    int acct_list_count;
    v3_account **acct_list;
} __attribute__ ((__packed__)) _v3_msg_0x4a_account;
typedef struct _v3_net_message_0x4a_perms {
    _v3_msg_0x4a header;        // 0
    v3_permissions perms;       // 20
} __attribute__ ((__packed__)) _v3_msg_0x4a_perms;
int _v3_get_0x4a(_v3_net_message *msg);
_v3_net_message *_v3_put_0x4a(uint8_t subtype, v3_account *account, v3_account *account2);
int _v3_destroy_0x4a(_v3_net_message *msg);/*}}}*/
typedef struct _v3_net_message_0x4b {/*{{{*/
    uint32_t type;              // 0
    uint32_t timestamp;         // 4
    uint32_t empty;             // 8
} __attribute__ ((__packed__)) _v3_msg_0x4b;
_v3_net_message *_v3_put_0x4b(void);/*}}}*/
typedef struct _v3_net_message_0x4c {/*{{{*/
    uint32_t type;              // 0
    uint16_t subtype;           // 4
    uint16_t property;          // 6
    uint16_t ack;               // 8
    uint16_t transaction_id;    // 10
    char *   value;             // 12
} __attribute__ ((__packed__)) _v3_msg_0x4c;
int _v3_get_0x4c(_v3_net_message *msg);
_v3_net_message *_v3_put_0x4c(uint16_t subtype, uint16_t property, uint16_t transaction_id, char *value);/*}}}*/
typedef struct _v3_net_message_0x50 {/*{{{*/
    uint32_t type;              // 0
    uint32_t timestamp;         // 4
    uint16_t guest_motd_flag;   // 8
    uint16_t message_num;       // 10
    uint16_t message_id;        // 12
    uint16_t message_size;      // 14
    uint8_t  message[256];      // 16
} __attribute__ ((__packed__)) _v3_msg_0x50;
int _v3_get_0x50(_v3_net_message *msg);/*}}}*/
typedef struct _v3_net_message_0x52 {/*{{{*/
    uint32_t type;              // 0
    uint16_t subtype;           // 4
    uint16_t user_id;           // 6
    uint16_t codec;             // 8
    uint16_t codec_format;      // 10
    uint16_t send_type;         // 12
    uint16_t unknown_1;         // 14
    uint32_t data_length;       // 16
    uint32_t pcm_length;        // 20
} __attribute__ ((__packed__)) _v3_msg_0x52;/*}}}*/
typedef struct _v3_net_message_0x52_0x00 {/*{{{*/
    _v3_msg_0x52 header;        // 0
    uint16_t unknown_4;         // 24
    uint16_t unknown_5;         // 26
    uint16_t unknown_6;         // 28
    uint16_t unknown_7;         // 30
} __attribute__ ((__packed__)) _v3_msg_0x52_0x00;/*}}}*/
typedef struct _v3_net_message_0x52_0x01_in {/*{{{*/
    _v3_msg_0x52 header;        // 0
    uint16_t unknown_4;         // 24
    uint16_t unknown_5;         // 26
    void *   data;              // 28 - variable length starts here
} __attribute__ ((__packed__)) _v3_msg_0x52_0x01_in;/*}}}*/
typedef struct _v3_net_message_0x52_0x01_out {/*{{{*/
    _v3_msg_0x52 header;        // 0
    uint16_t unknown_4;         // 24
    uint16_t unknown_5;         // 26
    uint16_t unknown_6;         // 28
    uint16_t unknown_7;         // 30
} __attribute__ ((__packed__)) _v3_msg_0x52_0x01_out;/*}}}*/
typedef struct _v3_net_message_0x52_0x02 {/*{{{*/
    _v3_msg_0x52 header;        // 0
    uint16_t unknown_4;         // 24
    uint16_t unknown_5;         // 26
} __attribute__ ((__packed__)) _v3_msg_0x52_0x02;/*}}}*/
typedef struct _v3_net_message_0x52_0x03 {/*{{{*/
    _v3_msg_0x52 header;        // 0
} __attribute__ ((__packed__)) _v3_msg_0x52_0x03;/*}}}*/
int _v3_get_0x52(_v3_net_message *msg);
_v3_net_message *_v3_put_0x52(uint8_t subtype, uint16_t codec, uint16_t codec_format, uint32_t pcmlength, uint32_t length, void *data);
int _v3_destroy_0x52(_v3_net_message *msg);
typedef struct _v3_net_message_0x53 {/*{{{*/
    uint32_t type;              // 0
    uint16_t user_id;           // 2
    uint16_t channel_id;        // 4
} __attribute__ ((__packed__)) _v3_msg_0x53;
int _v3_get_0x53(_v3_net_message *msg);/*}}}*/
typedef struct _v3_net_message_0x57 {/*{{{*/
    uint32_t type;              // 0
    uint16_t unknown_1;         // 4
    uint16_t is_licensed;       // 6
    uint16_t port;              // 8
    uint16_t max_clients;       // 10
    uint16_t connected_clients; // 12
    uint16_t unknown_2[3];      // 14
    uint8_t  _pad[8];           // 20
    char     name[32];          // 28
    char     version[16];       // 60
    char     unknown_3[32];     // 76
} __attribute__ ((__packed__)) _v3_msg_0x57;
int _v3_get_0x57(_v3_net_message *msg);/*}}}*/
typedef struct _v3_net_message_0x58 {/*{{{*/
    uint32_t type;              // 0
    uint16_t subtype;           // 4
    uint16_t error_id;          // 6
    uint16_t unknown1;          // 8
    uint16_t real_user_id;      // 10
    uint16_t phantom_user_id;   // 12
    uint16_t channel_id;        // 14
    uint16_t log_error;         // 16
    uint16_t unknown2;          // 18
} __attribute__ ((__packed__)) _v3_msg_0x58;
int _v3_get_0x58(_v3_net_message *msg);
_v3_net_message *_v3_put_0x58(uint16_t subtype, uint16_t channel, uint16_t phantom_user_id);/*}}}*/
typedef struct _v3_net_message_0x59 {/*{{{*/
    uint32_t type;              // 0
    uint16_t error;             // 4
    uint16_t minutes_banned;    // 6
    uint16_t log_error;         // 8
    uint16_t close_connection;  // 10
    uint16_t unknown_[4];       // 12
    char *   message;           // 20
} __attribute__ ((__packed__)) _v3_msg_0x59;
int _v3_get_0x59(_v3_net_message *msg);/*}}}*/
typedef struct _v3_net_message_0x5a {/*{{{*/
    uint32_t type;              // 0
    uint16_t subtype;           // 4
    uint16_t error;             // 6
    uint16_t user1;             // 8
    uint16_t user2;             // 8
    uint16_t msglen;            // 12 - variable length starts here
    char *   msg;               // 14
} __attribute__ ((__packed__)) _v3_msg_0x5a;
int _v3_get_0x5a(_v3_net_message *msg);
_v3_net_message *_v3_put_0x5a(uint16_t subtype, uint16_t user1, uint16_t user2, char *message);/*}}}*/
typedef struct _v3_net_message_0x5c {/*{{{*/
    uint32_t type;              // 0
    uint16_t subtype;           // 4
    uint16_t sum_1;             // 6
    uint32_t sum_2;             // 8
} __attribute__ ((__packed__)) _v3_msg_0x5c;
int _v3_get_0x5c(_v3_net_message *msg);
_v3_net_message *_v3_put_0x5c(uint8_t subtype);
uint32_t _v3_msg5c_scramble(uint8_t* in);
uint32_t _v3_msg5c_gensum(uint32_t seed, uint32_t iterations);/*}}}*/
typedef struct _v3_net_message_0x5d {/*{{{*/
    uint32_t type;              // 0
    uint16_t subtype;           // 4
    uint16_t user_count;        // 6
    _v3_msg_user *lobby;        // 8 - variable length starts here
    _v3_msg_user *user_list;
} __attribute__ ((__packed__)) _v3_msg_0x5d;
_v3_net_message *_v3_put_0x5d(uint16_t subtype, uint16_t count, v3_user *user);
int _v3_get_0x5d(_v3_net_message *msg);
int _v3_destroy_0x5d(_v3_net_message *msg);/*}}}*/
typedef struct _v3_net_message_0x60 {/*{{{*/
    uint32_t type;              // 0
    uint32_t channel_count;     // 4
    _v3_msg_channel *channel_list; // 8 - variable length stats here
} __attribute__ ((__packed__)) _v3_msg_0x60;
int _v3_get_0x60(_v3_net_message *msg);
int _v3_destroy_0x60(_v3_net_message *msg);/*}}}*/
typedef struct _v3_net_message_0x61 {/*{{{*/
    uint32_t type;              // 0
    uint32_t subtype;           // 4
    uint32_t bitmask_id;        // 8
    uint32_t ip_address;        // 12
    uint16_t ban_count;         // 16
    uint16_t ban_id;            // 18
    char     banned_user[32];   // 20
    char     banned_by[32];     // 52
    char     ban_reason[128];   // 84
} __attribute__ ((__packed__)) _v3_msg_0x61;
int _v3_get_0x61(_v3_net_message *msg);
_v3_net_message *_v3_put_0x61(uint32_t subtype, uint32_t bitmask_id, uint32_t ip_address, char *banned_user, char *ban_reason);/*}}}*/
typedef struct _v3_net_message_0x62 {/*{{{*/
    uint32_t type;              // 0
    uint16_t user_id_to;        // 4
    uint16_t user_id_from;      // 6
    uint32_t error_id;          // 8
} __attribute__ ((__packed__)) _v3_msg_0x62;
int _v3_get_0x62(_v3_net_message *msg);
_v3_net_message *_v3_put_0x62(uint16_t user_id);/*}}}*/
typedef struct _v3_net_message_0x63 {/*{{{*/
    uint32_t type;              // 0
    uint16_t subtype;           // 4
    uint16_t user_id;           // 6
    uint32_t unused;            // 8
    union {                     // 12
        uint8_t password_hash[0x20];
        char    reason[0x80];
    } __attribute__ ((__packed__)) t;
} __attribute__ ((__packed__)) _v3_msg_0x63;
_v3_net_message *_v3_put_0x63(uint16_t subtype, uint16_t user_id, char *string);/*}}}*/

char *      _v3_get_msg_string(void *offset, uint16_t *len);
int         _v3_put_msg_string(void *buffer, char *string);
uint16_t *  _v3_get_msg_uint16_array(void *offset, uint16_t *len);
int         _v3_put_msg_uint16_array(void *buffer, uint16_t len, uint16_t *array);
int         _v3_get_msg_channel(void *offset, _v3_msg_channel *channel);
int         _v3_put_msg_channel(void *buffer, _v3_msg_channel *channel);
int         _v3_get_msg_user(void *offset, _v3_msg_user *user);
int         _v3_put_msg_user(void *buf, _v3_msg_user *user);
int         _v3_get_msg_account(void *offset, _v3_msg_account *account);
int         _v3_put_msg_account(void *buf, _v3_msg_account *account);
int         _v3_parse_filter(v3_sp_filter *f, char *value);
int         _v3_strip_c0_set(char *s);

#endif // _LIBVENTRILO3_MESSAGE_H

