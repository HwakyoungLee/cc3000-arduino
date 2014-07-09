/*

 An example for connecting to BERG Cloud using the CC3000 shield.
 
 Copyright (c) 2014 BERG Cloud Ltd. http://bergcloud.com/
 
 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:
 
 The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.
 
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.
 
 */

#include <EEPROM.h>
#include <WebSocketClient.h>
#include <aJSON.h>
#include <Base64.h>
#include <SPI.h>
#include <Adafruit_CC3000.h>
#include <BERGCloudCC3000.h>

// This example describes how to connect to Berg, recieve commands and echo events back up to Berg.
// For a less detailed example please see WiFi_to_Website in the Examples/BERGCloudCC3000 menu.

//
// WiFi network configuration
//
// There are 2 ways to connect to Wifi: Smart Config or a hard coded SSID/Pass

// set WLAN_USE_SMARTCONFIG to true to use Smart Config.
#define WLAN_USE_SMARTCONFIG          true
// set these if you have set WLAN_USE_SMARTCONFIG to false
#define WLAN_SSID                     "my network"
#define WLAN_PASS                     "my password"
#define WLAN_SEC                      WLAN_SEC_WPA2

// if set to true WLAN_RECONNECT will cause the CC3000 to try and use previously saved Smart Config data.
// if set to false you will have to configure it everytime
#define WLAN_RECONNECT                true
// These can be set to increase the security of the data transfer between the Smart Config app and your device during configuration.
#define WLAN_SMARTCONFIG_DEVICE_NAME  "CC3000"
#define WLAN_SMARTCONFIG_KEY          NULL

// These values should be edited to reflect your Project setup on http://bergcloud.com/devcenter/
#define PROJECT_KEY "00000000000000000000000000000000"

#define VERSION     1

// set up a timer and some variables to regulate checking for commands
unsigned long pollTimer;
int pollGap = 1000;
unsigned long connectionTimeMS;
String deviceID;
boolean receivedCommand;

void setup()
{
  // open a serial connection over US
  Serial.begin(115200);
  Serial.println(F("--- reset ---"));
  Serial.print(F("BERGCloudCC3000 version: "));
  Serial.println(BERGCLOUD_LIB_VERSION, HEX);

  BERGCloudWLANConfig WLANConfig;
  WLANConfig.ssid = WLAN_SSID;
  WLANConfig.pass = WLAN_PASS;
  WLANConfig.secmode = WLAN_SEC;
  WLANConfig.smartConfigReconnect = WLAN_RECONNECT;
  WLANConfig.smartConfig = WLAN_USE_SMARTCONFIG;
  WLANConfig.smartConfigDeviceName = WLAN_SMARTCONFIG_DEVICE_NAME;
  WLANConfig.smartConfigKey = WLAN_SMARTCONFIG_KEY;
  BERGCloud.begin(WLANConfig);

  // To reset the claimcode of the device make a 
  // connection between pin 49 and GND during powerup
  checkForReclaimPin(49);

  Serial.print(F("Command/Event poll = "));
  Serial.print(pollGap);
  Serial.println(F("ms"));

  connectionTimeMS = millis();
  if (BERGCloud.connect(PROJECT_KEY, VERSION))
  {
    Serial.print("Connecting to ");
    if(WLAN_USE_SMARTCONFIG){
      Serial.print(F("wifi network"));
    }
    else{
      Serial.print(WLAN_SSID);
    }
    Serial.print(" took ");
    Serial.print((millis()-connectionTimeMS)/1000);
    Serial.println(" seconds");

    connectionTimeMS = millis();
    Serial.println("Connecting to BERGCloud...");
  }
  else{
    Serial.println("connect() returned false.");
  }

}

