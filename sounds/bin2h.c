/*
 * vim: softtabstop=4 shiftwidth=4 cindent foldmethod=marker expandtab
 *
 * $LastChangedDate$
 * $Revision$
 * $LastChangedBy$
 * $URL$
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
 */

#include <stdio.h>
#include <string.h>
#include <errno.h>

#define TYPE_NAME   "uint8_t" /* or "unsigned char" */
#define BUF_SIZE    1 << 8
#define FMT_HEX     "0x%02X"

int
main(int argc, char **argv) {
    if (argc < 4) {
        fprintf(stderr,
            "usage: %s [input] [output[.h]] [variable name]\n"
            "if output file exists, it will be overwritten.\n"
            "stdin and stdout supported; use - for input and/or output.\n",
        argv[0]);
        return 1;
    }

    FILE *in, *out;

    if (!strncmp(argv[1], "-", 1)) {
        in = stdin;
    } else if (!(in = fopen(argv[1], "r"))) {
        fprintf(stderr, "input: %s: error: %s\n", argv[1], strerror(errno));
        return 1;
    } else if (in) {
        fseek(in, 0, SEEK_END);
        if (!ftell(in)) {
            fprintf(stderr, "input: %s: error: %s\n", argv[1], "file is empty");
            return 1;
        }
        rewind(in);
    }
    if (!strncmp(argv[2], "-", 1)) {
        out = stdout;
    } else if (!(out = fopen(argv[2], "w+"))) {
        fprintf(stderr, "output: %s: error: %s\n", argv[2], strerror(errno));
        return 1;
    }

    unsigned char buf[BUF_SIZE];
    memset(&buf, 0, sizeof(buf));
    fprintf(out, "\n%s %s[] = {", TYPE_NAME, argv[3]);
    size_t read, ctr;
    while (!feof(in)) {
        read = fread(&buf, 1, BUF_SIZE, in);
        ctr = 0;
        while (ctr < read) {
            if (!(ctr % (1 << 4))) {
                fputs("\n   ", out);
            } else if (!(ctr % (1 << 3))) {
                fputs(" ", out);
            }
            fprintf(out, " " FMT_HEX ",", buf[ctr++]);
        }
    }
    fputs("\n};\n\n", out);

    if (in != stdin) {
        fclose(in);
    }
    if (out != stdout) {
        fclose(out);
    }

    fprintf(stderr, "%s: conversion done\n", argv[0]);
    return 0;
}
