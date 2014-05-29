/*

An example using the Adafruit SPI OLED module alongside the CC3000 shield.

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
#include <Arduino.h>
#include <EEPROM.h>
#include <WebSocketClient.h>
#include <aJSON.h>
#include <Base64.h>
#include <SPI.h>
#include <Adafruit_CC3000.h>
#include <BERGCloudCC3000.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

//
// Display configuration
//

#if (SSD1306_LCDHEIGHT != 32)
#error("Height incorrect, please fix Adafruit_SSD1306.h!");
#endif

// Pins may be changed to suit your setup
#define OLED_MOSI  A8
#define OLED_CLK   A9
#define OLED_DC    A10
#define OLED_CS    A11
#define OLED_RESET A12

//
// WiFi network configuration
//

// Uncomment the next line to use Smart Config.
// #define SMARTCONFIG

#ifdef SMARTCONFIG

// Smart Config
#define WLAN_SMARTCONFIG_DEVICE_NAME  "CC3000"
#define WLAN_SMARTCONFIG_KEY          NULL
#define WLAN_SMARTCONFIG_RECONNECT    false

#else

// Manual configuration
#define WLAN_SSID                     "NetworkName"
#define WLAN_PASS                     "NetworkPassword"
#define WLAN_SEC                      WLAN_SEC_WPA2

#endif

//
// These values should be edited to reflect your Project setup on bergcloud.com
//

#define PROJECT_KEY "00000000000000000000000000000000"
#define VERSION 1

Adafruit_SSD1306 display(OLED_MOSI, OLED_CLK, OLED_DC, OLED_RESET, OLED_CS);

void setup()
{
  BERGCloudWLANConfig WLANConfig;

#ifdef SMARTCONFIG
  WLANConfig.smartConfig = true;
  WLANConfig.smartConfigDeviceName = WLAN_SMARTCONFIG_DEVICE_NAME;
  WLANConfig.smartConfigKey = WLAN_SMARTCONFIG_KEY;
  WLANConfig.smartConfigReconnect = WLAN_SMARTCONFIG_RECONNECT;
#else
  WLANConfig.smartConfig = false;
  WLANConfig.ssid = WLAN_SSID;
  WLANConfig.pass = WLAN_PASS;
  WLANConfig.secmode = WLAN_SEC;
#endif

  /* Initialise serial for logging */
  Serial.begin(115200);
  Serial.println("--- reset ---");

  /* Initialise Berg WLAN */
  BERGCloud.begin(WLANConfig);

  /* Initialise display */
  display.begin(SSD1306_SWITCHCAPVCC);

  /* Update the status display */
  displayStatus();

  /* Connect to the network */
  BERGCloud.connect(PROJECT_KEY, VERSION);
}

void loop()
{
  /* Update Berg state */
  BERGCloud.loop();

  /* Update the status display */
  displayStatus();

  delay(1000);
}

void displayStatus(void)
{
  byte connectionState;
  byte claimingState;
  String claimcode;

  /* Clear the display */
  display.clearDisplay();

  /* Set text size, color and position */
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0,0);

  /* Get the connection state */
  if (BERGCloud.getConnectionState(connectionState))
  {
    switch(connectionState)
    {
      case BC_CONNECT_STATE_CONNECTED:
        display.println("Connected");
        break;
      case BC_CONNECT_STATE_CONNECTING:
          /* Connecting; check if the device has been claimed. */
          if (BERGCloud.getClaimingState(claimingState))
          {
            switch(claimingState)
            {
              case BC_CLAIM_STATE_CLAIMED:
                display.println("Connecting...");
                break;
              case BC_CLAIM_STATE_NOT_CLAIMED:
                  /* Not yet claimed so display the claimcode. */
                  if (BERGCloud.getClaimcode(claimcode))
                  {
                    display.println("Claim code: ");
                    display.println(claimcode);
                  }
                  break;
              default:
              break;
            }
          }
          break;
      case BC_CONNECT_STATE_DISCONNECTED:
        display.println("Disconnected");
        break;
      default:
        break;
    }
  }

  /* Write to the display */
  display.display();
}
