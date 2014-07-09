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

#define PROJECT_KEY "2d17dd4c6519f0a9ee741568db1f51f1"
#define VERSION     1

boolean previousConnectionState = false;
unsigned long pollTimer;
int pollGap = 1000;

void connectToBerg(){
  Serial.print(F("BERGCloudCC3000 version: "));
  Serial.println(BERGCLOUD_LIB_VERSION, HEX);

  BERGCloudWLANConfig WLANConfig;
  // if you're going to use smart config
  if(WLAN_USE_SMARTCONFIG){
    WLANConfig.smartConfigDeviceName = WLAN_DEVICENAME;
    WLANConfig.smartConfig = WLAN_USE_SMARTCONFIG;
    WLANConfig.smartConfigReconnect = WLAN_RECONNECT;
    Serial.println(F("Attempting to connect using Smart Config"));
  }
  else{
    // otherwise use the preset wifi configuration details from the other tab.
    WLANConfig.ssid = WLAN_SSID;
    WLANConfig.pass = WLAN_PASS;
    WLANConfig.secmode = WLAN_SEC;
    Serial.print(F("Attempting to connect to WiFi network "));
    Serial.println(WLAN_SSID);
  }

  // set up the wifi connection
  BERGCloud.begin(WLANConfig);
  // try to connect to Berg
  BERGCloud.connect(PROJECT_KEY, VERSION);
}

void loopBerg(){

  if(is_connected()){
    // if we're connected  
    if(!previousConnectionState){
      // if this is a new connection
      previousConnectionState = true;
      Serial.println(F("Connection to Berg established."));
      Serial.print(F("DeviceID = "));
      String device_id;
      BERGCloud.getDeviceID(device_id);
      Serial.println(device_id);
    }

    // check for a new command every 1000 milliseconds.
    if(millis()>=(pollTimer+pollGap)){
      pollTimer = millis();

      BERGCloudMessage command;
      String commandName;

      if (BERGCloud.pollForCommand(command, commandName)){
        // we have a command so pass it out to commandReceived()
        commandReceived(commandName, command);
      }
    }
  }

  BERGCloud.loop();
}

void sendEventToBerg(String &name, boolean state){
  BERGCloudMessage m;
  m.pack(state);
  BERGCloud.sendEvent(name, m);
}

void sendEventToBerg(String &name, BERGCloudMessage &m){
  BERGCloud.sendEvent(name, m);
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
      connection = true;
      break;
    case BC_CONNECT_STATE_CONNECTING:
      // check the claiming state
      if(!is_claimed()){
        String claimcode;
        if (BERGCloud.getClaimcode(claimcode))
        {
          Serial.println(F("//////////////////////////////////////////////////////////////"));
          Serial.println(F("To complete connection visit http://getconnected.bergcloud.com"));
          Serial.println(F("and claim your device using this claim code: "));
          Serial.println(claimcode);
          Serial.println(F("////////////////////////////// checking again in 5 seconds ///"));
          delay(5000);
        }
      }
      break;
    case BC_CONNECT_STATE_DISCONNECTED:
      previousConnectionState = false;
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
      claimed = true;
      break;
    case BC_CLAIM_STATE_NOT_CLAIMED:
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













