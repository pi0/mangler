/*
 * vim: softtabstop=4 shiftwidth=4 cindent foldmethod=marker expandtab
 *
 * $LastChangedDate: 2011-06-27 23:50:10 +0430 (Mon, 27 Jun 2011) $
 * $Revision: 1139 $
 * $LastChangedBy: econnell $
 * $URL: http://svn.mangler.org/mangler/trunk/libventrilo3/libventrilo3_message.c $
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

#include "libventrilo3_message.h"
#include "ventrilo3.h"

#define true    1
#define false   0

extern _v3_server v3_server;
extern _v3_luser v3_luser;
extern uint32_t _v3_hash_table[];

/*
 *  The following are utility functions used to convert data from a raw
 *  received packet into a reasonable value.  The string functions here
 *  will allocate memory when called, so any returned string must be
 *  free()'d later.
 */

// Functions to retreive a specific data type from a packet /*{{{*/
/*
 * Get a string from the packet whose length is identified by the first 2
 * bytes.  For example, a packet that contains:
 *
 *   04 62 6c 61 68             .blah
 *
 * Will allocate 5 bytes of memory (4 for the string + null) and copy the
 * next 4 bytes into that memory.
 *
 * Parameters:
 *     uint8_t *data            This is a pointer to the location of the
 *                           length (as a 2 byte int) of the packet.  The
 *                           actual string is assumed to be immediately
 *                           following.
 *     int *len              A pointer to an integer.  The number of bytes read
 *                           from the packet will be placed in this variable
 *
 * Returns:                  A pointer to a copy of the string.  The string
 *                           will be null terminated.
 */
char *
_v3_get_msg_string(void *offset, uint16_t *len) {/*{{{*/
    char *s;

    _v3_func_enter("_v3_get_msg_string");
    memcpy(len, offset, 2);
    *len = ntohs(*len);
    _v3_debug(V3_DEBUG_PACKET_PARSE, "getting %d (0x%04X) byte string", *len, *len);
    s = malloc(*len+1);
    memset(s, 0, *len+1);
    memcpy(s, offset+2, *len);
    s[*len] = '\0';
    *len+=2;

    _v3_func_leave("_v3_get_msg_string");
    return s;
}/*}}}*/

uint16_t *
_v3_get_msg_uint16_array(void *offset, uint16_t *len) {/*{{{*/
    uint16_t *s;
    uint16_t ctr;

    _v3_func_enter("_v3_get_msg_string");
    memcpy(len, offset, 2);
    *len = ntohs(*len);
    _v3_debug(V3_DEBUG_PACKET_PARSE, "getting %d (0x%04X) 16 bit elements", *len, *len);
    s = malloc(*len*2);
    memset(s, 0, *len*2);
    memcpy(s, offset+2, *len*2);
    for (ctr = 0; ctr < *len; ctr++) {
        s[ctr] = ntohs(s[ctr]);
    }
    *len = (*len*2)+2;

    _v3_func_leave("_v3_get_msg_string");
    return s;
}/*}}}*/

int
_v3_put_msg_uint16_array(void *buffer, uint16_t len, uint16_t *array) {/*{{{*/
    uint16_t count;
    void *p;

    _v3_func_enter("_v3_put_msg_uint16_array");
    *((uint16_t *)buffer) = htons(len);
    for (count = 0; count < len; count++) {
        p = buffer + 2 + count * 2;
        *((uint16_t *)p) = htons(array[count]);
    }

    _v3_func_leave("_v3_put_msg_uint16_array");
    return len*2 + 2;
}/*}}}*/

int
_v3_put_msg_string(void *buffer, char *string) {/*{{{*/
    uint16_t len;

    _v3_func_enter("_v3_put_msg_string");
    len = (string) ? strlen(string) : 0;
    *((uint16_t *)buffer) = htons(len);
    if (len) {
        memcpy(buffer+2, string, len);
    }

    _v3_func_leave("_v3_put_msg_string");
    return len + 2;
}/*}}}*/

int
_v3_get_msg_channel(void *offset, _v3_msg_channel *channel) {/*{{{*/
    uint16_t len;
    void *start_offset = offset;

    _v3_func_enter("_v3_get_msg_channel");
    // get the channel information
    memcpy(channel, offset, 48);
    offset+=48;

    channel->name = (char *)_v3_get_msg_string(offset, &len);
    offset+=len;
    channel->phonetic = (char *)_v3_get_msg_string(offset, &len);
    offset+=len;
    channel->comment = (char *)_v3_get_msg_string(offset, &len);
    offset+=len;

    _v3_func_leave("_v3_get_msg_channel");
    return (offset - start_offset);
}/*}}}*/

int
_v3_get_msg_user(void *offset, _v3_msg_user *user) {/*{{{*/
    uint16_t len;
    void *start_offset = offset;

    _v3_func_enter("_v3_get_msg_user");
    // get the user information
    memcpy(user, offset, 8);
    offset+=8;

    user->name = (char *)_v3_get_msg_string(offset, &len);
    offset+=len;
    user->phonetic = (char *)_v3_get_msg_string(offset, &len);
    offset+=len;
    user->comment = (char *)_v3_get_msg_string(offset, &len);
    offset+=len;
    user->integration_text = (char *)_v3_get_msg_string(offset, &len);
    offset+=len;
    user->url = (char *)_v3_get_msg_string(offset, &len);
    offset+=len;

    _v3_func_leave("_v3_get_msg_user");
    return (offset - start_offset);
}/*}}}*/

int
_v3_get_msg_rank(void *offset, _v3_msg_rank *rank) {/*{{{*/
    uint16_t len;
    void *start_offset = offset;

    _v3_func_enter("_v3_get_msg_rank");
    // get the rank information
    memcpy(rank, offset, 4);
    offset+=4;

    rank->name = (char *)_v3_get_msg_string(offset, &len);
    offset+=len;
    rank->description = (char *)_v3_get_msg_string(offset, &len);
    offset+=len;

    _v3_func_leave("_v3_get_msg_rank");
    return (offset - start_offset);
}/*}}}*/

int
_v3_get_msg_account(void *offset, _v3_msg_account *account) {/*{{{*/
    void *start_offset = offset;
    uint16_t len;
    int j;

    _v3_func_enter("_v3_get_msg_account");
    _v3_debug(V3_DEBUG_PACKET_PARSE, "start user acct parsing");

    memcpy(&(account->perms), offset, sizeof(account->perms));
    _v3_print_permissions(&account->perms);
    offset += sizeof(account->perms);

    account->username = _v3_get_msg_string(offset, &len);
    _v3_debug(V3_DEBUG_PACKET_PARSE, "name: %s", account->username);
    offset += len;
    account->owner = _v3_get_msg_string(offset, &len);
    _v3_debug(V3_DEBUG_PACKET_PARSE, "owner: %s", account->owner);
    offset += len;
    account->notes = _v3_get_msg_string(offset, &len);
    _v3_debug(V3_DEBUG_PACKET_PARSE, "notes: %s", account->notes);
    offset += len;
    account->lock_reason = _v3_get_msg_string(offset, &len);
    _v3_debug(V3_DEBUG_PACKET_PARSE, "lock reason: %s", account->lock_reason);
    offset += len;

    account->chan_admin = _v3_get_msg_uint16_array(offset, &len);
    for (j=0;j<(len-2)/2;j++)
        _v3_debug(V3_DEBUG_PACKET_PARSE, "channel admin: %u", account->chan_admin[j]);
    account->chan_admin_count = (len - 2) / 2;
    offset += len;
    account->chan_auth = _v3_get_msg_uint16_array(offset, &len);
    for (j=0;j<(len-2)/2;j++)
        _v3_debug(V3_DEBUG_PACKET_PARSE, "channel auth: %u", account->chan_auth[j]);
    account->chan_auth_count = (len - 2) / 2;
    offset += len;

    _v3_debug(V3_DEBUG_PACKET_PARSE, "end user acct parsing");

    _v3_func_leave("_v3_get_msg_account");
    return (offset - start_offset);
}/*}}}*/

int
_v3_put_msg_account(void *buf, v3_account *account) {/*{{{*/
    void *start_buffer = buf;

    _v3_func_enter("_v3_put_msg_account");
    // put the account perms
    _v3_debug(V3_DEBUG_PACKET_PARSE, "putting account id: %d", account->perms.account_id);
    memcpy(buf, &account->perms, sizeof(account->perms));
    buf += sizeof(account->perms);

    // put the account strings
    buf += _v3_put_msg_string(buf, account->username);
    buf += _v3_put_msg_string(buf, account->owner);
    buf += _v3_put_msg_string(buf, account->notes);
    buf += _v3_put_msg_string(buf, account->lock_reason);

    // put the account channels
    buf += _v3_put_msg_uint16_array(buf, account->chan_admin_count, account->chan_admin);
    buf += _v3_put_msg_uint16_array(buf, account->chan_auth_count, account->chan_auth);

    _v3_func_leave("_v3_put_msg_account");
    return (buf - start_buffer);
}/*}}}*/

int
_v3_put_msg_user(void *buffer, v3_user *user) {/*{{{*/
    void *start_buffer = buffer;

    _v3_func_enter("_v3_put_msg_user");
    // put the user information
    _v3_debug(V3_DEBUG_PACKET_PARSE, "putting user id: %d", user->id);
    memcpy(buffer, user, 8);
    buffer+=8;

    // put the user strings
    buffer += _v3_put_msg_string(buffer, user->name);
    buffer += _v3_put_msg_string(buffer, user->phonetic);
    buffer += _v3_put_msg_string(buffer, user->comment);
    buffer += _v3_put_msg_string(buffer, user->integration_text);
    buffer += _v3_put_msg_string(buffer, user->url);

    _v3_func_leave("_v3_put_msg_user");
    return (buffer - start_buffer);
}/*}}}*/

int
_v3_put_msg_channel(void *buffer, _v3_msg_channel *channel) {/*{{{*/
    void *start_buffer = buffer;

    _v3_func_enter("_v3_put_msg_channel");
    // put the channel information
    _v3_debug(V3_DEBUG_PACKET_PARSE, "putting channel id: %d", channel->id);
    memcpy(buffer, channel, 48);
    buffer+=48;

    // put the channel strings
    buffer += _v3_put_msg_string(buffer, channel->name);
    buffer += _v3_put_msg_string(buffer, channel->phonetic);
    buffer += _v3_put_msg_string(buffer, channel->comment);

    _v3_func_leave("_v3_put_msg_channel");
    return (buffer - start_buffer);
}/*}}}*/

