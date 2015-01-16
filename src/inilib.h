/*
 * vim: softtabstop=4 shiftwidth=4 cindent foldmethod=marker expandtab
 *
 * $LastChangedDate: 2011-06-27 23:50:10 +0430 (Mon, 27 Jun 2011) $
 * $Revision: 1139 $
 * $LastChangedBy: econnell $
 * $URL: http://svn.mangler.org/mangler/trunk/src/inilib.h $
 *
 * Copyright 2010-2011 Bob Shaffer II
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

#ifndef INILIB_H_INCLUDED
#define INILIB_H_INCLUDED

#include <iostream>
#include <map>
#include <vector>
#include <string>

#include <pthread.h>

#define ADD_GLIB_SUPPORT 1

#if ADD_GLIB_SUPPORT
#include <gtkmm.h>
#endif

using std::string;
using std::vector;
using std::map;
using std::istream;
using std::ostream;

class iniCaselessCmp {
    public:
    bool operator()(const string &left, const string &right) const;
};

class iniVariant {
    public:
    iniVariant();
    iniVariant(const string &s);
    iniVariant(const char *s);
    iniVariant(int n);
    iniVariant(unsigned n);
    iniVariant(long n);
    iniVariant(unsigned long n);
    iniVariant(long long n);
    iniVariant(double d);
    iniVariant(bool b);
#if ADD_GLIB_SUPPORT
    iniVariant(const Glib::ustring &s);
    Glib::ustring toUString() const;
#endif
    operator string &();
    operator const string &();
    string toString() const;
    string toUpper() const;
    string toLower() const;
    const char *toCString() const;
    int toInt() const;
    unsigned toUInt() const;
    long toLong() const;
    unsigned long toULong() const;
    long long toLLong() const;
    double toDouble() const;
    bool toBool() const;
    string::size_type length() const;
    bool operator==(const iniVariant &v) const;
    static const iniVariant &null() { return mNULLvariant; } 
    private:
    static iniVariant mNULLvariant;
    string mValue;
};

class iniValue : public vector<iniVariant> {
    public:
    iniValue() {}
    iniValue(const iniVariant &v);
    size_type count() const;
    void append(const iniVariant &v);
    operator iniVariant &();
    operator const iniVariant &();
    iniVariant value() const;
    bool operator==(const iniVariant &v) const;
    iniValue &operator=(const iniVariant &v);
    iniValue &operator+=(const iniVariant &v);
    string toString() const;
    string toUpper() const;
    string toLower() const;
    const char *toCString() const;
#if ADD_GLIB_SUPPORT
    Glib::ustring toUString() const;
#endif
    int toInt() const;
    unsigned toUInt() const;
    long toLong() const;
    unsigned long toULong() const;
    long long toLLong() const;
    double toDouble() const;
    bool toBool() const;
    string::size_type length() const;
};

class iniSection : public map<string, iniValue, iniCaselessCmp> {
    friend class iniFile;
    public:
    iniSection() {}
    bool contains(const string &s) const;
    iniValue::size_type count(const string &s) const;
    protected:
    ostream &save(ostream &out, bool quotes) const;
    static vector<string> parseLine(string s, bool quotes);
    static ostream &saveLine(ostream &out, const string &keyName, const string &value, bool quotes);
    static string quoteString(const string &s);
    static void trimString(string &s);
    static void unquoteString(string &s);
};

class iniFile : public map<string, iniSection, iniCaselessCmp> {
    public:
    iniFile();
    iniFile(const string &filename, bool rdonly = false, bool quotes = true);
    void setFilename(const string &filename);
    string getFilename() const;
    istream &load(istream &in);
    ostream &save(ostream &out) const;
    void reload();
    void save() const;
    ~iniFile();
    bool contains(const string &s) const;
    protected:
    static void cleanLine(string &s);
    static void removeBrackets(string &s);
    private:
    string mFilename;
    pthread_mutex_t mymutex;
    bool rdonly;
    bool quotes;
};

istream &operator>>(istream &in, iniFile &f);
ostream &operator<<(ostream &out, iniFile &f);

#endif

