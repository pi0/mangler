/*
 * $LastChangedDate: 2011-06-27 23:50:10 +0430 (Mon, 27 Jun 2011) $
 * $Revision: 1139 $
 * $LastChangedBy: econnell $
 * $URL: http://svn.mangler.org/mangler/trunk/libventrilo3/ventrilo_algo.h $
 *
 * All indents are spaces and indent width is 4.
 *
 * vim: softtabstop=4 shiftwidth=4 cindent foldmethod=marker expandtab
 *
 */

/*
Ventrilo encryption/decryption algorithm 0.2a
by Luigi Auriemma
e-mail: aluigi@autistici.org
web:    aluigi.org

INTRODUCTION
============
This algorithm is the method used by the chat program Ventrilo
(http://www.ventrilo.com) for encrypting the communication stream
between clients and servers.


THANX TO
========
Georg Hofstetter (http://www.g3gg0.de)


LICENSE
=======
    Copyright 2004,2005,2006,2007,2008,2009,2010,2011 Luigi Auriemma

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA

    http://www.gnu.org/licenses/gpl-2.0.txt
*/

#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#ifndef __VENT_ALGO_
#define __VENT_ALGO_

#include "ventrilo3.h"

//#define VENTRILO_ALGO_PROTOCOL    // not needed, it's only obfuscated code


int ventrilo_read_keys(ventrilo_key_ctx *client, ventrilo_key_ctx *server, uint8_t *data, int size
#ifdef VENTRILO_ALGO_PROTOCOL
    , int protocol
#endif
) {
    ventrilo_key_ctx    *tmp;
    int     i,
            del;

    _v3_debug(V3_DEBUG_STACK, " -> ventrilo_read_keys()");
    del = -1;
    for(i = 0; (i < size) && (data[i]); i++) {
        if(del >= 0) continue;
        if(data[i] == '|') {        // 2.3 (protocol 2), 3.0 (protocol 3) and so on
            del = i;
        } else if(data[i] == ',') { // 2.1 (protocol 1)
            del = i;
            tmp    = server;
            server = client;
            client = tmp;
        }
    }
    if(del < 0) return(-1);
    size = i;

    server->size = size - (del + 1);
    client->size = del;
    if((client->size > 256) || (server->size > 256)) return(-1);

    client->pos = 0;
    server->pos = 0;
    memcpy(client->key, data, client->size);
    memcpy(server->key, data + del + 1, server->size);
#ifdef VENTRILO_ALGO_PROTOCOL
    client->proto = protocol;
    server->proto = protocol;
#endif
    _v3_debug(V3_DEBUG_STACK, " <- ventrilo_read_keys()");
    return(0);
}



void ventrilo_first_dec(uint8_t *data, int size) {
    static const uint8_t    first[] = "\xAA\x55\x22\xCC\x69\x7C\x38\x91\x88\xF5\xE1";
    int     i;

    for(i = 0; i < size; i++) {
        data[i] -= first[i % 11] + (i % 27);
    }
}



void ventrilo_first_enc(uint8_t *data, int size) {
    static const uint8_t    first[] = "\xAA\x55\x22\xCC\x69\x7C\x38\x91\x88\xF5\xE1";
    int     i;

    for(i = 0; i < size; i++) {
        data[i] += first[i % 11] + (i % 27);
    }
}



void ventrilo_dec(ventrilo_key_ctx *ctx, uint8_t *data, int size) {
    int     i;

#ifdef VENTRILO_ALGO_PROTOCOL
    int     n;
    int64_t t;
    if(ctx->proto >= 3) {
        n = 0;
        for(i = 0; i < size; i++) {
            t = -1240768329;
            t *= i;
            n = (int)(t >> 32);
            n = (n + i) >> 5;
            data[i] += ((((n + (n >> 31)) * 45) - ctx->key[ctx->pos]) - i);
            ctx->pos++;
            if(ctx->pos == ctx->size) ctx->pos = 0;
        }
        return;
    }
#endif
    for(i = 0; i < size; i++) {
        data[i] -= ctx->key[ctx->pos] + (i % 45);
        ctx->pos++;
        if(ctx->pos == ctx->size) ctx->pos = 0;
    }
}



void ventrilo_enc(ventrilo_key_ctx *ctx, uint8_t *data, int size) {
    int     i;

#ifdef VENTRILO_ALGO_PROTOCOL
    int     n;
    int64_t t;
    if(ctx->proto >= 3) {
        n = 0;
        for(i = 0; i < size; i++) {
            t = -1240768329;
            t *= i;
            n = (int)(t >> 32);
            n = (n + i) >> 5;
            data[i] += ((ctx->key[ctx->pos] - ((n + (n >> 31)) * 45)) + i);
            ctx->pos++;
            if(ctx->pos == ctx->size) ctx->pos = 0;
        }
        return;
    }
#endif
    for(i = 0; i < size; i++) {
        data[i] += ctx->key[ctx->pos] + (i % 45);
        ctx->pos++;
        if(ctx->pos == ctx->size) ctx->pos = 0;
    }
}

#endif