int
_v3_put_msg_rank(void *buffer, _v3_msg_rank *rank) {/*{{{*/
    void *start_buffer = buffer;

    _v3_func_enter("_v3_put_msg_rank");

    *((uint16_t *)buffer) = rank->id; buffer += 2;
    *((uint16_t *)buffer) = rank->level; buffer += 2;
    buffer += _v3_put_msg_string(buffer, rank->name);
    buffer += _v3_put_msg_string(buffer, rank->description);

    _v3_func_leave("_v3_put_msg_rank");
    return (buffer - start_buffer);
}/*}}}*/

int
_v3_parse_filter(v3_sp_filter *f, char *value) {/*{{{*/
    char *a, *i, *t, *tmp;

    _v3_func_enter("_v3_parse_filter");
    a = value;
    i = strchr(a, ',') + 1;
    // make sure strchr didn't return null (in which case it would be 1 since
    // we added 1)
    if (i == (void *)1) {
        _v3_func_leave("_v3_parse_filter");
        return false;
    }
    tmp = i - 1;
    *tmp = 0;
    t = strchr(i, ',') + 1;
    if (t == (void *)1) {
        _v3_func_leave("_v3_parse_filter");
        return false;
    }
    tmp = t - 1;
    *tmp = 0;
    f->action = atoi(a);
    f->interval = atoi(i);
    f->times = atoi(t);
    _v3_debug(V3_DEBUG_INFO, "parsed filter: %d, %d, %d", f->action, f->interval, f->times);

    _v3_func_leave("_v3_parse_filter");
    return true;
}/*}}}*/

int
_v3_strip_c0_set(char *s) {/*{{{*/
    void *start = s;
    uint8_t *str = (uint8_t *)s;

    _v3_func_enter("_v3_strip_c0_set");

    while (*str) {
        *str = (*str < 0x20) ? 0x20 : *str;
        str++;
    }

    _v3_func_leave("_v3_strip_c0_set");
    return ((void *)str - start);
}/*}}}*/
/*}}}*/

/*
 * These functions parse or create the various message types.  For all
 * functions, the "data" member is an array of bytes either received from or
 * suitable to send to the server.  The "contents" member is the a properly
 * formatted structure of data.  For some packet types, both data and contents
 * will point to the same memory location (i.e. for static length messages).
 *
 * _v3_get_* functions accept a net message structure as a parameter and fill
 * in the contents member (allocating memory if needed).
 *
 * _v3_put_* functions accept the data needed to fill in the packet and return
 * a newly allocated net message structure with the data and contents members
 * filled in.
 *
 * _v3_destroy_* functions will free any memory allocated for a specific packet
 * type.  The message structure itself is *NOT* freed
 */

