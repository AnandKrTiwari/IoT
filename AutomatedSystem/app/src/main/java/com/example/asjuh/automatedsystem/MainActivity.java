package com.example.asjuh.automatedsystem;

import android.app.Activity;
import android.content.Context;
import android.content.SharedPreferences;
import android.graphics.Color;
import android.os.AsyncTask;
import android.os.Bundle;
import android.view.View;
import android.widget.EditText;
import android.widget.Toast;
import android.widget.ToggleButton;
import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.net.HttpURLConnection;
import java.net.MalformedURLException;
import java.net.ProtocolException;
import java.net.URL;

public class MainActivity extends Activity implements View.OnClickListener {
    public final static String PREF_IP = "PREF_IP_ADDRESS";
    public final static String PREF_PORT = "PREF_PORT_NUMBER";
    // declare buttons and text inputs
    private ToggleButton buttonPin2,buttonPin,buttonPin3;
   // private ToggleButton IOTbutton;
    private EditText editTextIPAddress, editTextPortNumber;
    // shared preferences objects used to save the IP address and port so that the user doesn't have to
    // type them next time he/she opens the app.
    SharedPreferences.Editor editor;
    SharedPreferences sharedPreferences;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        sharedPreferences = getSharedPreferences("HTTP_HELPER_PREFS", Context.MODE_PRIVATE);
        editor = sharedPreferences.edit();

        // assign buttons
        //IOTbutton = (ToggleButton)findViewById(R.id.switch1);
        buttonPin2 = (ToggleButton)findViewById(R.id.toggleButton2);
        buttonPin = (ToggleButton)findViewById(R.id.toggleButton);
        buttonPin3 = (ToggleButton)findViewById(R.id.toggleButton3);

        // assign text inputs
        editTextIPAddress = (EditText)findViewById(R.id.editTextIPAddress);
        editTextPortNumber = (EditText)findViewById(R.id.editTextPortNumber);

        // set button listener (this class)
        buttonPin2.setOnClickListener(this);
        buttonPin.setOnClickListener(this);
        buttonPin3.setOnClickListener(this);
       //  IOTbutton.setOnClickListener(this);
        // get the IP address and port number from the last time the user used the app,
        // put an empty string "" is this is the first time.
        editTextIPAddress.setText(sharedPreferences.getString(PREF_IP,""));
        editTextPortNumber.setText(sharedPreferences.getString(PREF_PORT,""));
    }

    @Override
    public void onClick(View view) {

        // get the pin number
        StringBuilder parameterValue = new StringBuilder("2");
        //StringBuilder parameter= new StringBuilder("2");
        // get the ip address
        String ipAddress = editTextIPAddress.getText().toString().trim();
        // get the port number
        String portNumber = editTextPortNumber.getText().toString().trim();


        // save the IP address and port for the next time the app is used
        editor.putString(PREF_IP, ipAddress); // set the ip address value to save
        editor.putString(PREF_PORT, portNumber); // set the port number to save
        editor.commit(); // save the IP and PORT

        // get the pin number from the button that was clicked
        if(view.getId() == buttonPin2.getId() && buttonPin2.getText().equals("ON") ) {
                parameterValue.replace(0,1,"8");
                buttonPin2.setTextColor(Color.GREEN);
        }
        else if(view.getId() == buttonPin.getId() && buttonPin.getText().equals("ON")){
            parameterValue.replace(0,1,"6");
            buttonPin.setTextColor(Color.GREEN);
        }
        else if(view.getId() == buttonPin2.getId() && buttonPin2.getText().equals("OFF")){
                parameterValue.replace(0,1,"1");
                buttonPin2.setTextColor(Color.RED);
        }
        else if(view.getId() == buttonPin.getId() && buttonPin.getText().equals("OFF")){
            parameterValue.replace(0,1,"2");
            buttonPin.setTextColor(Color.RED);
        }
        else if(view.getId() == buttonPin3.getId() && buttonPin3.getText().equals("ON")){
            parameterValue.replace(0,1,"4");
            buttonPin3.setTextColor(Color.GREEN);
        }
        else if(view.getId() == buttonPin3.getId() && buttonPin3.getText().equals("OFF")){
            parameterValue.replace(0,1,"3");
            buttonPin3.setTextColor(Color.RED);
        }
        if(ipAddress.length()>0 && portNumber.length()>0) {
            HttpGetRequest htp = new HttpGetRequest(this);
            htp.execute("http://" + ipAddress + ":" + portNumber + "/?" + "pin=" + parameterValue);
        }
    }



    public class HttpGetRequest extends AsyncTask<String,Void,String>{
        static final String REQUEST_METHOD = "GET";
        static final int READ_TIMEOUT = 30000;
        static final int CONNECTION_TIMEOUT = 30000;

        private Context mContext;
        String result;
        public HttpGetRequest(Context context) {
            //Relevant Context should be provided to newly created components (whether application context or activity context)
            //getApplicationContext() - Returns the context for all activities running in application
            mContext = context.getApplicationContext();
        }
        @Override
        protected String doInBackground(String... params) {
            String stringUrl = params[0];
            try {
                URL myUrl = new URL(stringUrl);
                //Create a connection
                HttpURLConnection connection =(HttpURLConnection)
                        myUrl.openConnection();
                //Set methods and timeouts
                connection.setRequestMethod(REQUEST_METHOD);
                connection.setReadTimeout(READ_TIMEOUT);
                connection.setConnectTimeout(CONNECTION_TIMEOUT);
                connection.setDoInput(true);
                //connection.setDoOutput(true);
                //Connect to our url
                connection.connect();
                //Create a new InputStreamReader
                InputStreamReader streamReader = new
                        InputStreamReader(connection.getInputStream());
                //Create a new buffered reader and String Builder
                BufferedReader reader = new BufferedReader(streamReader);
                StringBuilder stringBuilder = new StringBuilder();
                //Check if the line we are reading is not null
                String inputLine;
                while((inputLine = reader.readLine()) != null){
                    stringBuilder.append(inputLine);
                }
                //Close our InputStream and Buffered reader
                reader.close();
                streamReader.close();
                //Set our result equal to our stringBuilder
                result = stringBuilder.toString();
                connection.disconnect();

            } catch (ProtocolException e) {
                e.printStackTrace();
            } catch (MalformedURLException e) {
                e.printStackTrace();
            } catch (IOException e) {
                e.printStackTrace();
            }

            return result;
        }

        @Override
        protected void onPostExecute(String r) {
            //Print the response code as toast popup
            Toast.makeText(mContext, "Response code: " + r,
                    Toast.LENGTH_SHORT).show();

        }

        @Override
        protected void onPreExecute() {
            Toast.makeText(mContext, "Going for the network call..", Toast.LENGTH_SHORT).show();
        }
    }
}

