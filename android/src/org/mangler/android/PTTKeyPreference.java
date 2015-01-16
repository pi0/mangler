package org.mangler.android;

import android.app.AlertDialog;
import android.content.Context;
import android.content.DialogInterface;
import android.content.SharedPreferences;
import android.content.DialogInterface.OnKeyListener;
import android.os.Bundle;
import android.preference.DialogPreference;
import android.preference.PreferenceManager;
import android.util.AttributeSet;
import android.view.Gravity;
import android.view.KeyCharacterMap;
import android.view.KeyEvent;
import android.view.View;
import android.widget.LinearLayout;
import android.widget.TextView;

public class PTTKeyPreference extends DialogPreference implements OnKeyListener {
	
	int currentKeyCode;
	TextView currentKeyTextView;
	KeyCharacterMap keyMap;
	
    public PTTKeyPreference(Context context, AttributeSet attrs) {
        super(context, attrs);
        setupLayout(context, attrs);
    }

    public PTTKeyPreference(Context context, AttributeSet attrs, int defStyle) {
        super(context, attrs, defStyle);
        setupLayout(context, attrs);
    }

    private void setupLayout(Context context, AttributeSet attrs) { }

    @Override
    protected View onCreateDialogView() {
    	keyMap = KeyCharacterMap.load(0);
    	
    	LinearLayout view = new LinearLayout(this.getContext());
    	view.setOrientation(LinearLayout.VERTICAL);
    	view.setGravity(Gravity.CENTER_HORIZONTAL);
    	view.setPadding(10, 10, 10, 10);
    	
    	TextView title = new TextView(this.getContext());
    	title.setText("Push PTT Key");
    		
    	currentKeyTextView = new TextView(this.getContext());
    	
    	currentKeyCode = getPersistedInt(0);
    	
    	currentKeyTextView.setText("PTT Key: " + (keyMap.isPrintingKey(currentKeyCode) ? Character.toString(keyMap.getDisplayLabel(currentKeyCode)) : currentKeyCode));
    	
    	view.addView(title);
    	view.addView(currentKeyTextView);
    	
    	return view;
    }
	
    @Override
    protected void showDialog(Bundle state) {
    	super.showDialog(state);
    	getDialog().setOnKeyListener(this);	
    }
    
    public void onDialogClosed(boolean positiveResult) {
    	if (positiveResult) {
    		SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(this.getContext());
    		prefs.edit().putInt("ptt_key", currentKeyCode).commit();
    		persistInt(currentKeyCode);
    		if (currentKeyCode == KeyEvent.KEYCODE_HEADSETHOOK) {
    			AlertDialog.Builder builder = new AlertDialog.Builder(this.getContext());
				builder.setMessage("The headset mic button can only be used as a toggle for push-to-talk.  " +
						"The PTT Toggle feature will be automatically enabled regardless of the setting specified.  " +
						"Holding down the button will prevent you from transmitting.")
    			       .setCancelable(false)
    			       .setPositiveButton("OK", new DialogInterface.OnClickListener() {
    			           public void onClick(DialogInterface dialog, int id) {
   			                	
    			           }
    			       });
    			AlertDialog alert = builder.create();
    			alert.show();
    		}
    	}
    }

	public boolean onKey(DialogInterface d, int keyCode, KeyEvent event) {  	
		if (event.getAction() == KeyEvent.ACTION_DOWN) {
			currentKeyCode = keyCode;
			currentKeyTextView.setText("PTT Key: " + (keyMap.isPrintingKey(currentKeyCode) ? Character.toString(keyMap.getDisplayLabel(currentKeyCode)) : currentKeyCode));
		}
		
		return true;
	}
}