// Message 0x00 (0)  | HANDSHAKE /*{{{*/
_v3_net_message *
_v3_put_0x00(void) {/*{{{*/
    _v3_net_message *m;
    _v3_msg_0x00 *mc;

    _v3_func_enter("_v3_put_0x00");
    // Build our message
    m = malloc(sizeof(_v3_net_message));
    memset(m, 0, sizeof(_v3_net_message));
    m->type = 0x00;
    m->len = sizeof(_v3_msg_0x00);

    // Build our message contents
    mc = malloc(sizeof(_v3_msg_0x00));
    memset(mc, 0, sizeof(_v3_msg_0x00));

    mc->type = 0x00;
    strncpy(mc->version, "3.0.0", 16);
    memset(mc->salt1, 0, 32);
    memset(mc->salt2, 0, 32);
    int ctr;
    for (ctr = 0; ctr < 31; ctr++) {
        mc->salt1[ctr] = rand() % 93 + 33;
        mc->salt2[ctr] = rand() % 93 + 33;
    }
    m->contents = mc;
    m->data = (char *)mc;

    _v3_func_leave("_v3_put_0x00");
    return m;
}/*}}}*/
/*}}}*/
// Message 0x06 (6)  | AUTHENTICATION/LOGIN ERROR /*{{{*/
int
_v3_get_0x06(_v3_net_message *msg) {/*{{{*/
    _v3_msg_0x06 *m;

    _v3_func_enter("_v3_get_0x06");
    m = malloc(sizeof(_v3_msg_0x06));
    memcpy(m, msg->data, 12);
    if (m->subtype & 4) {
        m->encryption_key = malloc(msg->len - 12);
        memcpy(m->encryption_key, msg->data + 12, msg->len - 12);
    } else {
        m->unknown_2 = msg->data[12];
    }
    msg->contents = m;

    _v3_func_leave("_v3_get_0x06");
    return true;
}/*}}}*/
/*}}}*/
// Message 0x33 (51) | CHANNEL ADMIN INFO (?)/*{{{*/
int
_v3_get_0x33(_v3_net_message *msg) {/*{{{*/
    _v3_msg_0x33 *m;
    uint16_t len, ctr;
    void *offset;

    m = malloc(sizeof(_v3_msg_0x33));
    _v3_func_enter("_v3_get_0x33");
    memcpy(m, msg->data, 52);
    offset = (void *)msg->data + 52;
    m->channel_ids = (uint16_t *)_v3_get_msg_uint16_array(offset, &len);
    offset+=len;
    // len is the byte length in the packet.  the number of elements in the
    // array (the channel count) is (len - 2) / 2
    m->channel_id_count = (len - 2) / 2;
    _v3_debug(V3_DEBUG_PACKET_PARSE, "Channel Admin for %d channels", m->channel_id_count);
    for (ctr = 0; ctr < m->channel_id_count; ctr++) {
        _v3_debug(V3_DEBUG_PACKET_PARSE, " * channel %d", m->channel_ids[ctr]);
    }
    msg->contents = m;

    _v3_func_leave("_v3_get_0x33");
    return true;
}/*}}}*/
int
_v3_destroy_0x33(_v3_net_message *msg) {/*{{{*/
    _v3_msg_0x33 *m;

    m = msg->contents;
    _v3_func_enter("_v3_destroy_0x33");
    _v3_debug(V3_DEBUG_PACKET_PARSE, "freeing resources channel admin list packet");
    free(m->channel_ids);

    _v3_func_leave("_v3_destroy_0x33");
    return true;
}/*}}}*/
/*}}}*/
// Message 0x36 (54) | RANK LIST MODIFICATION /*{{{*/
int
_v3_get_0x36(_v3_net_message *msg) {/*{{{*/
    _v3_msg_0x36 *m;
    int ctr;
    void *offset;

    _v3_func_enter("_v3_get_0x36");
    m = malloc(sizeof(_v3_msg_0x36));
    memcpy(m, msg->data, 16);
    _v3_debug(V3_DEBUG_PACKET_PARSE, "packet contains %d ranks.  message subtype %02X", m->rank_count, m->subtype);
    _v3_debug(V3_DEBUG_PACKET_PARSE, "allocating %d bytes for ranklist packet", sizeof(_v3_msg_0x36));
    m = realloc(m, sizeof(_v3_msg_0x36));
    if (m->rank_count) {
        _v3_debug(V3_DEBUG_PACKET_PARSE, "allocating %d bytes (%d ranks * %d bytes)", m->rank_count*sizeof(_v3_msg_rank), m->rank_count, sizeof(_v3_msg_rank));
        m->rank_list = calloc(m->rank_count, sizeof(_v3_msg_rank));
        for (ctr = 0, offset = msg->data + 16; ctr < m->rank_count; ctr++) {
            offset += _v3_get_msg_rank(offset, &m->rank_list[ctr]);
            _v3_debug(V3_DEBUG_PACKET_PARSE, "got rank: id: %d | name: %s | description: %s",
                m->rank_list[ctr].id,
                m->rank_list[ctr].name,
                m->rank_list[ctr].description
                );
        }
    }
    msg->contents = m;

    _v3_func_leave("_v3_get_0x36");
    return true;
}/*}}}*/
_v3_net_message *
_v3_put_0x36(uint16_t subtype, v3_rank *rank) {/*{{{*/
    _v3_net_message *m;
    _v3_msg_0x36 *mc;

    _v3_func_enter("_v3_put_0x36");
    // Build our message
    m = malloc(sizeof(_v3_net_message));
    memset(m, 0, sizeof(_v3_net_message));
    m->type = 0x36;
    switch (subtype) {
        case V3_OPEN_RANK:
        case V3_CLOSE_RANK:
            m->len = 16;
            mc = malloc(m->len);
            break;
        case V3_ADD_RANK:
        case V3_MODIFY_RANK:
            m->len = 24 + (rank->name ? strlen(rank->name) : 0) + (rank->description ? strlen(rank->description) : 0);
            mc = malloc(m->len);
            break;
        case V3_REMOVE_RANK:
            m->len = 20;
            mc = malloc(m->len + 4);
            break;
    }
    memset(mc, 0, m->len);
    mc->type = 0x36;
    mc->subtype = subtype;
    mc->rank_count = 1;
    if (rank) {
        _v3_put_msg_rank(&mc->rank_list, rank);
    }
    m->data = (char *)mc;
    m->contents = mc;

    _v3_func_leave("_v3_put_0x36");
    return m;
}/*}}}*/
int
_v3_destroy_0x36(_v3_net_message *msg) {/*{{{*/
    _v3_msg_0x36 *m;
    int ctr;

    m = msg->contents;
    _v3_func_enter("_v3_destroy_0x36");
    if (m->rank_count) {
        for (ctr = 0; ctr < m->rank_count; ctr++) {
            _v3_debug(V3_DEBUG_PACKET_PARSE, "freeing resources for rank %d: %s", m->rank_list[ctr].id, m->rank_list[ctr].name);
            free(m->rank_list[ctr].name);
            free(m->rank_list[ctr].description);
        }
        free(m->rank_list);
    }

    _v3_func_leave("_v3_destroy_0x36");
    return true;
}/*}}}*/
/*}}}*/
// Message 0x37 (55) | PING /*{{{*/
int
_v3_get_0x37(_v3_net_message *msg) {/*{{{*/
    _v3_msg_0x37 *m;

    _v3_func_enter("_v3_get_0x37");
    if (msg->len != sizeof(_v3_msg_0x37)) {
        _v3_debug(V3_DEBUG_PACKET_PARSE, "expected %d bytes, but message is %d bytes", sizeof(_v3_msg_0x37), msg->len);
        _v3_func_leave("_v3_get_0x37");
        return false;
    }
    m = msg->contents = msg->data;
    _v3_debug(V3_DEBUG_PACKET_PARSE, "User Permissions:");
    _v3_debug(V3_DEBUG_PACKET_PARSE, "user_id.............: %d",   m->user_id);
    _v3_debug(V3_DEBUG_PACKET_PARSE, "ping................: %d",   m->ping);
    _v3_debug(V3_DEBUG_PACKET_PARSE, "sequence............: %d",   m->sequence);

    _v3_func_leave("_v3_get_0x37");
    return true;
}/*}}}*/
_v3_net_message *
_v3_put_0x37(int sequence) {/*{{{*/
    _v3_net_message *m;
    _v3_msg_0x37 *mc;

    _v3_func_enter("_v3_put_0x37");
    // Build our message
    m = malloc(sizeof(_v3_net_message));
    memset(m, 0, sizeof(_v3_net_message));
    m->type = 0x37;
    m->len = sizeof(_v3_msg_0x37);

    // Build our message contents
    mc = malloc(sizeof(_v3_msg_0x37));
    memset(mc, 0, sizeof(_v3_msg_0x37));

    mc->type = 0x37;
    mc->user_id = v3_luser.id;
    mc->sequence = sequence;
    mc->ping  = v3_luser.ping;
    m->contents = mc;
    m->data = (char *)mc;

    _v3_func_leave("_v3_put_0x37");
    return m;
}/*}}}*/
/*}}}*/
// Message 0x3a (58) | TEXT TO SPEECH MESSAGE /*{{{*/
int
_v3_get_0x3a(_v3_net_message *msg) {/*{{{*/
    _v3_msg_0x3a *m;

    _v3_func_enter("_v3_get_0x3a");
    if (msg->len < sizeof(_v3_msg_0x3a)) {
        msg->data = realloc(msg->data, sizeof(_v3_msg_0x3a));
    }
    m = msg->contents = msg->data;
    m->msg = _v3_get_msg_string(&m->msglen, &m->msglen);
    _v3_debug(V3_DEBUG_PACKET_PARSE, "got text to speech message: %s", m->msg);

    _v3_func_leave("_v3_get_0x3a");
    return true;
}/*}}}*/
_v3_net_message *
_v3_put_0x3a(char *message) {/*{{{*/
    _v3_net_message *m;
    _v3_msg_0x3a *mc;

    _v3_func_enter("_v3_put_0x3a");
    m = malloc(sizeof(_v3_net_message));
    memset(m, 0, sizeof(_v3_net_message));
    m->type = 0x3a;
    m->len = sizeof(_v3_msg_0x3a) - sizeof(mc->msg);

    mc = malloc(m->len);
    memset(mc, 0, m->len);
    mc->type = 0x3a;
    if (message) {
        m->len += strlen(message);
        mc = realloc(mc, m->len);
        _v3_put_msg_string(&mc->msglen, message);
    }
    m->contents = mc;
    m->data = (char *)mc;

    _v3_func_leave("_v3_put_0x3a");
    return m;
}/*}}}*/
/*}}}*/
// Message 0x3b (59) | FORCE CHANNEL MOVE /*{{{*/
int
_v3_get_0x3b(_v3_net_message *msg) {/*{{{*/
    _v3_msg_0x3b *m;

    _v3_func_enter("_v3_get_0x3b");
    if (msg->len != sizeof(_v3_msg_0x3b)) {
        _v3_debug(V3_DEBUG_PACKET_PARSE, "expected %d bytes, but message is %d bytes", sizeof(_v3_msg_0x3b), msg->len);
        _v3_func_leave("_v3_get_0x3b");
        return false;
    }
    m = msg->contents = msg->data;
    _v3_debug(V3_DEBUG_PACKET_PARSE, "Force Channel Move:");
    _v3_debug(V3_DEBUG_PACKET_PARSE, "id..................: %d",   m->id);
    _v3_debug(V3_DEBUG_PACKET_PARSE, "channel id..........: %d",   m->channel_id);
    _v3_debug(V3_DEBUG_PACKET_PARSE, "error id............: %d",   m->error_id);

    _v3_func_leave("_v3_get_0x3b");
    return true;
}/*}}}*/
_v3_net_message *
_v3_put_0x3b(uint16_t id, uint16_t channel_id) {/*{{{*/
    _v3_net_message *m;
    _v3_msg_0x3b *mc;

    _v3_func_enter("_v3_put_0x3b");
    // Build our message
    m = malloc(sizeof(_v3_net_message));
    memset(m, 0, sizeof(_v3_net_message));
    m->type = 0x3b;
    m->len = sizeof(_v3_msg_0x3b);

    // Build our message contents
    mc = malloc(sizeof(_v3_msg_0x3b));
    memset(mc, 0, sizeof(_v3_msg_0x3b));

    mc->type = 0x3b;
    mc->id = id;
    mc->channel_id = channel_id;
    m->contents = mc;
    m->data = (char *)mc;

    _v3_func_leave("_v3_put_0x3b");
    return m;
}/*}}}*/
/*}}}*/
// Message 0x3c (60) | CODEC INFORMATION /*{{{*/
int
_v3_get_0x3c(_v3_net_message *msg) {/*{{{*/
    _v3_msg_0x3c *m;

    _v3_func_enter("_v3_get_0x3c");
    if (msg->len != sizeof(_v3_msg_0x3c)) {
        _v3_debug(V3_DEBUG_PACKET_PARSE, "expected %d bytes, but message is %d bytes", sizeof(_v3_msg_0x3c), msg->len);
        _v3_func_leave("_v3_get_0x3c");
        return false;
    }
    m = msg->contents = msg->data;
    _v3_debug(V3_DEBUG_PACKET_PARSE, "Codec Information:");
    _v3_debug(V3_DEBUG_PACKET_PARSE, "codec...............: %d",   m->codec);
    _v3_debug(V3_DEBUG_PACKET_PARSE, "format..............: %d",   m->codec_format);

    _v3_func_leave("_v3_get_0x3c");
    return true;
}/*}}}*/
/*}}}*/
// Message 0x3f (63) | PLAY WAVE FILE MESSAGE /*{{{*/
int
_v3_get_0x3f(_v3_net_message *msg) {/*{{{*/
    _v3_msg_0x3f *m;

    _v3_func_enter("_v3_get_0x3f");
    if (msg->len < sizeof(_v3_msg_0x3f)) {
        msg->data = realloc(msg->data, sizeof(_v3_msg_0x3f));
    }
    m = msg->contents = msg->data;
    m->msg = _v3_get_msg_string(&m->msglen, &m->msglen);
    _v3_debug(V3_DEBUG_PACKET_PARSE, "got play wave file message: %s", m->msg);

    _v3_func_leave("_v3_get_0x3f");
    return true;
}/*}}}*/
_v3_net_message *
_v3_put_0x3f(char *message) {/*{{{*/
    _v3_net_message *m;
    _v3_msg_0x3f *mc;

    _v3_func_enter("_v3_put_0x3f");
    m = malloc(sizeof(_v3_net_message));
    memset(m, 0, sizeof(_v3_net_message));
    m->type = 0x3f;
    m->len = sizeof(_v3_msg_0x3f) - sizeof(mc->msg);

    mc = malloc(m->len);
    memset(mc, 0, m->len);
    mc->type = 0x3f;
    if (message) {
        m->len += strlen(message);
        mc = realloc(mc, m->len);
        _v3_put_msg_string(&mc->msglen, message);
    }
    m->contents = mc;
    m->data = (char *)mc;

    _v3_func_leave("_v3_put_0x3f");
    return m;
}/*}}}*/
/*}}}*/
// Message 0x42 (66) | CHAT/RCON /*{{{*/
int
_v3_get_0x42(_v3_net_message *msg) {/*{{{*/
    _v3_msg_0x42 *m;

    _v3_func_enter("_v3_get_0x42");
    if (msg->len < sizeof(_v3_msg_0x42)) {
        msg->data = realloc(msg->data, sizeof(_v3_msg_0x42));
    }
    m = msg->contents = msg->data;
    switch (m->subtype) {
        case 2:
        case 3:
            m->msg = _v3_get_msg_string(&m->msglen, &m->msglen);
            _v3_debug(V3_DEBUG_PACKET_PARSE, "got chat/rcon message: %s", m->msg);
            break;
    }

    _v3_func_leave("_v3_get_0x42");
    return true;
}/*}}}*/
_v3_net_message *
_v3_put_0x42(uint16_t subtype, uint16_t user_id, char *message) {/*{{{*/
    _v3_net_message *m;
    _v3_msg_0x42 *mc;

    _v3_func_enter("_v3_put_0x42");
    // Build our message
    m = malloc(sizeof(_v3_net_message));
    memset(m, 0, sizeof(_v3_net_message));
    m->type = 0x42;

    // Build our message contents
    uint16_t base = sizeof(_v3_msg_0x42) - (sizeof(char *) + sizeof(uint16_t));
    uint16_t len  = base;
    mc = malloc(base);
    memset(mc, 0, base);
    mc->type = 0x42;
    mc->subtype = subtype;
    mc->user_id = user_id;
    if (message) {
        len += strlen(message) + 2;
        mc = realloc(mc, len);
        _v3_put_msg_string((char *)mc + base, message);
    }
    m->contents = mc;
    m->data = (char *)mc;
    m->len = len;

    _v3_func_leave("_v3_put_0x42");
    return m;
}/*}}}*/
/*}}}*/
// Message 0x46 (70) | USER OPTIONS /*{{{*/
int
_v3_get_0x46(_v3_net_message *msg) {/*{{{*/
    _v3_msg_0x46 *m;

    _v3_func_enter("_v3_get_0x46");
    if (msg->len != sizeof(_v3_msg_0x46)) {
        _v3_debug(V3_DEBUG_PACKET_PARSE, "expected %d bytes, but message is %d bytes", sizeof(_v3_msg_0x46), msg->len);
        _v3_func_leave("_v3_get_0x46");
        return false;
    }
    m = msg->contents = msg->data;
    _v3_debug(V3_DEBUG_PACKET_PARSE, "User Settings:");
    _v3_debug(V3_DEBUG_PACKET_PARSE, "user_id.............: %d",   m->user_id);
    _v3_debug(V3_DEBUG_PACKET_PARSE, "setting.............: %d",   m->setting);
    _v3_debug(V3_DEBUG_PACKET_PARSE, "value...............: %d",   m->value);

    _v3_func_leave("_v3_get_0x46");
    return true;
}/*}}}*/
_v3_net_message *
_v3_put_0x46(uint16_t user_id, uint16_t setting, uint16_t value) {/*{{{*/
    _v3_net_message *m;
    _v3_msg_0x46 *mc;

    _v3_func_enter("_v3_put_0x46");
    // Build our message
    m = malloc(sizeof(_v3_net_message));
    memset(m, 0, sizeof(_v3_net_message));
    m->type = 0x46;
    m->len = sizeof(_v3_msg_0x46);

    // Build our message contents
    mc = malloc(sizeof(_v3_msg_0x46));
    memset(mc, 0, sizeof(_v3_msg_0x46));

    mc->type = 0x46;
    mc->user_id = user_id;
    mc->setting = setting;
    mc->value   = value;
    m->contents = mc;
    m->data = (char *)mc;

    _v3_func_leave("_v3_put_0x46");
    return m;
}/*}}}*/
/*}}}*/
// Message 0x48 (72) | LOGIN /*{{{*/
_v3_net_message *
_v3_put_0x48(void) {/*{{{*/
    _v3_net_message *m;
    _v3_msg_0x48 *mc;

    _v3_func_enter("_v3_put_0x48");
    // Build our message
    m = malloc(sizeof(_v3_net_message));
    memset(m, 0, sizeof(_v3_net_message));
    m->type = 0x48;
    m->len = sizeof(_v3_msg_0x48);

    // Build our message contents
    mc = malloc(sizeof(_v3_msg_0x48));
    memset(mc, 0, sizeof(_v3_msg_0x48));

    mc->type = 0x48;
    mc->subtype = 2;
    mc->server_ip = v3_server.ip;
    mc->portmask = v3_server.port ^ 0xffff;
    mc->show_login_name = true;
    mc->auth_server_index = v3_server.auth_server_index;
    memcpy(mc->handshake, v3_server.handshake, 16);
    strncpy(mc->client_version, V3_CLIENT_VERSION, 16);
    strncpy(mc->proto_version,  V3_PROTO_VERSION, 16);
    if (strlen(v3_luser.password)) {
        _v3_hash_password((uint8_t *)v3_luser.password, (uint8_t *)mc->password_hash);
    }
    strncpy(mc->username, v3_luser.name, 32);
    strncpy(mc->phonetic, v3_luser.phonetic, 32);
    strncpy(mc->os, "WIN32", 64);
    m->contents = mc;
    m->data = (char *)mc;

    _v3_func_leave("_v3_put_0x48");
    return m;
}/*}}}*/
/*}}}*/
// Message 0x49 (73) | GET/REQUEST CHANNEL LIST MODIFICATION /*{{{*/
int
_v3_get_0x49(_v3_net_message *msg) {/*{{{*/
    /*
     * PACKET: message type: 0x49 (73)
     * PACKET: data length : 98
     * PACKET:     49 00 00 00 00 00 01 00 00 00 00 00 00 00 00 00      I...............
     * PACKET:     00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00      ................
     * PACKET:     00 00 00 00 00 00 00 00 16 00 02 00 00 00 00 00      ................
     * PACKET:     01 00 01 00 01 00 01 00 01 00 01 00 00 00 00 00      ................
     * PACKET:     00 00 00 00 01 00 00 00 01 00 00 00 00 00 00 00      ................
     * PACKET:     00 00 00 00 00 00 00 00 00 04 74 65 73 74 00 00      ..........test..
     * PACKET:     00 00                                                ..
     */
    _v3_msg_0x49 *m;

    _v3_func_enter("_v3_get_0x49");
    m = malloc(sizeof(_v3_msg_0x49));
    memcpy(m, msg->data, sizeof(_v3_msg_0x49) - sizeof(void *));
    m->channel = malloc(sizeof(v3_channel));
    _v3_get_msg_channel(msg->data+sizeof(_v3_msg_0x49) - sizeof(void *), m->channel);
    _v3_debug(V3_DEBUG_PACKET_PARSE, "got channel: id: %d | parent: %d | name: %s | phonetic: %s | comment: %s",
            m->channel->id,
            m->channel->parent,
            m->channel->name,
            m->channel->phonetic,
            m->channel->comment
            );
    msg->contents = m;

    _v3_func_leave("_v3_get_0x49");
    return true;
}/*}}}*/
_v3_net_message *
_v3_put_0x49(uint16_t subtype, uint16_t user_id, char *channel_password, _v3_msg_channel *channel) {/*{{{*/
    _v3_net_message *msg;
    struct _v3_net_message_0x49 *msgdata;

    _v3_func_enter("_v3_put_0x49");
    msg = malloc(sizeof(_v3_net_message));
    memset(msg, 0, sizeof(_v3_net_message));
    msg->type = 0x49;
    switch (subtype) {
        case V3_CHANGE_CHANNEL:
            msg->len = sizeof(_v3_msg_0x49)-sizeof(void *) + sizeof(_v3_msg_channel) - sizeof(void *) * 4 + 6;
            _v3_debug(V3_DEBUG_PACKET_PARSE, "allocating %d bytes", msg->len);
            msgdata = malloc(sizeof(_v3_msg_0x49)-sizeof(void *)+sizeof(_v3_msg_channel));
            memset(msgdata, 0, sizeof(_v3_msg_0x49)-sizeof(void *)+sizeof(_v3_msg_channel));
            msgdata->type = msg->type;
            msgdata->subtype = V3_CHANGE_CHANNEL;
            msgdata->user_id = user_id;
            if (channel_password != NULL && strlen(channel_password) != 0) {
                _v3_hash_password((uint8_t *)channel_password, (uint8_t *)msgdata->hash_password);
            }
            _v3_put_msg_channel(&msgdata->channel, channel);
            msg->data = (char *)msgdata;
            _v3_func_leave("_v3_put_0x49");
            return(msg);
        case V3_ADD_CHANNEL:
        case V3_MODIFY_CHANNEL:
            msg->len = sizeof(_v3_msg_0x49) - sizeof(void *) + sizeof(_v3_msg_channel) - sizeof(void *) * 4 + 6;
            if (channel->name) msg->len += strlen(channel->name);
            if (channel->phonetic) msg->len += strlen(channel->phonetic);
            if (channel->comment) msg->len += strlen(channel->comment);
            _v3_debug(V3_DEBUG_PACKET_PARSE, "allocating %d bytes", msg->len);
            msgdata = malloc(msg->len);
            memset(msgdata, 0, msg->len);
            msgdata->type = msg->type;
            msgdata->subtype = subtype;
            msgdata->user_id = user_id;
            if (channel_password != NULL && strlen(channel_password) != 0) {
                _v3_hash_password((uint8_t *)channel_password, (uint8_t *)msgdata->hash_password);
            }
            _v3_put_msg_channel(&msgdata->channel, channel);
            msg->data = (char *)msgdata;
            _v3_func_leave("_v3_put_0x49");
            return(msg);
        case V3_REMOVE_CHANNEL:
            msg->len = sizeof(_v3_msg_0x49)-sizeof(void *) + sizeof(_v3_msg_channel) - sizeof(void *) * 4 + 6;
            _v3_debug(V3_DEBUG_PACKET_PARSE, "allocating %d bytes", msg->len);
            msgdata = malloc(sizeof(_v3_msg_0x49)-sizeof(void *)+sizeof(_v3_msg_channel));
            memset(msgdata, 0, sizeof(_v3_msg_0x49)-sizeof(void *)+sizeof(_v3_msg_channel));
            msgdata->type = msg->type;
            msgdata->subtype = subtype;
            msgdata->user_id = user_id;
            if (channel_password != NULL && strlen(channel_password) != 0) {
                _v3_hash_password((uint8_t *)channel_password, (uint8_t *)msgdata->hash_password);
            }
            _v3_put_msg_channel(&msgdata->channel, channel);
            msg->data = (char *)msgdata;
            _v3_func_leave("_v3_put_0x49");
            return(msg);
    }
    _v3_debug(V3_DEBUG_PACKET_PARSE, "unknown channel message subtype: %02X", subtype);

    _v3_func_leave("_v3_put_0x49");
    return NULL;
}/*}}}*/
/*}}}*/
// Message 0x4a (74) | USER PERMISSIONS /*{{{*/
int
_v3_get_0x4a(_v3_net_message *msg) {/*{{{*/
    _v3_msg_0x4a *m;
    void *offset;

    _v3_func_enter("_v3_get_0x4a");
    m = msg->contents = msg->data;
    _v3_debug(V3_DEBUG_PACKET_PARSE, "subtype.......: %d", m->subtype);
    _v3_debug(V3_DEBUG_PACKET_PARSE, "error_id......: %d", m->error_id);
    _v3_debug(V3_DEBUG_PACKET_PARSE, "unknown 1.....: %d", m->unknown_1);
    _v3_debug(V3_DEBUG_PACKET_PARSE, "count.........: %d", m->count);
    _v3_debug(V3_DEBUG_PACKET_PARSE, "start_id......: %d", m->start_id);
    _v3_debug(V3_DEBUG_PACKET_PARSE, "end_id........: %d", m->end_id);
    _v3_debug(V3_DEBUG_PACKET_PARSE, "unknown 2.....: %d", m->unknown_2);
    if (m->error_id != 0) {
        _v3_func_leave("_v3_get_0x4a");
        return true;
    }
    switch (m->subtype) {
        case V3_USERLIST_OPEN:
        case V3_USERLIST_ADD:
        case V3_USERLIST_MODIFY:
            {
                int i;
                _v3_msg_0x4a_account *msub = malloc(sizeof(_v3_msg_0x4a_account));
                memcpy(msub, m, sizeof(_v3_msg_0x4a));
                msg->contents = msub;

                msub->acct_list_count = msub->header.count;
                if (msub->header.count > 0) {
                    msub->acct_list = calloc(msub->header.count, sizeof(msub->acct_list[0]));

                    for (i = 0, offset = msg->data + sizeof(msub->header);i < msub->header.count; i++) {
                        v3_account *account = msub->acct_list[i] = malloc(sizeof(v3_account));
                        offset += _v3_get_msg_account(offset, account);
                    }
                } else {
                    msub->acct_list = NULL;
                }
            }
            break;
        case V3_USERLIST_REMOVE:
        case V3_USERLIST_LUSER:
        case V3_USERLIST_CHANGE_OWNER:
            {
                if (msg->len != sizeof(_v3_msg_0x4a_perms)) {
                    _v3_debug(V3_DEBUG_PACKET_PARSE, "expected %d bytes, but message is %d bytes", sizeof(_v3_msg_0x4a_perms), msg->len);
                    _v3_func_leave("_v3_get_0x4a");
                    return false;
                }
                //_v3_msg_0x4a_perms *msub = (_v3_msg_0x4a_perms *)m;
            }
            break;
        default:
            _v3_debug(V3_DEBUG_PACKET_PARSE, "unknown 0x4a subtype %02x", m->subtype);
            _v3_func_leave("_v3_get_0x4a");
            return false;
    }

    _v3_func_leave("_v3_get_0x4a");
    return true;
}/*}}}*/
_v3_net_message *
_v3_put_0x4a(uint8_t subtype, v3_account *account, v3_account *account2) {/*{{{*/
    _v3_net_message *m;
    _v3_msg_0x4a *mc;

    _v3_func_enter("_v3_put_0x4a");
    // Build our message
    m = malloc(sizeof(_v3_net_message));
    memset(m, 0, sizeof(_v3_net_message));
    m->type = 0x4a;
    switch (subtype) {
        case V3_USERLIST_OPEN:
        case V3_USERLIST_CLOSE:
            m->len = sizeof(_v3_msg_0x4a);
            mc = malloc(m->len);
            memset(mc, 0, m->len);
            if (subtype == V3_USERLIST_OPEN && account) {
                mc->start_id = account->perms.account_id;
            }
            break;
        case V3_USERLIST_ADD:
        case V3_USERLIST_MODIFY:
            m->len = sizeof(_v3_msg_0x4a) + sizeof(account->perms) + strlen(account->username) + 2 + strlen(account->owner) + 2 + strlen(account->notes) + 2 + strlen(account->lock_reason) + 2 + ((account->chan_admin_count * 2) + 2) + ((account->chan_auth_count * 2) + 2);
            mc = malloc(m->len);
            memset(mc, 0, m->len);
            mc->count = 1;
            _v3_put_msg_account((uint8_t *)mc + sizeof(_v3_msg_0x4a), account);
            break;
        case V3_USERLIST_REMOVE:
            m->len = sizeof(_v3_msg_0x4a_perms);
            mc = malloc(m->len);
            memset(mc, 0, m->len);
            ((_v3_msg_0x4a_perms *)mc)->header.count = 1;
            ((_v3_msg_0x4a_perms *)mc)->perms.account_id = account->perms.account_id;
            break;
        case V3_USERLIST_CHANGE_OWNER:
            m->len = sizeof(_v3_msg_0x4a_perms);
            mc = malloc(m->len);
            memset(mc, 0, m->len);
            ((_v3_msg_0x4a_perms *)mc)->header.count = 1;
            ((_v3_msg_0x4a_perms *)mc)->perms.account_id = account->perms.account_id;
            ((_v3_msg_0x4a_perms *)mc)->perms.replace_owner_id = account2->perms.account_id;
            break;
        default:
            break;
    }
    mc->type = 0x4a;
    mc->subtype = subtype;
    m->contents = mc;
    m->data = (char *)mc;

    _v3_func_leave("_v3_put_0x4a");
    return m;
}/*}}}*/
int
_v3_destroy_0x4a(_v3_net_message *msg) {/*{{{*/
    _v3_msg_0x4a *m;
    int i;

    _v3_func_enter("_v3_destroy_0x4a");
    m = msg->contents;
    switch (m->subtype) {
        case V3_USERLIST_OPEN:
            {
                _v3_msg_0x4a_account *msub = (_v3_msg_0x4a_account *)m;

                for (i = 0; i < msub->acct_list_count; i++) {
                    v3_account *acct = msub->acct_list[i];
                    _v3_debug(V3_DEBUG_PACKET_PARSE, "freeing resources for account %d: %s", acct->perms.account_id, acct->username);
                    v3_free_account(acct);
                }
                if (msub->acct_list) {
                    free(msub->acct_list);
                }
            }
            break;
        default:
            break;
    }

    _v3_func_leave("_v3_destroy_0x4a");
    return true;
}/*}}}*/
/*}}}*/
// Message 0x4b (75) | TIMESTAMP /*{{{*/
_v3_net_message *
_v3_put_0x4b(void) {/*{{{*/
    _v3_net_message *m;
    _v3_msg_0x4b *mc;

    _v3_func_enter("_v3_put_0x4b");
    // Build our message
    m = malloc(sizeof(_v3_net_message));
    memset(m, 0, sizeof(_v3_net_message));
    m->type = 0x4b;
    m->len = sizeof(_v3_msg_0x4b);

    // Build our message contents
    mc = malloc(sizeof(_v3_msg_0x4b));
    memset(mc, 0, sizeof(_v3_msg_0x4b));

    mc->type = 0x4b;
    mc->timestamp = time(NULL);
    m->contents = mc;
    m->data = (char *)mc;

    _v3_func_leave("_v3_put_0x4b");
    return m;
}/*}}}*/
/*}}}*/
// Message 0x4c (76) | SERVER PROPERTIES /*{{{*/
int
_v3_get_0x4c(_v3_net_message *msg) {/*{{{*/
    _v3_msg_0x4c *m;
    uint16_t len;

    _v3_func_enter("_v3_get_0x4c");
    if (msg->len < sizeof(_v3_msg_0x4c)) {
        msg->data = realloc(msg->data, sizeof(_v3_msg_0x4c));
    }
    m = msg->contents = msg->data;
    if (msg->len > sizeof(_v3_msg_0x4c) - sizeof(m->value)) {
        m->value = _v3_get_msg_string(&m->value, &len);
    } else {
        m->value = NULL;
    }

    _v3_func_leave("_v3_get_0x4c");
    return true;
}/*}}}*/
_v3_net_message *
_v3_put_0x4c(uint16_t subtype, uint16_t property, uint16_t transaction_id, char *value) {/*{{{*/
    _v3_net_message *m;
    _v3_msg_0x4c *mc;

    _v3_func_enter("_v3_put_0x4c");
    m = malloc(sizeof(_v3_net_message));
    memset(m, 0, sizeof(_v3_net_message));
    m->type = 0x4c;
    m->len = sizeof(_v3_msg_0x4c) - sizeof(mc->value);

    mc = malloc(m->len);
    memset(mc, 0, m->len);
    mc->type = 0x4c;
    mc->subtype = subtype;
    mc->property = property;
    mc->transaction_id = transaction_id;
    if (value) {
        m->len += sizeof(uint16_t) + strlen(value);
        mc = realloc(mc, m->len);
        _v3_put_msg_string(&mc->value, value);
    }
    m->contents = mc;
    m->data = (char *)mc;

    _v3_func_leave("_v3_put_0x4c");
    return m;
}/*}}}*/
/*}}}*/
// Message 0x50 (80) | MOTD /*{{{*/
int
_v3_get_0x50(_v3_net_message *msg) {/*{{{*/
    _v3_func_enter("_v3_get_0x50");

    msg->contents = msg->data;

    _v3_func_leave("_v3_get_0x50");
    return true;
}/*}}}*/
//* }}} */
// Message 0x52 (82) | SOUND DATA /*{{{*/
int
_v3_get_0x52(_v3_net_message *msg) {/*{{{*/
    _v3_msg_0x52 *m;

    _v3_func_enter("_v3_get_0x52");
    if (msg->len < sizeof(_v3_msg_0x52)) {
        _v3_debug(V3_DEBUG_PACKET_PARSE, "expected more than %d bytes, but message is %d bytes", sizeof(_v3_msg_0x52), msg->len);
        _v3_func_leave("_v3_get_0x52");
        return false;
    }
    m = malloc(sizeof(_v3_msg_0x52));
    memcpy(m, msg->data, sizeof(_v3_msg_0x52));
    _v3_debug(V3_DEBUG_PACKET_PARSE, "subtype.......: %d", m->subtype);
    _v3_debug(V3_DEBUG_PACKET_PARSE, "user_id.......: %d", m->user_id);
    _v3_debug(V3_DEBUG_PACKET_PARSE, "codec.........: %d", m->codec);
    _v3_debug(V3_DEBUG_PACKET_PARSE, "codec_format..: %d", m->codec_format);
    _v3_debug(V3_DEBUG_PACKET_PARSE, "send_type.....: %d", m->send_type);
    _v3_debug(V3_DEBUG_PACKET_PARSE, "unknown 1.....: %d", m->unknown_1);
    _v3_debug(V3_DEBUG_PACKET_PARSE, "data_length...: %d", m->data_length);
    _v3_debug(V3_DEBUG_PACKET_PARSE, "pcm_length....: %d", m->pcm_length);
    switch (m->subtype) {
        case V3_AUDIO_START:
            {
                _v3_msg_0x52_0x00 *msub = (_v3_msg_0x52_0x00 *)m;
                msub = realloc(m, sizeof(_v3_msg_0x52_0x00));
                memcpy(msub, msg->data, sizeof(_v3_msg_0x52_0x00));
                _v3_debug(V3_DEBUG_PACKET_PARSE, "unknown 4.....: %d", msub->unknown_4);
                _v3_debug(V3_DEBUG_PACKET_PARSE, "unknown 5.....: %d", msub->unknown_5);
                _v3_debug(V3_DEBUG_PACKET_PARSE, "unknown 6.....: %d", msub->unknown_6);
                _v3_debug(V3_DEBUG_PACKET_PARSE, "unknown 7.....: %d", msub->unknown_7);
                _v3_debug(V3_DEBUG_PACKET_PARSE, "user %d started transmitting", msub->header.user_id);
                msg->contents = msub;
                _v3_func_leave("_v3_get_0x52");
                return true;
            }
        case V3_AUDIO_DATA:
            {
                _v3_msg_0x52_0x01_in *msub = (_v3_msg_0x52_0x01_in *)m;
                msub = realloc(m, sizeof(_v3_msg_0x52_0x01_in));
                memcpy(msub, msg->data, sizeof(_v3_msg_0x52_0x01_in) - sizeof(msub->data));
                _v3_debug(V3_DEBUG_PACKET_PARSE, "unknown 4.....: %d", msub->unknown_4);
                _v3_debug(V3_DEBUG_PACKET_PARSE, "unknown 5.....: %d", msub->unknown_5);
                _v3_debug(V3_DEBUG_PACKET_PARSE, "allocated %d bytes for audio packet", sizeof(_v3_msg_0x52_0x01_in));
                _v3_debug(V3_DEBUG_PACKET_PARSE, "received an audio packet from user id %d", msub->header.user_id);
                if (msub->header.data_length > 0xffff) {
                    // we don't need more than 65535 bytes from a packet as this should never happen
                    _v3_debug(V3_DEBUG_PACKET_PARSE, "data length is too large: %d bytes", msub->header.data_length);
                    free(msub);
                    _v3_func_leave("_v3_get_0x52");
                    return false;
                }
                _v3_debug(V3_DEBUG_PACKET_PARSE, "allocating %d bytes for audio data", msub->header.data_length);
                msub->data = malloc(msub->header.data_length);
                memcpy(msub->data, msg->data + (sizeof(_v3_msg_0x52_0x01_in) - sizeof(msub->data)), msub->header.data_length);
                msg->contents = msub;
                _v3_func_leave("_v3_get_0x52");
                return true;
            }
            break;
        case V3_AUDIO_STOP:
            {
                _v3_msg_0x52_0x02 *msub = (_v3_msg_0x52_0x02 *)m;
                msub = realloc(m, sizeof(_v3_msg_0x52_0x02));
                memcpy(msub, msg->data, sizeof(_v3_msg_0x52_0x02));
                _v3_debug(V3_DEBUG_PACKET_PARSE, "unknown 4.....: %d", msub->unknown_4);
                _v3_debug(V3_DEBUG_PACKET_PARSE, "unknown 5.....: %d", msub->unknown_5);
                _v3_debug(V3_DEBUG_PACKET_PARSE, "user %d stopped transmitting", msub->header.user_id);
                msg->contents = msub;
                _v3_func_leave("_v3_get_0x52");
                return true;
            }
        case V3_AUDIO_MUTE:
            {
                _v3_msg_0x52_0x03 *msub = (_v3_msg_0x52_0x03 *)m;
                _v3_debug(V3_DEBUG_PACKET_PARSE, "user %d is transmit muted from server", msub->header.user_id);
                msg->contents = msub;
                _v3_func_leave("_v3_get_0x52");
                return true;
            }
        case V3_AUDIO_START_LOGIN:
            {
                _v3_msg_0x52_0x03 *msub = (_v3_msg_0x52_0x03 *)m;
                _v3_debug(V3_DEBUG_PACKET_PARSE, "user %d was transmitting before login", msub->header.user_id);
                msg->contents = msub;
                _v3_func_leave("_v3_get_0x52");
                return true;
            }
        case V3_AUDIO_QUEUE_AVAIL:
            {
                _v3_debug(V3_DEBUG_PACKET_PARSE, "user %d has the floor", m->user_id);
                msg->contents = m;
                _v3_func_leave("_v3_get_0x52");
                return true;
            }
        case V3_AUDIO_QUEUE_TAKEN:
            {
                _v3_debug(V3_DEBUG_PACKET_PARSE, "user %d doesn't have the floor", m->user_id);
                msg->contents = m;
                _v3_func_leave("_v3_get_0x52");
                return true;
            }
    }
    _v3_debug(V3_DEBUG_PACKET_PARSE, "unknown 0x52 subtype %02x", m->subtype);
    free(m);

    _v3_func_leave("_v3_get_0x52");
    return false;
}/*}}}*/
_v3_net_message *
_v3_put_0x52(uint8_t subtype, uint16_t codec, uint16_t codec_format, uint32_t pcmlength, uint32_t length, void *data) {/*{{{*/
    _v3_net_message *msg;
    _v3_msg_0x52_0x01_out *msgdata;

    _v3_func_enter("_v3_put_0x52");
    msg = malloc(sizeof(_v3_net_message));
    memset(msg, 0, sizeof(_v3_net_message));

    /*
     * First we go through and create our main message structure
     */
    switch (subtype) {
        case V3_AUDIO_START:
            _v3_debug(V3_DEBUG_PACKET_PARSE, "sending 0x52 subtype 0x00 size %d", sizeof(_v3_msg_0x52_0x00));
            msgdata = malloc(sizeof(_v3_msg_0x52_0x00));
            memset(msgdata, 0, sizeof(_v3_msg_0x52_0x00));
            msg->len = sizeof(_v3_msg_0x52_0x00);
            msgdata->unknown_4 = htons(1);
            msgdata->unknown_5 = htons(2);
            msgdata->unknown_6 = htons(1);
            break;
        case V3_AUDIO_DATA:
            _v3_debug(V3_DEBUG_PACKET_PARSE, "sending 0x52 subtype 0x01 header size %d data size %d", sizeof(_v3_msg_0x52_0x01_out), length);
            msgdata = malloc(sizeof(_v3_msg_0x52_0x01_out));
            memset(msgdata, 0, sizeof(_v3_msg_0x52_0x01_out));
            msg->len = sizeof(_v3_msg_0x52_0x01_out) + length;
            _v3_debug(V3_DEBUG_PACKET_PARSE, "setting pcm length to %d", pcmlength);
            msgdata->header.pcm_length = pcmlength;
            msgdata->header.data_length = length;
            // TODO: we really need to figure out what these values are
            msgdata->unknown_4 = htons(1);
            msgdata->unknown_5 = htons(2);
            msgdata->unknown_6 = htons(1);
            break;
        case V3_AUDIO_STOP:
            _v3_debug(V3_DEBUG_PACKET_PARSE, "sending 0x52 subtype 0x02 size %d", sizeof(_v3_msg_0x52_0x02));
            msgdata = malloc(sizeof(_v3_msg_0x52_0x02));
            memset(msgdata, 0, sizeof(_v3_msg_0x52_0x02));
            msg->len = sizeof(_v3_msg_0x52_0x02);
            break;
        default:
            free(msg);
            _v3_func_leave("_v3_put_0x52");
            return NULL;
    }
    msgdata->header.type = msg->type = 0x52;
    msgdata->header.subtype = subtype;
    msgdata->header.codec = codec;
    msgdata->header.codec_format = codec_format;

    /*
     * Now we allocate the actual msg->data for the network packet representation
     */
    _v3_debug(V3_DEBUG_MEMORY, "allocating %d bytes for data", msg->len);
    msg->data = malloc(msg->len);
    memset(msg->data, 0, msg->len);

    /*
     * Next, copy all of the data into the newly allocated memory
     */
    switch (subtype) {
        case V3_AUDIO_START:
            memcpy(msg->data, msgdata, sizeof(_v3_msg_0x52_0x00));
            break;
        case V3_AUDIO_DATA:
            memcpy(msg->data, msgdata, sizeof(_v3_msg_0x52_0x01_out));
            memcpy(msg->data + sizeof(_v3_msg_0x52_0x01_out), data, length);
            break;
        case V3_AUDIO_STOP:
            memcpy(msg->data, msgdata, sizeof(_v3_msg_0x52_0x02));
            break;
    }
    free(msgdata);

    _v3_func_leave("_v3_put_0x52");
    return msg;
}/*}}}*/
int
_v3_destroy_0x52(_v3_net_message *msg) {/*{{{*/
    _v3_msg_0x52 *m;
    _v3_msg_0x52_0x01_in *msubin;

    _v3_func_enter("_v3_destroy_0x52");
    m = msg->contents;
    switch (m->subtype) {
        case V3_AUDIO_DATA:
            msubin = (_v3_msg_0x52_0x01_in *)m;
            if (msubin->data) {
                _v3_debug(V3_DEBUG_PACKET_PARSE, "freeing %d bytes of audio data", msubin->header.data_length);
                free(msubin->data);
                msubin->data = NULL;
            }
            break;
    }

    _v3_func_leave("_v3_destroy_0x52");
    return true;
}/*}}}*/
/*}}}*/
// Message 0x53 (83) | CHANNEL CHANGE /*{{{*/
int
_v3_get_0x53(_v3_net_message *msg) {/*{{{*/
    _v3_func_enter("_v3_get_0x53");

    if (msg->len != sizeof(_v3_msg_0x53)) {
        _v3_debug(V3_DEBUG_PACKET_PARSE, "expected %d bytes, but message is %d bytes", sizeof(_v3_msg_0x53), msg->len);
        _v3_func_leave("_v3_get_0x53");
        return false;
    }
    msg->contents = msg->data;

    _v3_func_leave("_v3_get_0x53");
    return true;
}/*}}}*/
/*}}}*/
// Message 0x57 (87) | SERVER INFORMATION/*{{{*/
int
_v3_get_0x57(_v3_net_message *msg) {/*{{{*/
    _v3_func_enter("_v3_get_0x57");

    if (msg->len != sizeof(_v3_msg_0x57)) {
        _v3_debug(V3_DEBUG_PACKET_PARSE, "expected %d bytes, but message is %d bytes", sizeof(_v3_msg_0x57), msg->len);
        _v3_func_leave("_v3_get_0x57");
        return false;
    }
    msg->contents = msg->data;

    _v3_func_leave("_v3_get_0x57");
    return true;
}/*}}}*/
/*}}}*/
// Message 0x58 (88) | PHANTOM /*{{{*/
int
_v3_get_0x58(_v3_net_message *msg) {/*{{{*/
    _v3_msg_0x58 *m;

    _v3_func_enter("_v3_get_0x58");
    if (msg->len != sizeof(_v3_msg_0x58)) {
        _v3_debug(V3_DEBUG_PACKET_PARSE, "expected %d bytes, but message is %d bytes", sizeof(_v3_msg_0x58), msg->len);
        _v3_func_leave("_v3_get_0x58");
        return false;
    }
    m = msg->contents = msg->data;
    _v3_debug(V3_DEBUG_PACKET_PARSE, "Phantom:");
    _v3_debug(V3_DEBUG_PACKET_PARSE, "subtype.............: %d", m->subtype);
    _v3_debug(V3_DEBUG_PACKET_PARSE, "error_id............: %d", m->error_id);
    _v3_debug(V3_DEBUG_PACKET_PARSE, "real_user_id........: %d", m->real_user_id);
    _v3_debug(V3_DEBUG_PACKET_PARSE, "phantom_user_id.....: %d", m->phantom_user_id);
    _v3_debug(V3_DEBUG_PACKET_PARSE, "channel_id..........: %d", m->channel_id);
    _v3_debug(V3_DEBUG_PACKET_PARSE, "log_error...........: %d", m->log_error);

    _v3_func_leave("_v3_get_0x58");
    return true;
}/*}}}*/
_v3_net_message *
_v3_put_0x58(uint16_t subtype, uint16_t channel, uint16_t phantom_user_id) {/*{{{*/
    _v3_net_message *m;
    _v3_msg_0x58 *mc;

    _v3_func_enter("_v3_put_0x58");
    // Build our message
    m = malloc(sizeof(_v3_net_message));
    memset(m, 0, sizeof(_v3_net_message));
    m->type = 0x58;
    m->len = sizeof(_v3_msg_0x58);

    // Build our message contents
    mc = malloc(sizeof(_v3_msg_0x58));
    memset(mc, 0, sizeof(_v3_msg_0x58));
    mc->type = 0x58;
    mc->subtype = subtype;
    mc->real_user_id = v3_luser.id;
    switch (subtype) {
        case V3_PHANTOM_ADD:
            mc->channel_id = channel;
            break;
        case V3_PHANTOM_REMOVE:
            mc->phantom_user_id = phantom_user_id;
            break;
    }
    m->contents = mc;
    m->data = (char *)mc;

    _v3_func_leave("_v3_put_0x58");
    return m;
}/*}}}*/
/*}}}*/
// Message 0x59 (89) | ERROR MESSAGE /*{{{*/
int
_v3_get_0x59(_v3_net_message *msg) {/*{{{*/
    _v3_msg_0x59 *m;
    uint16_t plen;
    uint16_t len;

    _v3_func_enter("_v3_get_0x59");
    plen = sizeof(_v3_msg_0x59) - sizeof(void *);
    m = malloc(sizeof(_v3_msg_0x59));
    memset(m, 0, sizeof(_v3_msg_0x59));
    memcpy(m,  msg->data, plen);
    m->message = (char *)_v3_get_msg_string(msg->data+plen, &len);
    msg->contents = m;

    _v3_func_leave("_v3_get_0x59");
    return true;
}/*}}}*/
/*}}}*/
// Message 0x5a (90) | PRIVATE CHAT /*{{{*/
int
_v3_get_0x5a(_v3_net_message *msg) {/*{{{*/
    _v3_msg_0x5a *m;

    _v3_func_enter("_v3_get_0x5a");
    if (msg->len < sizeof(_v3_msg_0x5a)) {
        msg->data = realloc(msg->data, sizeof(_v3_msg_0x5a));
    }
    m = msg->contents = msg->data;
    switch (m->subtype) {
        case 2:
            m->msg = _v3_get_msg_string(&m->msglen, &m->msglen);
            _v3_debug(V3_DEBUG_PACKET_PARSE, "got private chat message: %s", m->msg);
            break;
    }

    _v3_func_leave("_v3_get_0x5a");
    return true;
}/*}}}*/
_v3_net_message *
_v3_put_0x5a(uint16_t subtype, uint16_t user1, uint16_t user2, char *message) {/*{{{*/
    _v3_net_message *m;
    _v3_msg_0x5a *mc;

    _v3_func_enter("_v3_put_0x5a");
    // Build our message
    m = malloc(sizeof(_v3_net_message));
    memset(m, 0, sizeof(_v3_net_message));
    m->type = 0x5a;

    // Build our message contents
    uint16_t base = sizeof(_v3_msg_0x5a) - (sizeof(char *) + sizeof(uint16_t));
    uint16_t len  = base;
    mc = malloc(base);
    memset(mc, 0, base);
    mc->type = 0x5a;
    mc->subtype = subtype;
    mc->user1 = user1;
    mc->user2 = user2;
    if (message) {
        len += strlen(message) + 2;
        mc = realloc(mc, len);
        _v3_put_msg_string((char *)mc + base, message);
    }
    m->contents = mc;
    m->data = (char *)mc;
    m->len = len;

    _v3_func_leave("_v3_put_0x5a");
    return m;
}/*}}}*/
/*}}}*/
// Message 0x5c (92) | HASH TABLE SCRAMBLE  /*{{{*/
int
_v3_get_0x5c(_v3_net_message *msg) {/*{{{*/
    _v3_msg_0x5c *m;

    _v3_func_enter("_v3_get_0x5c");
    if (msg->len != sizeof(_v3_msg_0x5c)) {
        _v3_debug(V3_DEBUG_PACKET_PARSE, "expected %d bytes, but message is %d bytes", sizeof(_v3_msg_0x5c), msg->len);
        _v3_func_leave("_v3_get_0x5c");
        return false;
    }
    m = msg->contents = msg->data;
    _v3_debug(V3_DEBUG_PACKET_PARSE, "Hash Scramble:");
    _v3_debug(V3_DEBUG_PACKET_PARSE, "subtype.............: %d",   m->subtype);
    _v3_debug(V3_DEBUG_PACKET_PARSE, "sum 1...............: %d",   m->sum_1);
    _v3_debug(V3_DEBUG_PACKET_PARSE, "sum 2...............: %d",   m->sum_2);

    _v3_func_leave("_v3_get_0x5c");
    return true;
}/*}}}*/
_v3_net_message *
_v3_put_0x5c(uint8_t subtype) {/*{{{*/
    _v3_net_message *m;
    _v3_msg_0x5c *mc;

    _v3_func_enter("_v3_put_0x5c");
    // Build our message
    m = malloc(sizeof(_v3_net_message));
    memset(m, 0, sizeof(_v3_net_message));
    m->type = 0x5c;
    m->len = sizeof(_v3_msg_0x5c);

    // Build our message contents
    mc = malloc(sizeof(_v3_msg_0x5c));
    memset(mc, 0, sizeof(_v3_msg_0x5c));
    mc->type = 0x5c;
    mc->subtype = subtype;

    // We may want to store rand() values for later reference.
    switch (subtype) {
        case 0:
            mc->sum_2 = _v3_msg5c_gensum(0xBAADF00D, 16);
            break;
        case 1:
            mc->sum_1 = (uint8_t)rand();
            break;
        case 2:
            mc->sum_2 = _v3_msg5c_gensum(0x0DBAADF0, 16);
            break;
        case 3:
            {
                uint8_t out[9];
                snprintf((char *)out, 8, "%08X", (uint32_t)rand());
                mc->sum_2 = _v3_msg5c_scramble(out);
            }
            break;
        case 4:
            mc->sum_2 = _v3_msg5c_gensum(0xBAADF00D, 32);
            break;
        case 5:
            {
                uint8_t out[9];
                snprintf((char *)out, 8, "%08x", (uint32_t)rand());
                mc->sum_2 = _v3_msg5c_scramble(out);
            }
            break;
        case 6:
            mc->sum_2 = _v3_msg5c_gensum(0xBAADF00D, 16);
            break;
        case 7:
            {
                uint8_t out[8];
                snprintf((char *)out, 8, "%08X", (uint32_t)rand());
                mc->sum_2 = _v3_msg5c_scramble(out);
            }
            break;
        case 8:
            mc->sum_2 = _v3_msg5c_gensum(0xBAADF00D, 32);
            break;
        case 9:
            mc->sum_2 = _v3_msg5c_gensum(0xBAADF00D, 16);
            break;
        case 10:
            mc->sum_2 = !rand();
            break;
    }
    m->contents = mc;
    m->data = (char *)mc;

    _v3_func_leave("_v3_put_0x5c");
    return m;
}/*}}}*/
uint32_t
_v3_msg5c_scramble(uint8_t* in) {/*{{{*/
    uint32_t i, out = 0;

    for (i = 0; i < 8; i++) {
        out = (out >> 8) ^ _v3_hash_table[(uint8_t)(in[i] ^ out)];
    }

    return out;
}/*}}}*/
uint32_t
_v3_msg5c_gensum(uint32_t seed, uint32_t iterations) {/*{{{*/
    uint32_t i, j, out = 0;
    uint32_t *ecx = (uint32_t *)malloc(sizeof(uint32_t) * iterations);

    for (i = 0; i < iterations; i++) {
        ecx[i] = seed;
    }
    for (i = 0; i < iterations; i++) {
        for (j = 0; j < 4; j++) {
            uint8_t offset = ((ecx[i] >> (j * 8)) ^ out) & 0xff;
            out = (out >> 8) ^ _v3_hash_table[offset];
        }
    }
    uint8_t formatted[9] = { 0 };
    snprintf((char *)formatted, 9, "%08x", out);
    free(ecx);

    return _v3_msg5c_scramble(formatted);
}/*}}}*/
/*}}}*/
// Message 0x5d (93) | USER LIST MODIFICATION /*{{{*/
int
_v3_get_0x5d(_v3_net_message *msg) {/*{{{*/
    _v3_msg_0x5d *m;
    int ctr;
    void *offset;

    _v3_func_enter("_v3_get_0x5d");
    m = malloc(sizeof(_v3_msg_0x5d));
    memcpy(m, msg->data, 8);
    _v3_debug(V3_DEBUG_PACKET_PARSE, "packet contains %d users.  message subtype %02X", m->user_count, m->subtype);
    _v3_debug(V3_DEBUG_PACKET_PARSE, "allocating %d bytes for userlist packet", sizeof(_v3_msg_0x5d));
    m = realloc(m, sizeof(_v3_msg_0x5d));
    _v3_debug(V3_DEBUG_PACKET_PARSE, "allocating %d bytes (%d users * %d bytes)", m->user_count*sizeof(_v3_msg_user), m->user_count, sizeof(_v3_msg_user));
    m->user_list = calloc(m->user_count, sizeof(_v3_msg_user));
    for (ctr = 0, offset = msg->data + 8; ctr < m->user_count; ctr++) {
        offset += _v3_get_msg_user(offset, &m->user_list[ctr]);
        _v3_debug(V3_DEBUG_PACKET_PARSE, "got user: id              : %d", m->user_list[ctr].id);
        _v3_debug(V3_DEBUG_PACKET_PARSE, "          channel         : %d", m->user_list[ctr].channel);
        _v3_debug(V3_DEBUG_PACKET_PARSE, "          name            : %s", m->user_list[ctr].name);
        _v3_debug(V3_DEBUG_PACKET_PARSE, "          phonetic        : %s", m->user_list[ctr].phonetic);
        _v3_debug(V3_DEBUG_PACKET_PARSE, "          comment         : %s", m->user_list[ctr].comment);
        _v3_debug(V3_DEBUG_PACKET_PARSE, "          integration_text: %s", m->user_list[ctr].integration_text);
        _v3_debug(V3_DEBUG_PACKET_PARSE, "          url             : %s", m->user_list[ctr].url);
        _v3_debug(V3_DEBUG_PACKET_PARSE, "          flags           : %d", m->user_list[ctr].bitfield);
        _v3_debug(V3_DEBUG_PACKET_PARSE, "          rank id         : %d", m->user_list[ctr].rank_id);
    }
    m->lobby = &m->user_list[0];
    msg->contents = m;

    _v3_func_leave("_v3_get_0x5d");
    return true;
}/*}}}*/
_v3_net_message *
_v3_put_0x5d(uint16_t subtype, uint16_t count, v3_user *user) {/*{{{*/
    _v3_net_message *msg;
    _v3_msg_0x5d *msgdata;
    int len = 0;
    int ctr;

    _v3_func_enter("_v3_put_0x5d");
    msg = malloc(sizeof(_v3_net_message));
    memset(msg, 0, sizeof(_v3_net_message));

    msgdata = malloc(sizeof(_v3_msg_0x5d));
    memset(msgdata, 0, sizeof(_v3_msg_0x5d));
    msgdata->type = 0x5d;
    msgdata->subtype = subtype;
    msgdata->user_count = count;

    _v3_debug(V3_DEBUG_MEMORY, "allocating %d bytes for data", V3_MAX_USER_SIZE * count + sizeof(_v3_msg_0x5d));
    msg->data = malloc(V3_MAX_USER_SIZE * count + sizeof(_v3_msg_0x5d));
    memset(msg->data, 0, V3_MAX_USER_SIZE * count + sizeof(_v3_msg_0x5d));
    memcpy(msg->data, msgdata, 8); // only the first 8 bytes are sent, the rest is user structures
    len += 8;
    for (ctr = 0; ctr < count; ctr++) {
        _v3_debug(V3_DEBUG_PACKET_PARSE, "putting user %d: %d", ctr, user[ctr].id);
        len += _v3_put_msg_user((void *)(msg->data+len), &user[ctr]);
    }
    msg->len = len;
    free(msgdata);

    _v3_func_leave("_v3_put_0x5d");
    return msg;
}/*}}}*/
int
_v3_destroy_0x5d(_v3_net_message *msg) {/*{{{*/
    _v3_msg_0x5d *m;
    int ctr;

    m = msg->contents;
    _v3_func_enter("_v3_destroy_0x5d");
    for (ctr = 0; ctr < m->user_count; ctr++) {
        _v3_debug(V3_DEBUG_PACKET_PARSE, "freeing resources for user %d: %s", m->user_list[ctr].id, m->user_list[ctr].name);
        free(m->user_list[ctr].name);
        free(m->user_list[ctr].phonetic);
        free(m->user_list[ctr].comment);
        free(m->user_list[ctr].integration_text);
        free(m->user_list[ctr].url);
    }
    free(m->user_list);

    _v3_func_leave("_v3_destroy_0x5d");
    return true;
}/*}}}*/
/*}}}*/
// Message 0x60 (96) | CHANNEL LIST MODIFICATION /*{{{*/
int
_v3_get_0x60(_v3_net_message *msg) {/*{{{*/
    _v3_msg_0x60 *m;
    int ctr;
    void *offset;

    _v3_func_enter("_v3_get_0x60");
    m = malloc(sizeof(_v3_msg_0x60));
    memcpy(m, msg->data, 8);
    _v3_debug(V3_DEBUG_PACKET_PARSE, "packet contains %d channels, allocating %d bytes", m->channel_count, m->channel_count * sizeof(_v3_msg_channel));
    m->channel_list = calloc(m->channel_count, sizeof(_v3_msg_channel));
    for (ctr = 0, offset = msg->data + 8;ctr < m->channel_count; ctr++) {
        offset += _v3_get_msg_channel(offset, &m->channel_list[ctr]);
        _v3_debug(V3_DEBUG_PACKET_PARSE, "got channel: id: %d | name: %s | phonetic: %s | comment: %s",
                m->channel_list[ctr].id,
                m->channel_list[ctr].name,
                m->channel_list[ctr].phonetic,
                m->channel_list[ctr].comment);
    }
    msg->contents = m;

    _v3_func_leave("_v3_get_0x60");
    return true;
}/*}}}*/
int
_v3_destroy_0x60(_v3_net_message *msg) {/*{{{*/
    _v3_msg_0x60 *m;
    int ctr;

    m = msg->contents;
    _v3_func_enter("_v3_destroy_0x60");
    for (ctr = 0; ctr < m->channel_count; ctr++) {
        _v3_debug(V3_DEBUG_PACKET_PARSE, "freeing resources for channel %d: %s", m->channel_list[ctr].id, m->channel_list[ctr].name);
        free(m->channel_list[ctr].name);
        free(m->channel_list[ctr].phonetic);
        free(m->channel_list[ctr].comment);
    }
    free(m->channel_list);

    _v3_func_leave("_v3_destroy_0x60");
    return true;
}/*}}}*/
/*}}}*/
// Message 0x61 (97) | BAN LIST MODIFICATION /*{{{*/
int
_v3_get_0x61(_v3_net_message *msg) {/*{{{*/
    _v3_msg_0x61 *m;

    _v3_func_enter("_v3_get_0x61");
    if (msg->len != sizeof(_v3_msg_0x61)) {
        _v3_debug(V3_DEBUG_PACKET_PARSE, "expected %d bytes, but message is %d bytes", sizeof(_v3_msg_0x61), msg->len);
        _v3_func_leave("_v3_get_0x61");
        return false;
    }
    m = msg->contents = msg->data;
    _v3_debug(V3_DEBUG_PACKET_PARSE, "Ban:");
    _v3_debug(V3_DEBUG_PACKET_PARSE, "subtype.............: %u", m->subtype);
    _v3_debug(V3_DEBUG_PACKET_PARSE, "bitmask_id..........: %u", m->bitmask_id);
    _v3_debug(V3_DEBUG_PACKET_PARSE, "ip_address..........: 0x%08X (%u.%u.%u.%u)",
            m->ip_address,
            (m->ip_address >> 24) & 0xff,
            (m->ip_address >> 16) & 0xff,
            (m->ip_address >> 8) & 0xff,
            m->ip_address & 0xff);
    _v3_debug(V3_DEBUG_PACKET_PARSE, "ban_count...........: %u", m->ban_count);
    _v3_debug(V3_DEBUG_PACKET_PARSE, "ban_id..............: %u", m->ban_id);
    _v3_debug(V3_DEBUG_PACKET_PARSE, "banned_user.........: %s", m->banned_user);
    _v3_debug(V3_DEBUG_PACKET_PARSE, "banned_by...........: %s", m->banned_by);
    _v3_debug(V3_DEBUG_PACKET_PARSE, "ban_reason..........: %s", m->ban_reason);

    _v3_func_leave("_v3_get_0x61");
    return true;
}/*}}}*/
_v3_net_message *
_v3_put_0x61(uint32_t subtype, uint32_t bitmask_id, uint32_t ip_address, char *banned_user, char *ban_reason) {/*{{{*/
    _v3_net_message *m;
    _v3_msg_0x61 *mc;

    _v3_func_enter("_v3_put_0x61");
    m = malloc(sizeof(_v3_net_message));
    memset(m, 0, sizeof(_v3_net_message));
    m->type = 0x61;
    m->len = sizeof(_v3_msg_0x61);

    mc = malloc(m->len);
    memset(mc, 0, m->len);
    mc->type = 0x61;
    mc->subtype = subtype;
    if (subtype == V3_ADMIN_BAN_REMOVE || (subtype == V3_ADMIN_BAN_ADD && bitmask_id < 0x20)) {
        mc->bitmask_id = bitmask_id;
    }
    mc->ip_address = ip_address;
    if (subtype != V3_ADMIN_BAN_LIST) {
        mc->ban_count = 1;
    }
    if (banned_user) {
        strncpy(mc->banned_user, banned_user, sizeof(mc->banned_user) - 1);
    }
    if (ban_reason) {
        strncpy(mc->ban_reason, ban_reason, sizeof(mc->ban_reason) - 1);
    }
    m->contents = mc;
    m->data = (char *)mc;

    _v3_func_leave("_v3_put_0x61");
    return m;
}/*}}}*/
/*}}}*/
// Message 0x62 (98) | USER PAGE /*{{{*/
int
_v3_get_0x62(_v3_net_message *msg) {/*{{{*/
    _v3_msg_0x62 *m;

    _v3_func_enter("_v3_get_0x62");
    if (msg->len != sizeof(_v3_msg_0x62)) {
        _v3_debug(V3_DEBUG_PACKET_PARSE, "expected %d bytes, but message is %d bytes", sizeof(_v3_msg_0x62), msg->len);
        _v3_func_leave("_v3_get_0x62");
        return false;
    }
    m = msg->contents = msg->data;
    _v3_debug(V3_DEBUG_PACKET_PARSE, "User Page:");
    _v3_debug(V3_DEBUG_PACKET_PARSE, "user_id_to..........: %d", m->user_id_to);
    _v3_debug(V3_DEBUG_PACKET_PARSE, "user_id_from........: %d", m->user_id_from);
    _v3_debug(V3_DEBUG_PACKET_PARSE, "error_id............: %d", m->error_id);

    _v3_func_leave("_v3_get_0x62");
    return true;
}/*}}}*/
_v3_net_message *
_v3_put_0x62(uint16_t user_id) {/*{{{*/
    _v3_net_message *m;
    _v3_msg_0x62 *mc;

    _v3_func_enter("_v3_put_0x62");
    m = malloc(sizeof(_v3_net_message));
    memset(m, 0, sizeof(_v3_net_message));
    m->type = 0x62;
    m->len = sizeof(_v3_msg_0x62);

    mc = malloc(m->len);
    memset(mc, 0, m->len);

    mc->type = 0x62;
    mc->user_id_to = user_id;
    mc->user_id_from = v3_luser.id;
    m->contents = mc;
    m->data = (char *)mc;

    _v3_func_leave("_v3_put_0x62");
    return m;
}/*}}}*/
/*}}}*/
// Message 0x63 (99) | ADMIN /*{{{*/
_v3_net_message *
_v3_put_0x63(uint16_t subtype, uint16_t user_id, char *string) {/*{{{*/
    _v3_net_message *m;
    _v3_msg_0x63 *mc;

    _v3_func_enter("_v3_put_0x63");
    // Build our message
    m = malloc(sizeof(_v3_net_message));
    memset(m, 0, sizeof(_v3_net_message));
    m->type = 0x63;
    m->len = sizeof(_v3_msg_0x63);

    // Build our message contents
    mc = malloc(sizeof(_v3_msg_0x63));
    memset(mc, 0, sizeof(_v3_msg_0x63));

    mc->type = 0x63;
    mc->subtype = subtype;
    switch (subtype) {
        case V3_ADMIN_LOGIN:
            _v3_hash_password((uint8_t *)string, mc->t.password_hash);
            break;
        case V3_ADMIN_LOGOUT:
            break;
        case V3_ADMIN_KICK:
        case V3_ADMIN_BAN:
        case V3_ADMIN_CHANNEL_BAN:
            mc->user_id = user_id;
            strncpy(mc->t.reason, string, sizeof(mc->t.reason));
            break;
    }
    m->contents = mc;
    m->data = (char *)mc;

    _v3_func_leave("_v3_put_0x63");
    return m;
}/*}}}*/
/*}}}*/

