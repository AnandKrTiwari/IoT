#include <SoftwareSerial.h>
 
#define DEBUG true
 int led = 8;   //testing the Iot logic,put an led at pin 8
  boolean iotLogic=true;//Iot Status
int threshold = 500;
int bulbThreshold = 200;// correct value to be put after testing the bulb and its affect on ldr
SoftwareSerial esp8266(2,3); // make RX Arduino line is pin 2, make TX Arduino line is pin 3.
                             // This means that you need to connect the TX line from the esp to the Arduino's pin 2
                            // and the RX line from the esp to the Arduino's pin 3
void setup()
{
  
 
  pinMode(led,OUTPUT);
  Serial.begin(115200);
  esp8266.begin(115200); // your esp's baud rate might be different
  
  pinMode(11,OUTPUT);
  digitalWrite(11,LOW);
   
  sendCommand("AT+RST\r\n",3000,DEBUG); // reset module
  sendCommand("AT+CWMODE=3\r\n",3000,DEBUG); // configure as access point
  sendCommand("AT+CWJAP=\"PKS\",\"91101132\"\r\n",3000,DEBUG);
  delay(10000);
  sendCommand("AT+CIFSR\r\n",3000,DEBUG); // get ip address
  sendCommand("AT+CIPMUX=1\r\n",3000,DEBUG); // configure for multiple connections
  sendCommand("AT+CIPSERVER=1,80\r\n",3000,DEBUG); // turn on server on port 80
  
  Serial.println("Server Ready");
}

void loop()
{
  //int pinNumber=11;
    int pinNo;
    int connectionId;

    int sensorValue = analogRead(A0);
  
  if(esp8266.available()) // check if the esp is sending a message 
  {

     if(esp8266.find("+IPD,"))
    {
     delay(1000); // wait for the serial buffer to fill up (read all the serial data)
     // get the connection id so that we can then disconnect
      connectionId = esp8266.read()-48; // subtract 48 because the read() function returns 
                                           // the ASCII decimal value and 0 (the first decimal number) starts at 48
    }

      //switch for IotLogic
      if(esp8266.find("IoT="))
      {
         int iot= esp8266.read()-48;
         if(iot == 1)
         {
          iotLogic = true;
         }
         else if(iot == 0)
         {
          iotLogic = false;
         }
              
      }

      if(iotLogic == true)//ldr controls
      {
             if(sensorValue > threshold)
            {
             digitalWrite(led,LOW);
            
            }
            if(sensorValue<threshold)
            {
              
             digitalWrite(led,HIGH);
            
            } 
            delay(1);   
      }


      
      if(iotLogic == false) //switch controls
      {
         esp8266.find("pin="); // advance cursor to "pin="
          pinNo= esp8266.read()-48;
         Serial.println(pinNo);

         //Toggle switch
         //int pinNo=val;
      digitalWrite(pinNo,!digitalRead(pinNo));//toggle
        
      }


 int sensorBulb = analogRead(A1);

      
     // build string that is send back to device that is requesting pin toggle
     String content;
     content = "Pin ";
     content += pinNo;
     content += " is ";
     
     //if(digitalRead(pinNo) && sensorBulb<bulbThreshold)  // accounts for the change is the value of ldr serving as a tester for Bulb
     if(digitalRead(pinNo))
     {
       content += "ON";
     }
     else
     {
       content += "OFF";
     }

     sendHTTPResponse(connectionId,content);
     
     // make close command
     String closeCommand = "AT+CIPCLOSE="; 
     closeCommand+=connectionId; // append connection id
     closeCommand+="\r\n";
     
     sendCommand(closeCommand,1000,DEBUG); // close connection
    }
  }



//====================================



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
 
