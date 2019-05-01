#include <SoftwareSerial.h>
 
#define DEBUG true
int led =8;
int ldr = A0;
int threshold=500;
int connectionId;
int sensorval;
int a=0;
SoftwareSerial esp8266(10,11); // make RX Arduino line is pin 2, make TX Arduino line is pin 3.
                             // This means that you need to connect the TX line from the esp to the Arduino's pin 2
                             // and the RX line from the esp to the Arduino's pin 3
void setup()
{
  Serial.begin(9600);
  esp8266.begin(9600); // your esp's baud rate might be different
  
  pinMode(led,OUTPUT);   
  digitalWrite(led,LOW);
  sendCommand("AT+RST\r\n",2000,DEBUG); // reset module
  sendCommand("AT+CWMODE=2\r\n",2000,DEBUG); // configure as access point
  delay(3000);
  sendCommand("AT+CIFSR\r\n",2000,DEBUG); // get ip address
  sendCommand("AT+CIPMUX=1\r\n",2000,DEBUG); // configure for multiple connections
  sendCommand("AT+CIPSERVER=1,80\r\n",2000,DEBUG); // turn on server on port 80
  Serial.println("Server Ready");
}
 
void loop()
{
   sensorval=analogRead(ldr);
    if(a!=11){
   if(sensorval>threshold){
     digitalWrite(8,LOW);
    }
   else{
    digitalWrite(8,HIGH);
   }
  }
  delay(500);
  if(esp8266.available()) // check if the esp is sending a message 
  {
     a = checkLed();
     // build string that is send back to device that is requesting pin toggle
     String content;
     content = "Pin ";
     content += led;
     content += " is ";
     
     if(digitalRead(led))
     {
       content += "ON";
     }
     else
     {
       content += "OFF";
     }
     
     sendHTTPResponse(connectionId,content);
     
     // make close command
     //String closeCommand = "AT+CIPCLOSE="; 
     //loseCommand+=connectionId; // append connection id
     //closeCommand+="\r\n";
     
     //sendCommand(closeCommand,1000,DEBUG); // close connection
  }
}

/*void checkLdr(int sensorval){
      if(esp8266.find("+IPD,"))
    {
     delay(1000); // wait for the serial buffer to fill up (read all the serial data)
     // get the connection id so that we can then disconnect
     connectionId = esp8266.read()-48; // subtract 48 because the read() function returns 
                                           // the ASCII decimal value and 0 (the first decimal number) starts at 48   
     Serial.println(connectionId);
         esp8266.find("IOT=");// advance cursor to "pin="
         delay(1000);
     int iot= esp8266.read()-48;
     Serial.println(iot);
      if(iot==1){
      if(sensorval>400){
        digitalWrite(led,LOW);
      }
      else{
        digitalWrite(led,HIGH);
      }
    }
}
}*/
 
 int checkLed(){
   if(esp8266.find("+IPD,"))
    {
     delay(1000); // wait for the serial buffer to fill up (read all the serial data)
     // get the connection id so that we can then disconnect
     connectionId = esp8266.read()-48; // subtract 48 because the read() function returns 
                                           // the ASCII decimal value and 0 (the first decimal number) starts at 48   
     Serial.println(connectionId);

      esp8266.find("pin=");
      delay(500);
      int val=esp8266.read()-48;
      Serial.println(val);
      if(val==1){
        digitalWrite(led,HIGH);
      }else if(val==0){
        digitalWrite(led,LOW);
      }
      if(digitalRead(led)){
        return 11;
      }
      else{
        return 2;
      }
    }
 }
/*
* Name: sendData
* Description: Function used to send data to ESP8266.
* Params: command - the data/command to send; timeout - the time to wait for a response; debug - print to Serial window?(true = yes, false = no)
* Returns: The response from the esp8266 (if there is a reponse)
*/
String sendData(String command, const int timeout, boolean debug)
{
    String response = "";
    
    int dataSize = command.length();
    char data[dataSize];
    command.toCharArray(data,dataSize);
           
    esp8266.write(data,dataSize); // send the read character to the esp8266
    if(debug)
    {
      Serial.println("\r\n====== HTTP Response From Arduino ======");
      Serial.write(data,dataSize);
      Serial.println("\r\n========================================");
    }
    
    long int time = millis();
    
    while( (time+timeout) > millis())
    {
      while(esp8266.available())
      {
        
        // The esp has data so display its output to the serial window 
        char c = esp8266.read(); // read the next character.
        response+=c;
      }  
    }
    
    if(debug)
    {
      Serial.println(response);
    }
    
    return response;
}
 
/*
* Name: sendHTTPResponse
* Description: Function that sends HTTP 200, HTML UTF-8 response
*/
void sendHTTPResponse(int connectionId, String content)
{
     
     // build HTTP response
     String httpResponse;
     String httpHeader;
     // HTTP Header
     httpHeader = "HTTP/1.1 200 OK\r\nContent-Type: text/html; charset=UTF-8\r\n"; 
     httpHeader += "Content-Length: ";
     httpHeader += content.length();
     httpHeader += "\r\n";
     httpHeader +="Connection: keep-alive\r\n\r\n";
     httpResponse = httpHeader + content + " "; // There is a bug in this code: the last character of "content" is not sent, I cheated by adding this extra space
     sendCIPData(connectionId,httpResponse);
}
 
/*
* Name: sendCIPDATA
* Description: sends a CIPSEND=<connectionId>,<data> command
*
*/
void sendCIPData(int connectionId, String data)
{
   String cipSend = "AT+CIPSEND=";
   cipSend += connectionId;
   cipSend += ",";
   cipSend +=data.length();
   cipSend +="\r\n";
   sendCommand(cipSend,1000,DEBUG);
   sendData(data,1000,DEBUG);
}
 
/*
* Name: sendCommand
* Description: Function used to send data to ESP8266.
* Params: command - the data/command to send; timeout - the time to wait for a response; debug - print to Serial window?(true = yes, false = no)
* Returns: The response from the esp8266 (if there is a reponse)
*/
String sendCommand(String command, const int timeout, boolean debug)
{
    String response = "";
           
    esp8266.print(command); // send the read character to the esp8266
    
    long int time = millis();
    
    while( (time+timeout) > millis())
    {
      while(esp8266.available())
      {
        
        // The esp has data so display its output to the serial window 
        char c = esp8266.read(); // read the next character.
        response+=c;
      }  
    }
    
    if(debug)
    {
      Serial.print(response);
    }
    
    return response;
}
 
 