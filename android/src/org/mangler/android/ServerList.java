/*
 * Copyright 2010-2011 Daniel Sloof <daniel@danslo.org>
 *
 * This file is part of Mangler.
 *
 * $LastChangedDate: 2011-07-13 09:50:43 +0430 (Wed, 13 Jul 2011) $
 * $Revision: 1145 $
 * $LastChangedBy: econnell $
 * $URL: http://svn.mangler.org/mangler/trunk/android/src/org/mangler/android/ServerList.java $
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

import android.app.ListActivity;
import android.app.Notification;
import android.app.NotificationManager;
import android.app.PendingIntent;
import android.app.ProgressDialog;
import android.content.BroadcastReceiver;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.ServiceConnection;
import android.database.Cursor;
import android.media.AudioManager;
import android.os.Bundle;
import android.os.IBinder;
import android.preference.PreferenceManager;
import android.view.ContextMenu;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.view.ContextMenu.ContextMenuInfo;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.ListView;
import android.widget.SimpleCursorAdapter;
import android.widget.Toast;
import android.widget.AdapterView.AdapterContextMenuInfo;

public class ServerList extends ListActivity {

	public static String characterEncoding = "ISO-8859-1";

	static {
		// Load native library.
		System.loadLibrary("ventrilo_interface");
	}
	
	private static final int ACTIVITY_CONNECT = 0;
	private static final int ACTIVITY_CREATE = 1;
	private static final int ACTIVITY_EDIT = 2;

	private static final int ADD_ID = Menu.FIRST;
	private static final int EDIT_ID = Menu.FIRST + 1;
	private static final int CLONE_ID = Menu.FIRST + 2;
    private static final int DELETE_ID = Menu.FIRST + 3;
    private static final int SETTINGS_ID = Menu.FIRST + 4;

    public static final String NOTIFY_ACTION = "org.mangler.android.NotifyAction";
    
    private ProgressDialog connectProgress = null;

	private ManglerDBAdapter dbHelper;
	
	// Notifications
	private NotificationManager notificationManager;
	private static final int ONGOING_NOTIFICATION = 1;
	
    /** Called when the activity is first created. */
    @Override
    public void onCreate(Bundle savedInstanceState) {
    	super.onCreate(savedInstanceState);
        
    	ServerList.characterEncoding = PreferenceManager.getDefaultSharedPreferences(getBaseContext()).getString("charset", "ISO-8859-1");
        setContentView(R.layout.main);

        // Volume controls.
        setVolumeControlStream(AudioManager.STREAM_MUSIC);
    	
    	// Set debug level.
    	VentriloInterface.debuglevel(VentriloDebugLevels.V3_DEBUG_INFO);

        // Volume controls.
        setVolumeControlStream(AudioManager.STREAM_MUSIC);

        // Notification broadcast receiver.
        registerReceiver(notificationReceiver, new IntentFilter(NOTIFY_ACTION));
        
		((Button) findViewById(R.id.AddServerButton)).setOnClickListener(new OnClickListener() {
			public void onClick(View v) {
	        	Intent i = new Intent(ServerList.this, ServerEdit.class);
	       	 	startActivityForResult(i, ACTIVITY_CREATE);
			}
		});

        bindService(new Intent(this, EventService.class), serviceconnection, Context.BIND_AUTO_CREATE);
    	
        dbHelper = new ManglerDBAdapter(this);
        dbHelper.open();
    	fillData();
        registerForContextMenu(getListView());
        
        notificationManager = (NotificationManager)getSystemService(NOTIFICATION_SERVICE);
    }
    
    @Override
    public void onResume() {
    	super.onResume();
		setRequestedOrientation(Integer.parseInt(PreferenceManager.getDefaultSharedPreferences(getBaseContext()).getString("orientation", "4")));
    }

	@Override
	protected void onDestroy() {
		super.onDestroy();
		
		// Unregister notification broadcase receiver.
        unregisterReceiver(notificationReceiver);
        
		unbindService(serviceconnection);
		
		dbHelper.close();
	}

    // Populate listview with entries from database
    private void fillData() {
        Cursor serverCursor = dbHelper.fetchServers();
        startManagingCursor(serverCursor);

        // Display simple cursor adapter
        String[] serverInfo = new String[]{ManglerDBAdapter.KEY_SERVERS_SERVERNAME, ManglerDBAdapter.KEY_SERVERS_USERNAME};
        SimpleCursorAdapter servers = new SimpleCursorAdapter(this, R.layout.server_row, serverCursor, serverInfo, new int[]{R.id.slistservername, R.id.slistusername});
        setListAdapter(servers);
    }

    public boolean onCreateOptionsMenu(Menu menu) {
        menu.add(0, ADD_ID, 0, "Add Server").setIcon(R.drawable.menu_add);
        menu.add(0, SETTINGS_ID, 0, "Settings").setIcon(R.drawable.menu_settings);
        return true;
    }

    public boolean onOptionsItemSelected(MenuItem item) {
        switch (item.getItemId()) {
        case ADD_ID:
        	Intent i = new Intent(this, ServerEdit.class);
       	 	startActivityForResult(i, ACTIVITY_CREATE);
            return true;
    	case SETTINGS_ID:
			Intent intent = new Intent(ServerList.this, Settings.class);
			startActivity(intent);
    		return true;
        }
        return false;
    }

    @Override
	public void onCreateContextMenu(ContextMenu menu, View v, ContextMenuInfo menuInfo) {
		super.onCreateContextMenu(menu, v, menuInfo);
		menu.add(0, EDIT_ID, 0, "Edit Server");
		menu.add(0, CLONE_ID, 1, "Clone Server");
		menu.add(0, DELETE_ID, 2, "Delete Server");
	}

    @Override
	public boolean onContextItemSelected(MenuItem item) {
		AdapterContextMenuInfo info = (AdapterContextMenuInfo) item.getMenuInfo();
		switch(item.getItemId()) {
			case EDIT_ID:
		        Intent i = new Intent(this, ServerEdit.class);
		        i.putExtra(ManglerDBAdapter.KEY_SERVERS_ROWID, info.id);
		        startActivityForResult(i, ACTIVITY_EDIT);
		        fillData();
		        return true;
			case CLONE_ID:
		        dbHelper.cloneServer(info.id);
		        fillData();
		        return true;
			case DELETE_ID:
		        dbHelper.deleteServer(info.id);
		        fillData();
		        return true;
		}
		return super.onContextItemSelected(item);
	}

	@Override
	protected void onListItemClick(ListView l, View v, int position, long id) {
		super.onListItemClick(l, v, position, id);
		
		connectToServer(id);
	}
	
    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent intent) {
        super.onActivityResult(requestCode, resultCode, intent);

        if (requestCode == ACTIVITY_CONNECT) {
        	VentriloInterface.logout();
        }
        
        notificationManager.cancelAll();
        fillData();
    }
    
	private void connectToServer(long id) {
		
		connectProgress = ProgressDialog.show(this, "", "Connecting. Please wait...", true);

		Cursor server = dbHelper.fetchServer(id);
		startManagingCursor(server);
		
		final int serverid = (int)id;
		final String servername = server.getString(server.getColumnIndexOrThrow(ManglerDBAdapter.KEY_SERVERS_SERVERNAME));
		final String hostname = server.getString(server.getColumnIndexOrThrow(ManglerDBAdapter.KEY_SERVERS_HOSTNAME));
		final String port = Integer.toString(server.getInt(server.getColumnIndexOrThrow(ManglerDBAdapter.KEY_SERVERS_PORTNUMBER)));
		final String username = server.getString(server.getColumnIndexOrThrow(ManglerDBAdapter.KEY_SERVERS_USERNAME));
		final String password = server.getString(server.getColumnIndexOrThrow(ManglerDBAdapter.KEY_SERVERS_PASSWORD));
		final String phonetic = server.getString(server.getColumnIndexOrThrow(ManglerDBAdapter.KEY_SERVERS_PHONETIC));
		
		// Get rid of any data from previous connections.
		UserList.clear();
		ChannelList.clear();

		// Add lobby.
		ChannelListEntity entity = new ChannelListEntity();
		entity.id = 0;
		entity.name = "Lobby";
		entity.type = ChannelListEntity.CHANNEL;
		entity.parentid = 0;
		ChannelList.add(entity);

		Thread t = new Thread(new Runnable() {
			public void run() {
				if (VentriloInterface.login(
						hostname + ":" + port, 
						username, 
						password, 
						phonetic)) {
					
					// Start receiving packets.
					startRecvThread();
					
					Intent serverView = new Intent(ServerList.this, ServerView.class)
					.putExtra("serverid", serverid)
					.putExtra("servername", servername);
					connectProgress.dismiss();
					startActivityForResult(serverView, ACTIVITY_CONNECT);
					//Intent notificationIntent = new Intent(ServerList.this, ServerView.class);
					serverView.addFlags(Intent.FLAG_ACTIVITY_CLEAR_TOP);
					Notification notification = new Notification(R.drawable.notification, "Connected to server", System.currentTimeMillis());
					notification.setLatestEventInfo(getApplicationContext(), "Mangler", "Connected to " + servername, PendingIntent.getActivity(ServerList.this, 0, serverView, 0));
					notification.flags = Notification.FLAG_ONGOING_EVENT;
					notificationManager.notify(ONGOING_NOTIFICATION, notification);
					
				} else {
					connectProgress.dismiss();
					VentriloEventData data = new VentriloEventData();
					VentriloInterface.error(data);
					sendBroadcast(new Intent(ServerList.NOTIFY_ACTION)
						.putExtra("notification", "Connection to server failed:\n" + EventService.StringFromBytes(data.error.message)));
				}
			}
		});
		t.start();
	}
	
	private void startRecvThread() {
		Runnable recvRunnable = new Runnable() {
			public void run() {
				while (VentriloInterface.recv())
					;
			}
		};
		(new Thread(recvRunnable)).start();
	}
	
	private BroadcastReceiver notificationReceiver = new BroadcastReceiver() {
		public void onReceive(Context context, Intent intent) {
			Toast.makeText(ServerList.this, intent.getStringExtra("notification"), Toast.LENGTH_LONG).show();
		}
	};
    
	private final ServiceConnection serviceconnection = new ServiceConnection() {
		public void onServiceConnected(ComponentName className, IBinder service) {
			// eventservice = ((EventService.EventBinder)service).getService();
		}
		
		public void onServiceDisconnected(ComponentName arg0) {
			// eventservice = null;
		}
	};
}