void loop()
{
  if(!is_connected()){
    // if not connected to BERGCloud, check the claim state for this device
    if (!is_claimed()){
      // if not claimed print the claim code to the Serial monitor
      Serial.println(F("Claiming state: Not claimed"));
      String claimcode;
      if (BERGCloud.getClaimcode(claimcode))
      {
        Serial.println(F("/////////////////////////////////////////////////////////////////////"));
        Serial.println(F("To complete connection visit http://bergcloud.com/devcenter/projects/"));
        Serial.print(F("using this claim code: "));
        Serial.println(claimcode);
        Serial.println(F("/////////////////////////////////////////////////////////////////////"));
      }
      else{
        Serial.println(F("getClaimcode() returned false."));
      }
    }

    Serial.println(F("Checking connection again in 5 seconds"));
    delay(5000);
    connectionTimeMS = millis();
  }
  else{
    // check the millis() against our pollGap
    if(millis()>=(pollTimer+pollGap)){

      receivedCommand = false;

      pollTimer=millis();

      BERGCloudMessage command, event;
      String text;
      int number;
      String commandName;

      ////////////////////////////////
      /// CHECKING FOR A COMMAND
      ////////////////////////////////
      
      // if there is a command waiting, get it and unpack the data in the payload.
      // in this example we also return an event when we get a command, as an echo.

      if (BERGCloud.pollForCommand(command, commandName)){
        Serial.println("Received command:\t"+commandName);

        receivedCommand = true;
        // Try to decode the two common types of serialized
        // data: An integer and a string
        // example payload [123, "Testing"]

        ///////////////////////////////////////
        /// UNPACKING THE COMMAND PAYLOAD
        ///////////////////////////////////////
        if (command.unpack(number))
        {
          Serial.print("Containing number:\t");
          Serial.println(number);
        }
        else{
          Serial.println(F("unpack(int) returned false."));
        }

        if (command.unpack(text))
        {
          Serial.print("Containing text:\t");
          Serial.println(text);
        }
        else{
          Serial.println(F("unpack(text) returned false."));
        }

        Serial.print(F("Returning an event...\t"));

        ////////////////////////////////
        /// SENDING AN EVENT
        ////////////////////////////////
        
        // pack some text into a message.
        event.pack("Hello!");
        // send that message to Berg
        if (BERGCloud.sendEvent("Echo", event)){
          Serial.println(F("ok"));
        }
        else{
          Serial.println(F("failed/busy"));
        }
      }

      ////////////////////////////////
      /// GENERAL STATUS REPORT
      ////////////////////////////////
      BERGCloud.getDeviceID(deviceID);
      Serial.print(F("Device: "));
      Serial.print(deviceID);
      Serial.print(F(" Up time = "));
      Serial.print((millis()-connectionTimeMS)/1000);

      if(!receivedCommand){
        Serial.println(F(" seconds, no new commands"));
      }
      else{
        Serial.println(F(" seconds"));
      }
    }
  }


  BERGCloud.loop();
}


////////////////////////////////////////////////
/// HELPER FUNCTION TO WRAP getConnectionState()
////////////////////////////////////////////////
boolean is_connected(){
  boolean connection = false;
  byte state;
  if(BERGCloud.getConnectionState(state)){
    switch(state){
    case BC_CONNECT_STATE_CONNECTED:
      //Serial.println("Connection state: Connected");
      connection = true;
      break;
    case BC_CONNECT_STATE_CONNECTING:
      Serial.println(F("Connection state: Connecting..."));
      break;
    case BC_CONNECT_STATE_DISCONNECTED:
      Serial.println(F("Connection state: Disconnected"));
      break;
    default:
      Serial.println(F("Connection state: Unknown!"));
      break;
    }
  }
  else{
    Serial.print(F("getting connection state failed"));
  }  
  return connection;
}

//////////////////////////////////////////////
/// HELPER FUNCTION TO WRAP getClaimingState()
//////////////////////////////////////////////
boolean is_claimed(){
  boolean claimed = false;
  byte state;
  // check the claiming state and use a switch to check the returned value.
  if (BERGCloud.getClaimingState(state)){
    switch(state){
    case BC_CLAIM_STATE_CLAIMED:
      //Serial.println("Claim State: Claimed");
      claimed = true;
      break;
    case BC_CLAIM_STATE_NOT_CLAIMED:
      //Serial.println("Claim State: Not Claimed");
      break;
    default:
      Serial.println(F("Claim State: Unknown!"));
      break;
    }
  }
  else{
    Serial.println(F("getClaimingState() returned false."));
  }
  return claimed;
}

////////////////////////////////////////////
/// HELPER FUNCTION TO WRAP resetClaimcode()
////////////////////////////////////////////
void checkForReclaimPin(int thePin){
  pinMode(thePin, INPUT_PULLUP);
  if(!digitalRead(thePin)){
    Serial.println(F("RESETTING CLAIMCODE"));
    BERGCloud.resetClaimcode();
  }
  // wait for 500 ms
  delay(500);
}




























