/*
 * vim: softtabstop=4 shiftwidth=4 cindent foldmethod=marker expandtab
 *
 * $LastChangedDate: 2011-06-27 23:50:10 +0430 (Mon, 27 Jun 2011) $
 * $Revision: 1139 $
 * $LastChangedBy: econnell $
 * $URL: http://svn.mangler.org/mangler/trunk/src/manglercharset.cpp $
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
#include "manglercharset.h"

Glib::ustring serverCharset;
const char *charsetslist[] = {
    "System default",
    "ISO-8859-15 (Western Europe)",
    "ISO-8859-2 (Central Europe)",
    "ISO-8859-5 (Ukrainian)",
    "ISO-8859-7 (Greek)",
    "ISO-8859-8 (Hebrew)",
    "ISO-8859-9 (Turkish)",
    "ISO-2022-JP (Japanese)",
    "SJIS (Japanese)",
    "CP949 (Korean)",
    "CP1251 (Cyrillic)",
    "KOI8-R (Cyrillic)",
    "CP1256 (Arabic)",
    "CP1257 (Baltic)",
    "GB18030 (Chinese)",
    "TIS-620 (Thai)",
    NULL
};

static Glib::ustring iso_8859_1_to_utf8 (const char *input) {/*{{{*/
    Glib::ustring output;
    unsigned char *text = (unsigned char *)input;
    int len = strlen(input);
    static const gunichar lowtable[] = /* 74 byte table for 80-a4 */
    {
    /* compressed utf-8 table */
        0x20ac, /* 80 Euro. CP1252 from here on... */
        0x81,   /* 81 NA */
        0x201a, /* 82 */
        0x192,  /* 83 */
        0x201e, /* 84 */
        0x2026, /* 85 */
        0x2020, /* 86 */
        0x2021, /* 87 */
        0x2c6,  /* 88 */
        0x2030, /* 89 */
        0x160,  /* 8a */
        0x2039, /* 8b */
        0x152,  /* 8c */
        0x8d,   /* 8d NA */
        0x17d,  /* 8e */
        0x8f,   /* 8f NA */
        0x90,   /* 90 NA */
        0x2018, /* 91 */
        0x2019, /* 92 */
        0x201c, /* 93 */
        0x201d, /* 94 */
        0x2022, /* 95 */
        0x2013, /* 96 */
        0x2014, /* 97 */
        0x2dc,  /* 98 */
        0x2122, /* 99 */
        0x161,  /* 9a */
        0x203a, /* 9b */
        0x153,  /* 9c */
        0x9d,   /* 9d NA */
        0x17e,  /* 9e */
        0x178,  /* 9f */
        0xa0,   /* a0 */
        0xa1,   /* a1 */
        0xa2,   /* a2 */
        0xa3,   /* a3 */
        0x20ac  /* a4 ISO-8859-15 Euro. */
    };

    while (len) {
        if (G_UNLIKELY(*text >= 0x80) && G_UNLIKELY(*text <= 0xa4)) {
            int idx = *text - 0x80;
            output += lowtable[idx];
        } else {
            output += (gunichar)*text;    /* ascii/iso88591 maps directly */
        }

        text++;
        len--;
    }
    return output;
}/*}}}*/
void set_charset(Glib::ustring charset) {/*{{{*/
    charset = charset.uppercase();

    if (charset.find(' ') != Glib::ustring::npos)
        charset = charset.erase(charset.find(' '));

    if (charset.empty() || !charset.compare("SYSTEM")) {
        charset.clear();
    } else {
        try {
            Glib::IConv test("UTF-8", charset);
        } catch (...) {
            fprintf(stderr, "Charset '%s' isn't supported by your system - using system locale\n", charset.c_str());
            charset.clear();
        }
    }

    serverCharset = charset;
}/*}}}*/
std::string ustring_to_c(Glib::ustring input) {/*{{{*/
    std::string to_charset, converted;

    // check if input is already 7-bit
    if (input.is_ascii())
        return input;

    // try encoding using the selected charset, unless its some utf-8
    if (!serverCharset.empty() && serverCharset.find("UTF-8") == Glib::ustring::npos) {
        try {
            return Glib::convert(input, serverCharset, "UTF-8");
        } catch (...) {}
    }

    // try encoding using the locale charset, unless its some utf-8
    if (Glib::get_charset(to_charset) == true)
        to_charset = "ISO-8859-1";

    try {
        converted = Glib::convert_with_fallback(input, to_charset, "UTF-8", "?");
    } catch (...) {
        converted = input;
    }
    return converted;
}/*}}}*/
Glib::ustring c_to_ustring(const char *input) {/*{{{*/
    Glib::ustring converted, input_u;
    // if input is NULL, return an empty string
    if (! input) return "";

    input_u = input;

    // check if input is already valid UTF-8
    if (input_u.validate())
        return input_u;

    // try to convert using the chosen charset
    if (!serverCharset.empty()) {
        try {
            return Glib::convert(input, "UTF-8", serverCharset);
        } catch (...) {}
    }

    // try to convert using the current locale
    try {
        converted = Glib::locale_to_utf8(input);
    } catch (...) {
        // locale conversion failed
        converted = iso_8859_1_to_utf8(input);
    }
    return converted;
}/*}}}*/
