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

//
// WiFi network configuration
//

#define WLAN_SSID                     "NetworkName"
#define WLAN_PASS                     "NetworkPassword"

#define WLAN_SEC                      WLAN_SEC_WPA2
#define WLAN_RECONNECT                false
#define WLAN_USE_SMARTCONFIG          false
#define WLAN_SMARTCONFIG_DEVICE_NAME  "DeviceName"
#define WLAN_SMARTCONFIG_PASSWORD     "DevicePassword"

// These values should be edited to reflect your Project setup on http://bergcloud.com/devcenter/
#define PROJECT_KEY "00000000000000000000000000000000"
#define VERSION     1

unsigned long pollTimer;
int pollGap = 1000;
unsigned long connectionTimeMS;
String deviceAddress;
boolean receivedCommand;

void setup()
{
  Serial.begin(115200);
  Serial.println("--- reset ---");

  BERGCloudWLANConfig WLANConfig;
  WLANConfig.ssid = WLAN_SSID;
  WLANConfig.pass = WLAN_PASS;
  WLANConfig.secmode = WLAN_SEC;
  WLANConfig.smartConfigReconnect = WLAN_RECONNECT;
  WLANConfig.smartConfig = WLAN_USE_SMARTCONFIG;
  WLANConfig.smartConfigDeviceName = WLAN_SMARTCONFIG_DEVICE_NAME;
  WLANConfig.smartConfigKey = WLAN_SMARTCONFIG_PASSWORD;
  BERGCloud.begin(WLANConfig);

  // To reset the claimcode of the device make a 
  // connection between pin 50 and GND during powerup
  checkForReclaimPin(50);

  Serial.print("Uses WLAN_SSID ");
  Serial.println(WLAN_SSID);
  Serial.print("Command/Event poll = ");
  Serial.print(pollGap);
  Serial.println("ms");

  connectionTimeMS = millis();
  if (BERGCloud.connect(PROJECT_KEY, VERSION))
  {
    Serial.print("Connecting to ");
    Serial.print(WLAN_SSID);
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
      Serial.println("Claiming state: Not claimed");
      String claimcode;
      if (BERGCloud.getClaimcode(claimcode))
      {
        Serial.println(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>");
        Serial.println("To complete connection visit http://bergcloud.com/devcenter/projects/");
        Serial.println("and claim your device under 'List and claim devices' in your project");
        Serial.print("using this claim code: ");
        Serial.println(claimcode);
        Serial.println("<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<");
      }
      else{
        Serial.println("getClaimcode() returned false.");
      }
    }

    Serial.println("Checking connection again in 5 seconds");
    delay(5000);
    connectionTimeMS = millis();
  }
  else{
    // check the millis() against our pollGap
    if(millis()>=(pollTimer+pollGap)){

      receivedCommand = false;

      pollTimer=millis();

      String address;

      BERGCloudMessage command, event;
      String text;
      int number;
      String commandName;

      ////////////////////////////////
      /// CHECKING FOR A COMMAND
      ////////////////////////////////

      if (BERGCloud.pollForCommand(command, commandName)){
        Serial.println("Recieved command:\t"+commandName);

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
          Serial.println("unpack(int) returned false.");
        }

        if (command.unpack(text))
        {
          Serial.print("Containing text:\t");
          Serial.println(text);
        }
        else{
          Serial.println("unpack(text) returned false.");
        }

        Serial.print("Returning an event...\t");

        ////////////////////////////////
        /// SENDING AN EVENT
        ////////////////////////////////
        event.pack("Hello!");

        if (BERGCloud.sendEvent("Echo", event)){
          Serial.println("ok");
        }
        else{
          Serial.println("failed/busy");
        }
      }

      ////////////////////////////////
      /// GENERAL STATUS REPORT
      ////////////////////////////////
      BERGCloud.getDeviceAddress(deviceAddress);
      Serial.print("Device: ");
      Serial.print(deviceAddress);
      Serial.print(" Up time = ");
      Serial.print((millis()-connectionTimeMS)/1000);

      if(!receivedCommand){
        Serial.println(" seconds, no new commands");
      }
      else{
        Serial.println(" seconds");
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
      Serial.println("Connection state: Connecting...");
      break;
    case BC_CONNECT_STATE_DISCONNECTED:
      Serial.println("Connection state: Disconnected");
      break;
    default:
      Serial.println("Connection state: Unknown!");
      break;
    }
  }
  else{
    Serial.print("getting connection state failed");
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
      Serial.println("Claim State: Unknown!");
      break;
    }
  }
  else{
    Serial.println("getClaimingState() returned false.");
  }
  return claimed;
}

////////////////////////////////////////////
/// HELPER FUNCTION TO WRAP resetClaimcode()
////////////////////////////////////////////
void checkForReclaimPin(int thePin){
  pinMode(thePin, INPUT_PULLUP);
  if(!digitalRead(thePin)){
    Serial.println("RESETTING CLAIMCODE");
    BERGCloud.resetClaimcode();
  }
}





































