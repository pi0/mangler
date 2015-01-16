/*
 * Copyright 2010 Daniel Sloof <daniel@danslo.org>
 *
 * This file is part of Mangler.
 *
 * $LastChangedDate: 2010-07-28 23:49:19 +0430 (Wed, 28 Jul 2010) $
 * $Revision: 1053 $
 * $LastChangedBy: ekilfoil $
 * $URL: http://svn.mangler.org/mangler/trunk/android/src/org/mangler/android/ManglerDBAdapter.java $
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

package org.mangler.android;

import android.content.ContentValues;
import android.content.Context;
import android.database.Cursor;
import android.database.SQLException;
import android.database.sqlite.SQLiteDatabase;
import android.database.sqlite.SQLiteOpenHelper;
import android.util.Log;

public class ManglerDBAdapter {

	public static final String KEY_SERVERS_ROWID = "_id";
    public static final String KEY_SERVERS_SERVERNAME = "servername";
    public static final String KEY_SERVERS_HOSTNAME = "hostname";
    public static final String KEY_SERVERS_PORTNUMBER = "portnumber";
    public static final String KEY_SERVERS_PASSWORD = "password";
    public static final String KEY_SERVERS_USERNAME = "username";
    public static final String KEY_SERVERS_PHONETIC = "phonetic";
    
    public static final String KEY_VOLUME_SERVERID = "_server_id";
    public static final String KEY_VOLUME_USERNAME = "username";
    public static final String KEY_VOLUME_LEVEL = "level";

    public static final String KEY_PASSWORD_SERVERID = "_server_id";
    public static final String KEY_PASSWORD_CHANNEL = "channel";
    public static final String KEY_PASSWORD_PASSWORD = "password";

    private static final String TAG = "ManglerDBAdapter";
    
    private DatabaseHelper dbHelper;
    private SQLiteDatabase db;
    
    // SQL string for creating database
    private static final String DATABASE_SERVERS_CREATE = "create table servers (_id integer primary key autoincrement, servername text not null, hostname text not null, portnumber integer not null, password text not null, username text not null, phonetic text not null);";
    private static final String DATABASE_VOLUME_CREATE = "create table volume (_server_id integer, username text not null, level integer not null,primary key (_server_id,username));";
    private static final String DATABASE_PASSWORD_CREATE = "create table password (_server_id integer, channel int not null, password text not null,primary key (_server_id,channel));";

    private static final String DATABASE_NAME = "manglerdata";
    private static final String DATABASE_SERVER_TABLE = "servers";
    private static final String DATABASE_VOLUME_TABLE = "volume";
    private static final String DATABASE_PASSWORD_TABLE = "password";

    private static final int DATABASE_VERSION = 4;

    private final Context context;

    private static class DatabaseHelper extends SQLiteOpenHelper {

        DatabaseHelper(Context context) {
            super(context, DATABASE_NAME, null, DATABASE_VERSION);
        }

        @Override
        public void onCreate(SQLiteDatabase db) {
        	try {
        		db.execSQL(DATABASE_SERVERS_CREATE);
        		db.execSQL(DATABASE_VOLUME_CREATE);
        		db.execSQL(DATABASE_PASSWORD_CREATE);
        	} catch (Exception e) {
        		e.printStackTrace();
        	}
        }

        @Override
        public void onUpgrade(SQLiteDatabase db, int oldVersion, int newVersion) {
            Log.w(TAG, "Upgrading database from version " + oldVersion + " to "
                    + newVersion + ", which will destroy all old data");
            if (oldVersion < 2) {
            	db.execSQL("DROP TABLE IF EXISTS servers");
            	onCreate(db);
            }
            if (oldVersion < 3) {
            	db.execSQL(DATABASE_VOLUME_CREATE);
        		db.execSQL(DATABASE_PASSWORD_CREATE);
            }
            if (oldVersion < 4) {
            	db.execSQL("UPDATE " + DATABASE_VOLUME_TABLE +
            			" SET " + KEY_VOLUME_LEVEL + "=79 " +
            			" WHERE " + KEY_VOLUME_LEVEL + "=74;");
            }
        }
    }

    /**
     * Constructor - takes the context to allow the database to be
     * opened/created
     * 
     * @param context the Context within which to work
     */
    public ManglerDBAdapter(Context context) {
        this.context = context;
    }

    /**
     * Open the mangler database. If it cannot be opened, try to create a new
     * instance of the database. If it cannot be created, throw an exception to
     * signal the failure
     * 
     * @return this (self reference, allowing this to be chained in an
     *         initialization call)
     * @throws SQLException if the database could be neither opened or created
     */
    public ManglerDBAdapter open() throws SQLException {
        dbHelper = new DatabaseHelper(context);
        db = dbHelper.getWritableDatabase();
        return this;
    }
    
    public void close() {
        dbHelper.close();
    }
    
    /**
     * Create a new server. If the server is successfully created return the new rowId for that server, otherwise return
     * a -1 to indicate failure.
     * 
     * @param servername
     * @param hostname
     * @param portnumber
     * @param password
     * @param username
     * @param phonetic
     * @return rowId or -1 if failed
     */
    public long createServer(String servername, String hostname, int portnumber, String password, String username, String phonetic) {
        ContentValues initialValues = new ContentValues();
        initialValues.put(KEY_SERVERS_SERVERNAME, servername);
        initialValues.put(KEY_SERVERS_HOSTNAME, hostname);
        initialValues.put(KEY_SERVERS_PORTNUMBER, portnumber);
        initialValues.put(KEY_SERVERS_PASSWORD, password);
        initialValues.put(KEY_SERVERS_USERNAME, username);
        initialValues.put(KEY_SERVERS_PHONETIC, phonetic);

        return db.insert(DATABASE_SERVER_TABLE, null, initialValues);
    }
    
    /**
     * Clone a server. If the server is successfully created return the new rowId for that server, otherwise return
     * a -1 to indicate failure.
     * 
     * @param rowId of server to copy
     * @return rowId of copied server or -1 if failed
     */
    public long cloneServer(long rowId) {
    	Cursor cursor = fetchServer(rowId);
        ContentValues initialValues = new ContentValues();
        initialValues.put(KEY_SERVERS_SERVERNAME, cursor.getString(cursor.getColumnIndexOrThrow(ManglerDBAdapter.KEY_SERVERS_SERVERNAME)));
        initialValues.put(KEY_SERVERS_HOSTNAME, cursor.getString(cursor.getColumnIndexOrThrow(ManglerDBAdapter.KEY_SERVERS_HOSTNAME)));
        initialValues.put(KEY_SERVERS_PORTNUMBER, cursor.getInt(cursor.getColumnIndexOrThrow(ManglerDBAdapter.KEY_SERVERS_PORTNUMBER)));
        initialValues.put(KEY_SERVERS_PASSWORD, cursor.getString(cursor.getColumnIndexOrThrow(ManglerDBAdapter.KEY_SERVERS_PASSWORD)));
        initialValues.put(KEY_SERVERS_USERNAME, cursor.getString(cursor.getColumnIndexOrThrow(ManglerDBAdapter.KEY_SERVERS_USERNAME)));
        initialValues.put(KEY_SERVERS_PHONETIC, cursor.getString(cursor.getColumnIndexOrThrow(ManglerDBAdapter.KEY_SERVERS_PHONETIC)));

        return db.insert(DATABASE_SERVER_TABLE, null, initialValues);
    }
    
    /**
     * Delete the server with the given rowId
     * 
     * @param rowId id of server to delete
     * @return true if deleted, false otherwise
     */
    public boolean deleteServer(long rowId) {
    	db.delete(DATABASE_VOLUME_TABLE, KEY_VOLUME_SERVERID + "=" + rowId, null);
    	db.delete(DATABASE_PASSWORD_TABLE, KEY_PASSWORD_SERVERID + "=" + rowId, null);
        return db.delete(DATABASE_SERVER_TABLE, KEY_SERVERS_ROWID + "=" + rowId, null) > 0;
    }
    
    /**
     * Return a Cursor over the list of all servers in the database
     * 
     * @return Cursor over all servers
     */
    public Cursor fetchServers() {

        return db.query(DATABASE_SERVER_TABLE, new String[] {KEY_SERVERS_ROWID, KEY_SERVERS_SERVERNAME, 
        		KEY_SERVERS_HOSTNAME, KEY_SERVERS_PORTNUMBER, KEY_SERVERS_PASSWORD, KEY_SERVERS_USERNAME, KEY_SERVERS_PHONETIC}, null, null, null, null, null);
    }
    
    /**
     * Return a Cursor positioned at the server that matches the given rowId
     * 
     * @param rowId id of server to retrieve
     * @return Cursor positioned to matching server, if found
     * @throws SQLException if server could not be found/retrieved
     */
    public Cursor fetchServer(long rowId) throws SQLException {

        Cursor cursor =
                db.query(true, DATABASE_SERVER_TABLE, new String[] {KEY_SERVERS_ROWID,
                        KEY_SERVERS_SERVERNAME, KEY_SERVERS_HOSTNAME, KEY_SERVERS_PORTNUMBER, KEY_SERVERS_PASSWORD, KEY_SERVERS_USERNAME, KEY_SERVERS_PHONETIC}, 
                        KEY_SERVERS_ROWID + "=" + rowId, null, null, null, null, null);
        if (cursor != null) {
            cursor.moveToFirst();
        }
        return cursor;
    }

    /**
     * Update the server using the details provided. The server to be updated is
     * specified using the rowId, and it is altered to use the values passed in
     * 
     * @param rowId id of server to update
     * @param servername
     * @param hostname
     * @param portnumber
     * @param password
     * @param username
     * @param phonetic
     * @return true if the server was successfully updated, false otherwise
     */
    public boolean updateServer(long rowId, String servername, String hostname, int portnumber, String password, String username, String phonetic) {
        ContentValues args = new ContentValues();
        args.put(KEY_SERVERS_SERVERNAME, servername);
        args.put(KEY_SERVERS_HOSTNAME, hostname);
        args.put(KEY_SERVERS_PORTNUMBER, portnumber);
        args.put(KEY_SERVERS_PASSWORD, password);
        args.put(KEY_SERVERS_USERNAME, username);
        args.put(KEY_SERVERS_PHONETIC, phonetic);

        return db.update(DATABASE_SERVER_TABLE, args, KEY_SERVERS_ROWID + "=" + rowId, null) > 0;
    }
    
    public boolean setVolume(int serverid, String username, int level) {
        ContentValues args = new ContentValues();
        args.put(KEY_VOLUME_SERVERID, serverid);
        args.put(KEY_VOLUME_USERNAME, username);
        args.put(KEY_VOLUME_LEVEL, level);
        return db.replace(DATABASE_VOLUME_TABLE, KEY_VOLUME_SERVERID, args) >= 0 ? true : false;
    }
    
    public int getVolume(int serverid, String username) {
    	int level = 79;
    	
    	Log.d("mangler",KEY_VOLUME_SERVERID + "=" + serverid + " and " + KEY_VOLUME_USERNAME + "='" + username + "'"); 
        Cursor cursor =
        	db.query(true,
        		DATABASE_VOLUME_TABLE,
        		new String[] {
        				KEY_VOLUME_LEVEL
        			},
        		KEY_VOLUME_SERVERID + "=" + serverid + " and " + KEY_VOLUME_USERNAME + "=?",
        		new String [] {
        			username
        		}
        		, null, null, null, null);
		if (cursor != null) {
			try {
				cursor.moveToFirst();
				level = cursor.getInt(cursor.getColumnIndexOrThrow(ManglerDBAdapter.KEY_VOLUME_LEVEL));
				cursor.close();
			} catch (Exception e) {
				cursor.close();
				return level;
			}
		}
		
		return level;
	}
    
    public boolean setPassword(int serverid, int channelid, String password) {
        ContentValues args = new ContentValues();
        args.put(KEY_PASSWORD_SERVERID, serverid);
        args.put(KEY_PASSWORD_CHANNEL, channelid);
        args.put(KEY_PASSWORD_PASSWORD, password);

        return db.replace(DATABASE_PASSWORD_TABLE, KEY_PASSWORD_SERVERID, args) >= 0 ? true : false;
    }
    
    public String getPassword(int serverid, int channelid) {
    	String password = "";
    	
        Cursor cursor =
        	db.query(true,
        		DATABASE_PASSWORD_TABLE,
        		new String[] {
        				KEY_PASSWORD_PASSWORD
        			},
        			KEY_PASSWORD_SERVERID + "=" + serverid + " and " + KEY_PASSWORD_CHANNEL + "=" + channelid,
        			null, null, null, null, null);
		if (cursor != null) {
			try {
				cursor.moveToFirst();
				password = cursor.getString(cursor.getColumnIndexOrThrow(ManglerDBAdapter.KEY_PASSWORD_PASSWORD));
				cursor.close();
			} catch (Exception e) {
				cursor.close();
				Log.d("mangler", "no password found in database");
				return "";
			}
		}
		Log.d("mangler", "password '" + password + "' found in database");
		return password;
    }
}