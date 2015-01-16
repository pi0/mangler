/*
 * Copyright 2010-2011 Daniel Sloof <daniel@danslo.org>
 *
 * This file is part of Mangler.
 *
 * $LastChangedDate: 2012-05-26 06:05:03 +0430 (Sat, 26 May 2012) $
 * $Revision: 1171 $
 * $LastChangedBy: Haxar $
 * $URL: http://svn.mangler.org/mangler/trunk/android/src/org/mangler/android/ServerView.java $
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

import java.util.HashMap;

import android.app.AlertDialog;
import android.app.TabActivity;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.IntentFilter;
import android.media.AudioManager;
import android.os.Build;
import android.os.Bundle;
import android.os.PowerManager;
import android.os.Vibrator;
import android.preference.PreferenceManager;
import android.text.InputType;
import android.text.util.Linkify;
import android.util.Log;
import android.view.ContextMenu;
import android.view.KeyEvent;
import android.view.LayoutInflater;
import android.view.Menu;
import android.view.MenuItem;
import android.view.MotionEvent;
import android.view.View;
import android.view.Window;
import android.view.WindowManager;
import android.view.ContextMenu.ContextMenuInfo;
import android.view.View.OnKeyListener;
import android.view.View.OnTouchListener;
import android.view.inputmethod.InputMethodManager;
import android.widget.AdapterView;
import android.widget.Button;
import android.widget.EditText;
import android.widget.ImageView;
import android.widget.ListView;
import android.widget.ScrollView;
import android.widget.SeekBar;
import android.widget.SimpleAdapter;
import android.widget.TabHost;
import android.widget.TabWidget;
import android.widget.TextView;
import android.widget.Toast;
import android.widget.AdapterView.AdapterContextMenuInfo;
import android.widget.AdapterView.OnItemClickListener;
import android.widget.SeekBar.OnSeekBarChangeListener;

public class ServerView extends TabActivity {
	
	static {
		// Let's try loading the library here too... maybe this will fix the
		// unsatisfied link problems? Maybe the phone is killing the Main
		// Activity and unloading the library?
		System.loadLibrary("ventrilo_interface");
	}
	
	// Server ID that we're connected to
	private static int serverid = -1;
	public String servername = "Server View";
	
	// Database connection
	private ManglerDBAdapter dbHelper;

	// Actions.
	public static final String EVENT_ACTION				= "org.mangler.android.EventAction";
	public static final String NOTIFY_ACTION			= "org.mangler.android.NotifyAction";

	// Menu options.
	private final int OPTION_JOIN_CHAT  = 1;
	private final int OPTION_HIDE_TABS  = 2;
	private final int OPTION_DISCONNECT = 3;
	private final int OPTION_SETTINGS = 4;
	private final int OPTION_ADMIN_LOGIN = 5;
	
	// User context menu options (does java not have enums?)
	private final int CM_OPTION_VOLUME = 1;
	private final int CM_OPTION_COMMENT = 2;
	private final int CM_OPTION_SEND_PAGE = 3;
	private final int CM_OPTION_KICK= 4;
	private final int CM_OPTION_BAN = 6;
	private final int CM_OPTION_MUTE = 7;
	private final int CM_OPTION_GLOBAL_MUTE= 8;
	private final int CM_OPTION_MOVE_USER = 9;
	private final int CM_OPTION_URL = 10;
	
	// Channel context menu options
	private final int CM_OPTION_PHANTOM = 1;
	
	// List adapters.
	private SimpleAdapter channelAdapter;
	private SimpleAdapter userAdapter;
	
	EventHandler eventHandler = null;
	
	// Text to Speech
	TTSWrapper ttsWrapper = null;
	
	// Text to Speech Message types
	public final String TTS_CHANNEL = "Channel";
	public final String TTS_LOGIN   = "Login";
	public final String TTS_PAGE    = "Page";

	// State variables.
	private boolean userInChat = false;
	private boolean tabsHidden = false;
	private boolean isAdmin = false;
	public int ping = 0;
	// WakeLock
	private PowerManager.WakeLock wl;
	
	
	
    @Override
    public void onCreate(Bundle savedInstanceState) {
    	super.onCreate(savedInstanceState);
		boolean fullscreen = PreferenceManager.getDefaultSharedPreferences(getBaseContext()).getBoolean("fullscreen", false);
		ServerList.characterEncoding = PreferenceManager.getDefaultSharedPreferences(getBaseContext()).getString("charset", "ISO-8859-1");
		if (fullscreen) {
			getWindow().setFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN, WindowManager.LayoutParams.FLAG_FULLSCREEN);
			requestWindowFeature(Window.FEATURE_NO_TITLE);
		}
		setContentView(R.layout.server_view);

		// Chat TextView
		TextView chatMessages = (TextView) findViewById(R.id.messages);
		
		// Get the server id and name that we're connected to
		Bundle extras = getIntent().getExtras();
    	if (extras != null) {
    		serverid = getIntent().getIntExtra("serverid", 0);
    		servername = getIntent().getStringExtra("servername");
    	}
    	
		// Restore state.
		if (savedInstanceState != null) {
			ping = savedInstanceState.getInt("ping");
			serverid = savedInstanceState.getInt("serverid");
			servername = savedInstanceState.getString("servername");
			this.setTitle();
			userInChat = savedInstanceState.getBoolean("chatopen");
			chatMessages.setText(savedInstanceState.getString("chatmessages"));
			((EditText) findViewById(R.id.message)).setEnabled(userInChat);
		}
        
		// Set up database adapter
        dbHelper = new ManglerDBAdapter(this);
        dbHelper.open();

        
        // Volume controls.
        setVolumeControlStream(AudioManager.STREAM_MUSIC);
        
        // Text to speech init     
        if (ttsWrapper == null) {
        	ttsWrapper = TTSWrapper.getInstance(this);
        }
        
        // Add tabs.
        TabHost tabhost = getTabHost();
        tabhost.addTab(tabhost.newTabSpec("channel").setContent(R.id.channelView).setIndicator("Channels"));
        tabhost.addTab(tabhost.newTabSpec("user").setContent(R.id.userView).setIndicator("Users"));
    	tabhost.addTab(tabhost.newTabSpec("chat").setContent(R.id.chatView).setIndicator("Chat"));
        //tabhost.addTab(tabhost.newTabSpec("talk").setContent(R.id.talkView).setIndicator("Debug"));

        // Create adapters.
	    channelAdapter 	= new SimpleAdapter(this, ChannelList.data, R.layout.channel_row, new String[] { "indent", "xmitStatus", "name" }, new int[] { R.id.indent, R.id.crowimg, R.id.crowtext } );
	    userAdapter 	= new SimpleAdapter(this, UserList.data, R.layout.user_row, new String[] { "userstatus", "username", "comment" }, new int[] { R.id.urowimg, R.id.urowtext, R.id.urowid } );

	    // Set adapters.
	    ((ListView)findViewById(R.id.channelList)).setAdapter(channelAdapter);
	    ((ListView)findViewById(R.id.userList)).setAdapter(userAdapter);

	    // List item clicks.
	    ((ListView)findViewById(R.id.channelList)).setOnItemClickListener(onListClick);

	    // Register receivers.
        registerReceiver(eventReceiver, new IntentFilter(EVENT_ACTION));
        registerReceiver(notifyReceiver, new IntentFilter(NOTIFY_ACTION));

        // Control listeners.
	    ((EditText)findViewById(R.id.message)).setOnKeyListener(onChatMessageEnter);
	    ((Button)findViewById(R.id.talkButton)).setOnTouchListener(onTalkPress);
	    

		// Restore state.
		if (savedInstanceState != null) {
			ping = savedInstanceState.getInt("ping");
			serverid = savedInstanceState.getInt("serverid");
			servername = savedInstanceState.getString("servername");
			this.setTitle();
			userInChat = savedInstanceState.getBoolean("chatopen");
			chatMessages.setText(savedInstanceState.getString("chatmessages"));
			((EditText) findViewById(R.id.message)).setEnabled(userInChat);
		}
		
	    eventHandler = new EventHandler(this);
	
	    ((EditText)findViewById(R.id.message)).setVisibility(userInChat ? TextView.VISIBLE : TextView.GONE);
	    
	    // Get a wakelock to prevent sleeping and register an onchange preference callback
	    PowerManager pm = (PowerManager) getSystemService(Context.POWER_SERVICE);
	    wl = pm.newWakeLock(PowerManager.SCREEN_DIM_WAKE_LOCK|PowerManager.ON_AFTER_RELEASE, "Mangler");
		boolean prevent_sleep = PreferenceManager.getDefaultSharedPreferences(getBaseContext()).getBoolean("prevent_sleep", false);
		if (prevent_sleep) {
			if (!wl.isHeld()) {
				wl.acquire();
			}
		}
		
		registerForContextMenu(((ListView)findViewById(R.id.channelList)));
		registerForContextMenu(((ListView)findViewById(R.id.userList)));

		isAdmin = VentriloInterface.getpermission("serveradmin");
		
		int level = PreferenceManager.getDefaultSharedPreferences(getBaseContext()).getInt("xmitvolume", 79);
		VentriloInterface.setxmitvolume(level);
		Log.e("mangler", "login setting xmit level to " + level);
		
		eventHandler.process();
		notifyAdaptersDataSetChanged();
	}
    
    @Override
    protected void onResume() {
		boolean prevent_sleep = PreferenceManager.getDefaultSharedPreferences(getBaseContext()).getBoolean("prevent_sleep", false);
		ServerList.characterEncoding = PreferenceManager.getDefaultSharedPreferences(getBaseContext()).getString("charset", "ISO-8859-1");
		setRequestedOrientation(Integer.parseInt(PreferenceManager.getDefaultSharedPreferences(getBaseContext()).getString("orientation", "4")));
		super.onResume();
		if (prevent_sleep) {
			if (!wl.isHeld()) {
				wl.acquire();
			}
		} else {
			if (wl.isHeld()) {
				wl.release();
			}
		}
    }

    @Override
    protected void onSaveInstanceState(Bundle outState) {
    	outState.putInt("serverid", serverid);
    	outState.putInt("ping", ping);
    	outState.putString("servername", servername);
    	outState.putString("chatmessages", ((TextView)findViewById(R.id.messages)).getText().toString());
    	outState.putBoolean("chatopen", userInChat);
    	super.onSaveInstanceState(outState);
    }
    
	@Override
	public boolean onKeyDown(int keyCode, KeyEvent event) {
		boolean ptt_toggle = PreferenceManager.getDefaultSharedPreferences(getBaseContext()).getBoolean("ptt_toggle", false);
		
		if (PreferenceManager.getDefaultSharedPreferences(getBaseContext()).getBoolean("custom_ptt", false)) {
			int pttCode = PreferenceManager.getDefaultSharedPreferences(getBaseContext()).getInt("ptt_key", KeyEvent.KEYCODE_CAMERA);
			
			if (keyCode == pttCode) {
				if (!Recorder.recording()) {
					startPtt();
				} else if (ptt_toggle || keyCode == KeyEvent.KEYCODE_HEADSETHOOK) {
					stopPtt();
				}
				return true;
			}
			if (keyCode == KeyEvent.KEYCODE_CAMERA && pttCode == KeyEvent.KEYCODE_FOCUS) {
				return true;
			}
		}
		return super.onKeyDown(keyCode, event);
	}

	@Override
	public boolean onKeyUp(int keyCode, KeyEvent event) {
		boolean ptt_toggle = PreferenceManager.getDefaultSharedPreferences(getBaseContext()).getBoolean("ptt_toggle", false);
		if (PreferenceManager.getDefaultSharedPreferences(getBaseContext()).getBoolean("custom_ptt", false)) {
			if (keyCode == PreferenceManager.getDefaultSharedPreferences(getBaseContext()).getInt("ptt_key", KeyEvent.KEYCODE_CAMERA)) {
				if (!ptt_toggle && keyCode != KeyEvent.KEYCODE_HEADSETHOOK) {
					stopPtt();
					return true;
				}
			}
		}
		return super.onKeyUp(keyCode, event);
	}

    @Override
    public void onDestroy() {
    	super.onDestroy();
    	
    	if (Recorder.recording()) {			
    		stopPtt();
    	}
    	
    	// release a wakelock if we have one
    	if (wl.isHeld()) {
    		wl.release();
    	}
    	    	
    	if (ttsWrapper != null) {
    		ttsWrapper.shutdown();
    	}
    	
    	dbHelper.close();
    	
    	// Unregister receivers.
        unregisterReceiver(eventReceiver);
        unregisterReceiver(notifyReceiver);
    }
    
	// I probably should name this function something else, but not sure what to
	// name it
    public void setTitle() {
		setTitle((isAdmin ? "[A] " : "") + servername + " - Ping: " + ((ping < 65535 && ping > 0) ? ping + "ms" : "checking..."));
    }

    public boolean onCreateOptionsMenu(Menu menu) {
    	 // Create our menu buttons.
    	if (tabsHidden) {
    		menu.add(0, OPTION_HIDE_TABS, 0, "Show Tabs").setIcon(R.drawable.menu_show_tabs);
        	final TabWidget tabWidget = (TabWidget)findViewById(android.R.id.tabs);
			tabWidget.setEnabled(false);
			tabWidget.setVisibility(TextView.GONE);
    	} else {
    		menu.add(0, OPTION_HIDE_TABS, 0, "Hide Tabs").setIcon(R.drawable.menu_hide_tabs);
    	}
        menu.add(0, OPTION_SETTINGS, 0, "Settings").setIcon(R.drawable.menu_settings);
        if (!userInChat) {
        	menu.add(0, OPTION_JOIN_CHAT, 0, "Join Chat").setIcon(R.drawable.menu_join_chat);
        } else {
        	menu.add(0, OPTION_JOIN_CHAT, 0, "Leave Chat").setIcon(R.drawable.menu_leave_chat);
        }
    	if (! isAdmin) {
    		menu.add(0, OPTION_ADMIN_LOGIN, 0, "Admin Login").setIcon(R.drawable.menu_admin_login);
    	}
        menu.add(0, OPTION_DISCONNECT, 0, "Disconnect").setIcon(R.drawable.menu_disconnect);
        return true;
    }

    public boolean onOptionsItemSelected(MenuItem item) {
    	// Handle menu buttons.
    	final EditText message = (EditText)findViewById(R.id.message);
    	final TabWidget tabWidget = (TabWidget)findViewById(android.R.id.tabs);
        switch(item.getItemId()) {
        	case OPTION_JOIN_CHAT:
        		if (!userInChat) {
        			VentriloInterface.joinchat();
        			message.setEnabled(true);
        			message.setVisibility(TextView.VISIBLE);
        			userInChat = true;
        			item.setIcon(R.drawable.menu_leave_chat);
        			item.setTitle("Leave chat");
        		} else {
        			VentriloInterface.leavechat();
        			message.setEnabled(false);
        			message.setVisibility(TextView.GONE);
        			userInChat = false;
        			item.setIcon(R.drawable.menu_join_chat);
        			item.setTitle("Join chat");
        		}
        		break;
        		
        	case OPTION_HIDE_TABS:
        		if (tabsHidden) {
        			tabsHidden = false;
        			item.setIcon(R.drawable.menu_hide_tabs);
        			item.setTitle("Hide Tabs");
        			tabWidget.setEnabled(true);
        			tabWidget.setVisibility(TextView.VISIBLE);
        		} else {
        			tabsHidden = true;
        			item.setIcon(R.drawable.menu_show_tabs);
        			item.setTitle("Show Tabs");
        			tabWidget.setEnabled(false);
        			tabWidget.setVisibility(TextView.GONE);
        		}
        		break;
        		
        	case OPTION_ADMIN_LOGIN:
        		final MenuItem fitem = item;
				final EditText input = new EditText(this);
				input.setInputType(InputType.TYPE_TEXT_VARIATION_VISIBLE_PASSWORD);
				AlertDialog.Builder alert = new AlertDialog.Builder(this)
				.setTitle("Admin Password")
				.setMessage("Please enter the server's admin password")
				.setView(input)
				.setPositiveButton("OK", new DialogInterface.OnClickListener() {
					public void onClick(DialogInterface dialog, int whichButton) {
						VentriloInterface.adminlogin(input.getText().toString());
						// we get booted if this fails, so assume success
						setIsAdmin(true);
						fitem.setVisible(false);
					}
				})
				.setNegativeButton("Cancel", new DialogInterface.OnClickListener() {
					public void onClick(DialogInterface dialog, int whichButton) {
						// No password entered.
					}
				});
				alert.show();
        		break;
        		
        	case OPTION_DISCONNECT:
        		VentriloInterface.logout();
        		finish();
        		return true;
        		
        	case OPTION_SETTINGS:
				Intent intent = new Intent(ServerView.this, Settings.class);
				startActivity(intent);
        		return true;

        	default:
        		return false;
        }
        return true;
    }

	public void setIsAdmin(boolean isAdmin) {
		this.isAdmin = isAdmin;
	}
	
	public boolean getIsAdmin() {
		return isAdmin;
	}
	
	private void messagesAppend(String text) {
		final TextView textview = (TextView)findViewById(R.id.messages);
		final ScrollView scrollview = (ScrollView)findViewById(R.id.chatScroll);
		textview.append(text);
		Linkify.addLinks(textview, Linkify.ALL);
		scrollview.post(new Runnable() {
			public void run() {
				scrollview.fullScroll(View.FOCUS_DOWN);
			}
		});
	}
	
	public void addChatMessage(String username, String message) {
		messagesAppend("\n" + username  + ": " + message);
	}
	
	public void addChatUser(String username) {
		messagesAppend("\n* " + username + " has joined the chat.");
	}
	
	public void removeChatUser(String username) {
		messagesAppend("\n* " + username + " has left the chat.");
	}
	
	public void notifyAdaptersDataSetChanged() {
		ChannelListEntity entity = new ChannelListEntity(
				ChannelListEntity.CHANNEL,
				VentriloInterface.getuserchannel(VentriloInterface.getuserid()));
		if (findViewById(R.id.userViewHeader) != null) {
			((TextView)findViewById(R.id.userViewHeader)).setText(
					"" + UserList.data.size() + " Users | " +
					(entity.id == 0 ? "Lobby" : entity.name)
					);
		}
		userAdapter.notifyDataSetChanged();
		channelAdapter.notifyDataSetChanged();
	}
	
	
	private BroadcastReceiver eventReceiver = new BroadcastReceiver() {
		public void onReceive(Context context, Intent intent) {
			eventHandler.process();
		}
	};
	
	private BroadcastReceiver notifyReceiver = new BroadcastReceiver() {
		public void onReceive(Context context, Intent intent) {
			Toast.makeText(ServerView.this, intent.getStringExtra("message"), Toast.LENGTH_SHORT).show();
		}
	};
	
	private void changeChannel(final short channelid) {
		if(VentriloInterface.getuserchannel(VentriloInterface.getuserid()) != channelid) {
			final int protectedby;
			if((protectedby = VentriloInterface.channelrequirespassword(channelid)) > 0) {
				final String password = dbHelper.getPassword(serverid, protectedby);
				final EditText input = new EditText(this);
				input.setInputType(InputType.TYPE_TEXT_VARIATION_VISIBLE_PASSWORD);
				input.setText(password);
				// Create dialog box for password.
				AlertDialog.Builder alert = new AlertDialog.Builder(this)
					.setTitle("Channel is password protected")
					.setMessage("Please insert a password to join this channel.")
					.setView(input)
					.setPositiveButton("OK", new DialogInterface.OnClickListener() {
						public void onClick(DialogInterface dialog, int whichButton) {
							if (input.getText().toString() != password) {
								dbHelper.setPassword(serverid, protectedby, input.getText().toString());
							}
							VentriloInterface.changechannel(channelid, input.getText().toString());
						}
					})
					.setNegativeButton("Cancel", new DialogInterface.OnClickListener() {
						public void onClick(DialogInterface dialog, int whichButton) {
							// No password entered.
						}
					});
				alert.show();
			}
			else {
				// No password required.
				VentriloInterface.changechannel(channelid, "");
			}
		}
	}

	private OnItemClickListener onListClick = new OnItemClickListener() {
		@SuppressWarnings("unchecked")
		public void onItemClick(AdapterView<?> parent, View view, int position, long id) {
			short channelid = 0;
			int type = (Integer)((HashMap<String, Object>)(parent.getItemAtPosition(position))).get("type");
			if (type == ChannelListEntity.CHANNEL) {
				channelid = (Short)((HashMap<String, Object>)(parent.getItemAtPosition(position))).get("id");
			} else {
				channelid = (Short)((HashMap<String, Object>)(parent.getItemAtPosition(position))).get("parentid");
			}
			changeChannel(channelid);
		}
	};
	
	private void setUserVolume(short id) {
		final ChannelListEntity entity = new ChannelListEntity(ChannelListEntity.USER, id);
		LayoutInflater inflater = (LayoutInflater) this.getSystemService(LAYOUT_INFLATER_SERVICE);
		final View layout = inflater.inflate(R.layout.volume, null);
		AlertDialog.Builder alert = new AlertDialog.Builder(this);
		final TextView percent = (TextView) layout.findViewById(R.id.VolumeLevel);
		final SeekBar seek = (SeekBar) layout.findViewById(R.id.VolumeBar);
		seek.setMax(158);
		int level = dbHelper.getVolume(serverid, entity.name);
		level = level < 148 ? level : 158; // hack for volume oddities
		if (id == VentriloInterface.getuserid()) {
			alert.setTitle("Set Transmit Volume");
			level = PreferenceManager.getDefaultSharedPreferences(getBaseContext()).getInt("xmitvolume", 79);
			seek.setProgress(level);
		} else {
			alert.setTitle("Set Volume for " + entity.name);
			seek.setProgress(level);
		}
		percent.setText((seek.getProgress() * 200) / seek.getMax() + "%");
		seek.setOnSeekBarChangeListener(new OnSeekBarChangeListener() {
			public void onProgressChanged(SeekBar seekBar, int progress,
					boolean fromUser) {
				if (progress >= 67 && progress <= 81) {
					seekBar.setProgress(79);
					percent.setText("100%");
				} else if (progress >= 148) {
					seekBar.setProgress(158);
					percent.setText("200%");
				} else
					percent.setText((progress * 200) / seekBar.getMax() + "%");
			}
			public void onStartTrackingTouch(SeekBar seekBar) {
			}
			public void onStopTrackingTouch(SeekBar seekBar) {
			}
		});
		alert.setView(layout)
		.setPositiveButton("OK", new DialogInterface.OnClickListener() {
			public void onClick(DialogInterface dialog, int which) {
				int level = seek.getProgress() > 148 ? 148 : seek.getProgress();
				if (entity.id == VentriloInterface.getuserid()) {
					VentriloInterface.setxmitvolume(level);
					dbHelper.setVolume(serverid, entity.name, level);
					PreferenceManager.getDefaultSharedPreferences(getBaseContext()).edit().putInt("xmitvolume", level);
				} else {
					VentriloInterface.setuservolume(entity.id, level);
					dbHelper.setVolume(serverid, entity.name, level);
				}
			}
		}).setNegativeButton("Cancel", new DialogInterface.OnClickListener() {
			public void onClick(DialogInterface dialog, int which) {
				return;
			}
		});
		alert.show();
	}
	
	@Override
	public void onCreateContextMenu(ContextMenu menu, View v, ContextMenuInfo menuInfo) {
		super.onCreateContextMenu(menu, v, menuInfo);
		AdapterContextMenuInfo cmi = (AdapterContextMenuInfo) menuInfo;
		int clposition = cmi.position;
		int groupId = 0;
		if (v.getId() == R.id.userList) {
			groupId = 1;
			HashMap<String, Object> user = UserList.data.get(cmi.position);
			clposition = UserList.getChannelListLocation(Short.parseShort(user.get("userid").toString()));
		}		
		ChannelListEntity entity = new ChannelListEntity(ChannelList.data.get(clposition));
		Log.d("mangler", "isChannel: " + ChannelList.isChannel(clposition));
		
		if (entity.type == ChannelListEntity.USER) {
			if (VentriloInterface.getuserid() == entity.realUserId) {
				menu.setHeaderTitle(entity.name);
				int itempos = 1;
				menu.add(groupId, CM_OPTION_PHANTOM, itempos++, "Remove Phantom");
			} else if (entity.id != VentriloInterface.getuserid()) {
				// create the menu for other users
				int itempos = 1;
				boolean serveradmin = VentriloInterface.getpermission("serveradmin");
				Log.d("mangler", "am i a server admin? " + serveradmin);
				menu.setHeaderTitle(entity.name);
				
				menu.add(groupId, CM_OPTION_VOLUME, itempos++, "Set Volume");
				if (entity.comment != "" ||	entity.url != "") {
					menu.add(groupId, CM_OPTION_COMMENT, itempos++, "View Comment/URL");
				}
				if (dbHelper.getVolume(serverid, entity.name) == 0) {
					menu.add(groupId, CM_OPTION_MUTE, itempos++, "Unmute");
				} else {
					menu.add(groupId, CM_OPTION_MUTE, itempos++, "Mute");
				}
				if (serveradmin || VentriloInterface.getpermission("sendpage")) {
					menu.add(groupId, CM_OPTION_SEND_PAGE, itempos++, "Send Page");
				}
				if (!entity.inMyChannel()) {
					if (serveradmin || VentriloInterface.getpermission("moveuser")) {
						menu.add(groupId, CM_OPTION_MOVE_USER, itempos++, "Move User to Your Channel");
					}
				}
				if (serveradmin || VentriloInterface.getpermission("kickuser")) {
					menu.add(groupId, CM_OPTION_KICK, itempos++, "Kick");
				}
				if (serveradmin || VentriloInterface.getpermission("banuser")) {
					menu.add(groupId, CM_OPTION_BAN, itempos++, "Ban");
				}
				if (serveradmin) {
					menu.add(groupId, CM_OPTION_GLOBAL_MUTE, itempos++, "Global Mute");
				}
			} else {
				// create menu for our own options
				int itempos = 1;
				
				// this is done in the settings menu now
				//menu.add(groupId, CM_OPTION_VOLUME, itempos++, "Set Transmit Level");
				menu.add(groupId, CM_OPTION_COMMENT, itempos++, "Set Comment");
				menu.add(groupId, CM_OPTION_URL, itempos++, "Set URL");
			}
		} else if (entity.type == ChannelListEntity.CHANNEL) {
			menu.setHeaderTitle(entity.name);
			int itempos = 1;
			menu.add(groupId, CM_OPTION_PHANTOM, itempos++, "Add Phantom");
		}
	}
	
	@Override
	public boolean onContextItemSelected(MenuItem item) {
		AdapterContextMenuInfo cmi = (AdapterContextMenuInfo) item.getMenuInfo();
		int clposition = cmi.position;
		
		// see if the item was clicked from the user list
		if (item.getGroupId() == 1) {
			HashMap<String, Object> user = UserList.data.get(cmi.position);
			clposition = UserList.getChannelListLocation(Short.parseShort(user.get("userid").toString()));
		}
		
		short id = Short.parseShort(ChannelList.data.get(clposition).get("id").toString());
		ChannelListEntity entity = new ChannelListEntity(ChannelListEntity.USER, id);
		if (ChannelList.isChannel(clposition) == false) {
			if (entity.realUserId == VentriloInterface.getuserid()) {
				removePhantom(VentriloInterface.getuserchannel(entity.id));
			} else {
				switch (item.getItemId()) {
				case CM_OPTION_VOLUME:
					setUserVolume(id);
					break;
				case CM_OPTION_MUTE:
					if ((dbHelper.getVolume(serverid, entity.name)) == 0) {
						dbHelper.setVolume(serverid, entity.name, 79);
						VentriloInterface.setuservolume(entity.id, 79);
					} else {
						dbHelper.setVolume(serverid, entity.name, 0);
						VentriloInterface.setuservolume(entity.id, 0);
					}
					break;
				case CM_OPTION_GLOBAL_MUTE:
					VentriloInterface.globalmute(entity.id);
					break;
				case CM_OPTION_SEND_PAGE:
					VentriloInterface.sendpage(entity.id);
					break;
				case CM_OPTION_MOVE_USER:
					Log.e("mangler", "moving user" + entity.id + " to channel " + VentriloInterface.getuserchannel(VentriloInterface.getuserid()));
					if (!entity.inMyChannel()) {
						VentriloInterface.forcechannelmove(entity.id, VentriloInterface.getuserchannel(VentriloInterface.getuserid()));
					}
					break;
				case CM_OPTION_KICK:
					kickUser(id);
					break;
				case CM_OPTION_BAN:
					banUser(id);
					break;
				case CM_OPTION_COMMENT:
					if (id == VentriloInterface.getuserid()) {
						final ChannelListEntity fentity = entity;
						final EditText input = new EditText(this);
						input.setText(entity.comment);
						AlertDialog.Builder alert = new AlertDialog.Builder(this)
						.setTitle("Comment")
						.setMessage("Enter a comment:")
						.setView(input)
						.setPositiveButton("OK", new DialogInterface.OnClickListener() {
							public void onClick(DialogInterface dialog, int whichButton) {
								VentriloInterface.settext(input.getText().toString(), fentity.url, "", true);
							}
						})
						.setNegativeButton("Cancel", new DialogInterface.OnClickListener() {
							public void onClick(DialogInterface dialog, int whichButton) {
							}
						});
						alert.show();
					} else {
						viewComment(id);
					}
					break;
				case CM_OPTION_URL:
					if (id == VentriloInterface.getuserid()) {
						final ChannelListEntity fentity = entity;
						final EditText input = new EditText(this);
						if (entity.url.length() == 0) {
							input.setText("http://");
						} else {
							input.setText(entity.url);
						}
						AlertDialog.Builder alert = new AlertDialog.Builder(this)
						.setTitle("URL")
						.setMessage("Enter a URL:")
						.setView(input)
						.setPositiveButton("OK", new DialogInterface.OnClickListener() {
							public void onClick(DialogInterface dialog, int whichButton) {
								VentriloInterface.settext(fentity.comment, input.getText().toString(), "", true);
							}
						})
						.setNegativeButton("Cancel", new DialogInterface.OnClickListener() {
							public void onClick(DialogInterface dialog, int whichButton) {
							}
						});
						alert.show();
					}
					break;
				}
			}
		} else {
			switch (item.getItemId()) {
			case CM_OPTION_PHANTOM:
				addPhantom(id);
				break;		
			}
		}
		return super.onContextItemSelected(item);
	}
	
	private void addPhantom(short id) {
		VentriloInterface.phantomadd(id);
	}
	
	private void removePhantom(short id) {
		VentriloInterface.phantomremove(id);
	}
	private void kickUser(short id) {
		final EditText input = new EditText(this);
		final short userid = id;
		// Create dialog box for password.
		AlertDialog.Builder alert = new AlertDialog.Builder(this)
			.setTitle("Kick Reason")
			.setMessage("Please enter a reason for kicking this user")
			.setView(input)
			.setPositiveButton("OK", new DialogInterface.OnClickListener() {
				public void onClick(DialogInterface dialog, int whichButton) {
					VentriloInterface.kick(userid, input.getText().toString());
				}
			})
			.setNegativeButton("Cancel", new DialogInterface.OnClickListener() {
				public void onClick(DialogInterface dialog, int whichButton) {
				}
			});
		alert.show();
	}

	private void banUser(short id) {
		final EditText input = new EditText(this);
		final short userid = id;
		// Create dialog box for password.
		AlertDialog.Builder alert = new AlertDialog.Builder(this)
			.setTitle("Ban Reason")
			.setMessage("Please enter a reason for banning this user (note: you cannot unban users from this interface)")
			.setView(input)
			.setPositiveButton("OK", new DialogInterface.OnClickListener() {
				public void onClick(DialogInterface dialog, int whichButton) {
					VentriloInterface.ban(userid, input.getText().toString());
				}
			})
			.setNegativeButton("Cancel", new DialogInterface.OnClickListener() {
				public void onClick(DialogInterface dialog, int whichButton) {
				}
			});
		alert.show();
	}

	public void setUserVolumeFromDatabase(ChannelListEntity entity) {
		int level = dbHelper.getVolume(serverid, entity.name);
		VentriloInterface.setuservolume(entity.id, level);
	}
	
	private void viewComment(short id) {
		final ChannelListEntity entity = new ChannelListEntity(ChannelListEntity.USER, id);
		TextView comment = new TextView(this);
		comment.setPadding(15, 0, 15, 0);
		comment.setText("Comment: " + entity.comment + "\n" + "URL: " + entity.url);
		Linkify.addLinks(comment, Linkify.ALL);
		new AlertDialog.Builder(this).setTitle(entity.name).setView(comment).show();
	}

	private OnTouchListener onTalkPress = new OnTouchListener() {
		public boolean onTouch(View v, MotionEvent m) {
			boolean ptt_toggle = PreferenceManager.getDefaultSharedPreferences(getBaseContext()).getBoolean("ptt_toggle", false);
			
			switch (m.getAction()) {
				case MotionEvent.ACTION_DOWN:
					if (!Recorder.recording()) {
						startPtt();
					} else if (ptt_toggle) {
						stopPtt();
					}
					break;
				case MotionEvent.ACTION_UP:
					if (! ptt_toggle) {
						stopPtt();
					}
					break;
			}
			return true;
		}
	};
	
	private void startPtt() {
		boolean force_8khz = PreferenceManager.getDefaultSharedPreferences(getBaseContext()).getBoolean("force_8khz", false);
		Recorder.setForce_8khz(force_8khz);
		if (!Recorder.start()) {
			Toast.makeText(this, "Unsupported recording rate for hardware: " + Integer.toString(Recorder.rate()) + "Hz", Toast.LENGTH_SHORT).show();
			return;
		}
		boolean ptt_vibrate = PreferenceManager.getDefaultSharedPreferences(getBaseContext()).getBoolean("ptt_vibrate", false);
		if (ptt_vibrate) {
			Vibrator v = (Vibrator) getSystemService(Context.VIBRATOR_SERVICE);
			v.vibrate(50);
		}
		Player.setVolume(0.2f);
	    Button pttButton = ((Button)findViewById(R.id.talkButton));
	    pttButton.setPressed(true);
		((ImageView)findViewById(R.id.transmitStatus)).setImageResource(R.drawable.xmit_on);
		UserList.updateStatus(VentriloInterface.getuserid(), R.drawable.xmit_on);
		userAdapter.notifyDataSetChanged();
		ChannelList.updateStatus(VentriloInterface.getuserid(), R.drawable.xmit_on);
		channelAdapter.notifyDataSetChanged();
	}
	
	private void stopPtt() {
		((TextView)findViewById(R.id.recorderInfo)).setText(
				"Last Xmit Info\n\n" +
				"Channel Rate: " + VentriloInterface.getchannelrate(VentriloInterface.getuserchannel(VentriloInterface.getuserid())) + "\n" +
				"Record Rate: " + Recorder.rate() + "\n" +
				"Buffer Size: " + Recorder.buflen() + "\n");
		boolean ptt_vibrate = PreferenceManager.getDefaultSharedPreferences(getBaseContext()).getBoolean("ptt_vibrate", false);
		if (ptt_vibrate) {
			Vibrator v = (Vibrator) getSystemService(Context.VIBRATOR_SERVICE);
			v.vibrate(50);
		}
		Button pttButton = ((Button)findViewById(R.id.talkButton));
		pttButton.setPressed(false);
		Recorder.stop();
		Player.setVolume(1.0f);
		((ImageView)findViewById(R.id.transmitStatus)).setImageResource(R.drawable.xmit_off);
		UserList.updateStatus(VentriloInterface.getuserid(), R.drawable.xmit_off);
		userAdapter.notifyDataSetChanged();
		ChannelList.updateStatus(VentriloInterface.getuserid(), R.drawable.xmit_off);
		channelAdapter.notifyDataSetChanged();
	}

	private OnKeyListener onChatMessageEnter = new OnKeyListener() {
		public boolean onKey(View v, int keyCode, KeyEvent event) {
			if ((event.getAction() == KeyEvent.ACTION_DOWN) && (keyCode == KeyEvent.KEYCODE_ENTER)) {
				// Send chat message.
				final EditText message = (EditText)findViewById(R.id.message);
				VentriloInterface.sendchatmessage(message.getText().toString());

				// Clear message field.
				message.setText("");

				// Hide keyboard.
				((InputMethodManager)getSystemService(INPUT_METHOD_SERVICE)).hideSoftInputFromWindow(message.getWindowToken(), 0);
				return true;
			}
			return false;
		}
	};
	
	public void tts(String ttsType, String text) {
		boolean enable_tts = PreferenceManager.getDefaultSharedPreferences(getBaseContext()).getBoolean("enable_tts" + ttsType, false);
		Log.e("mangler", "enable tts: " + enable_tts);
		// TTS is only available on 1.6 and higher and Build.VERSION.SDK_INT
		// isn't available in 1.5 :/
		if (enable_tts && Build.VERSION.RELEASE != "1.5") {
			ttsWrapper.speak(text);
		}
	}
}
