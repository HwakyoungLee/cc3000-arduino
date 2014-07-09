/*

 An example for connecting to BERG Cloud using the CC3000 shield and Smart Config.
 
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

// This example describes how to connect use Smart Config with a key with the Arduino Mega and Adafruit CC3000 shield.

// set WLAN_USE_SMARTCONFIG to true to use Smart Config.
#define WLAN_USE_SMARTCONFIG          true
// if set to true WLAN_RECONNECT will cause the CC3000 to try and use previously saved Smart Config data.
// if set to false you will have to configure it each time the Arduino resets.
// because we want to test the Smart Config process we'll set it to false
#define WLAN_RECONNECT                false
// These can be set to increase the security of the data transfer between the Smart Config app and your device during configuration.
#define WLAN_SMARTCONFIG_DEVICE_NAME  "CC3000"
#define WLAN_SMARTCONFIG_KEY          "abcdefghijklmnop"

// These values would need to be edited to reflect your Project setup on http://bergcloud.com/devcenter/
// However, it is not important for this example.
#define PROJECT_KEY "00000000000000000000000000000000"
#define VERSION     1

unsigned long connectionTimeMS;

void setup()
{
  // open a serial connection over US
  Serial.begin(115200);
  Serial.println(F("--- reset ---"));
  Serial.print(F("BERGCloudCC3000 version: "));
  Serial.println(BERGCLOUD_LIB_VERSION, HEX);

  BERGCloudWLANConfig WLANConfig;
  WLANConfig.smartConfigReconnect = WLAN_RECONNECT;
  WLANConfig.smartConfig = WLAN_USE_SMARTCONFIG;
  WLANConfig.smartConfigDeviceName = WLAN_SMARTCONFIG_DEVICE_NAME;
  WLANConfig.smartConfigKey = WLAN_SMARTCONFIG_KEY;
  BERGCloud.begin(WLANConfig);

  connectionTimeMS = millis();
  if (BERGCloud.connect(PROJECT_KEY, VERSION))
  {
    Serial.print("Connecting took ");
    Serial.print((millis()-connectionTimeMS)/1000);
    Serial.println(" seconds.");
    Serial.println("That's all for this example.");
  }
  else{
    Serial.println("connect() returned false.");
  }

}

void loop(){
}

