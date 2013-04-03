package com.colingibbs;


import java.io.IOException;

import org.apache.http.HttpEntity;
import org.apache.http.HttpResponse;
import org.apache.http.client.ClientProtocolException;
import org.apache.http.client.methods.HttpPost;
import org.apache.http.impl.client.DefaultHttpClient;
import org.apache.http.util.EntityUtils;

import com.larswerkman.colorpicker.ColorPicker;
import com.larswerkman.colorpicker.ColorPicker.OnColorChangedListener;
import com.larswerkman.colorpicker.OpacityBar;
import com.larswerkman.colorpicker.SVBar;

import android.app.Activity;
import android.graphics.Color;
import android.os.AsyncTask;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.EditText;
import android.widget.TextView;

public class WeblightsActivity extends Activity implements OnColorChangedListener {
    
	private static final String TAG = "Weblights";
	private static final String baseurl = "http://192.168.0.11/";
	private ColorPicker picker;
	private int red;
	private int green;
	private int blue;
	private TextView redtext;
	private TextView greentext;
	private TextView bluetext;
	
	/** Called when the activity is first created. */
    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.main);
        
        picker = (ColorPicker) findViewById(R.id.picker);

		picker.setOnColorChangedListener(this);
	
    }
    
	@Override
	public void onColorChanged(int color) {
		//gives the color when it's changed.
	}
    
    //this is the method that gets called when a button is pushed
    //all of the buttons are defined in main.xml
    public void ChangeColor(View v){
		String params = "";
    	
		//figure out which button was pushed
		switch(v.getId()){
    	case R.id.off:
    		params = "off";
    		break;
    	case R.id.fade:
    		//first param is time on each color (in microseconds)
    		//second param is time per step (multiply by 255 to get time to fade)
    		//EditText timeAtColor = (EditText) findViewById(R.id.setTimeAtColor);
    		//EditText timePerStep = (EditText) findViewById(R.id.setTimePerStep);
    		
    		//params = "fade/"+ timeAtColor.getText().toString() + "/" + timePerStep.getText().toString();
    		params = "fade/10000000/200000";
    		break;
    	case R.id.changecolor:
    		picker.setOldCenterColor(picker.getColor());
			
			red = Color.red(picker.getColor());
			green = Color.green(picker.getColor());
			blue = Color.blue(picker.getColor());
			
			String url = baseurl + "rgb/" + red + "/" + green + "/" + blue;
			Log.i(TAG, "Submitting change: " + url);
	    	
	    	//need to submit this in a background thread or Android will cry
	    	new SubmitColorChange().execute(url);
    		break;
		}
		
    	String url = baseurl + params;
    	Log.i(TAG, "Submitting change: " + url);
    	
    	//need to submit this in a background thread or Android will cry
    	new SubmitColorChange().execute(url);
    }   
}

//this handles submitting the http request in the background
class SubmitColorChange extends AsyncTask<String, Void, String> {
	 
    private static final String TAG = "Weblights";
    DefaultHttpClient httpclient = new DefaultHttpClient();

    protected String doInBackground(String... urls) {
        try {
            String url= urls[0];
            HttpPost httppost = new HttpPost(url);
         // Execute HTTP Post Request
    	    HttpResponse response = httpclient.execute(httppost);
    	    HttpEntity resEntity = response.getEntity();  
    	    
            Log.i(TAG,EntityUtils.toString(resEntity));
            return EntityUtils.toString(resEntity);
        } catch (Exception e) {
        	e.printStackTrace();
        	return null;
        }
    }

    protected void onPostExecute(String response) {
        // TODO: check this.exception 
        // TODO: do something with the feed
    }
}