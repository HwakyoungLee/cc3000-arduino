// This sketch is for use with the http://bergcloud.com/devcenter/tutorials/wifi-to-website example

#include <EEPROM.h>
#include <WebSocketClient.h>
#include <aJSON.h>
#include <Base64.h>
#include <SPI.h>
#include <Adafruit_CC3000.h>
#include <BERGCloudCC3000.h>

#define WLAN_USE_SMARTCONFIG          true

#define WLAN_SSID                     "my network"
#define WLAN_PASS                     "my password"
#define WLAN_SEC                      WLAN_SEC_WPA2
#define WLAN_RECONNECT                true
#define WLAN_DEVICENAME               "CC3000"

// Variables for button control //
boolean buttonPressed = false;
String eventName = "button";
#define LED                           48
#define BUTTON                        49

void setup(){
  // start a serial connection over USB
  Serial.begin(115200);
  Serial.println("--- reset ---");
  // assign a pin for an led
  pinMode(LED, OUTPUT);
  // assigne a pin for a button
  pinMode(BUTTON, INPUT_PULLUP);
  
  connectToBerg();
}

void loop(){
  // continually check the wifi connection
  loopBerg();
  
  if(is_connected()){
    // if we're connected send any button presses on pin BUTTON up to Berg
    checkForButtonPress();
  }
}

void commandReceived(String &name, BERGCloudMessage &theMessage){
  // expects a string
  String text;
  theMessage.unpack(text);
  // check to see if the string is one we're looking for...
  if(text=="led-on"){
    digitalWrite(LED, HIGH);
  }
  else if(text=="led-off"){
    digitalWrite(LED, LOW);
  }
  else{
    // if it's a different string to those above print it to the Serial Monitor
    Serial.print(F("Command contents = "));
    Serial.println(text);
  }
}

void checkForButtonPress(){
  boolean currentState = digitalRead(BUTTON);
  if(!currentState){
    if(!buttonPressed){
      Serial.print("Sending ");
      Serial.println(true);
      sendEventToBerg(eventName, true);
      buttonPressed=true;
    }
  }
  else{
    if(buttonPressed){
      Serial.print("Sending ");
      Serial.println(false);
      sendEventToBerg(eventName, false);
      buttonPressed=false;
    }
  }
}









