/*

BERGCloud library for Arduino (TI CC3000 WLAN)

Copyright (c) 2014 Berg Cloud Limited http://bergcloud.com/

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
#include "BERGCloudBase.h"
#include "CC3000Client.h"
#include "WebSocketClient.h"
#include "Base64.h"
#include "aJSON.h"

#ifdef BERGCLOUD_PACK_UNPACK
#include "BERGCloudMessageBase.h"
#endif

// These are the interrupt and control pins
#define ADAFRUIT_CC3000_IRQ   3  // MUST be an interrupt pin!
// These can be any two pins
#define ADAFRUIT_CC3000_VBAT  5
#define ADAFRUIT_CC3000_CS    10
// Use hardware SPI for the remaining pins
// On an UNO, SCK = 13, MISO = 12, and MOSI = 11

#define DHCP_TIMEOUT_100MS   300 // 30 Seconds
#define DNS_RESOLVE_ATTEMPTS 10
#define SMARTCONFIG_ATTEMPTS 10

//#define JSON_DEBUG_PRINT

class BERGCloudWLANConfig 
{
public:
BERGCloudWLANConfig()
{
  ssid = NULL;
  pass = NULL;
  smartConfigDeviceName = NULL;
  smartConfigKey = NULL;
  secmode = WLAN_SEC_WPA2;
  smartConfig = false;
  smartConfigReconnect = false;
}
public:
  const char *ssid;
  const char *pass;
  const char *smartConfigDeviceName;
  const char *smartConfigKey;
  uint8_t secmode;
  bool smartConfig;
  bool smartConfigReconnect;
};

class BERGCloudCC3000 : public BERGCloudBase
{
public:
  using BERGCloudBase::getClaimcode;
  virtual bool getClaimcode(String& claimcode, boolean hyphens = true);
  using BERGCloudBase::getDeviceAddress;
  bool getDeviceAddress(String &address);
#ifdef BERGCLOUD_PACK_UNPACK
  using BERGCloudBase::pollForCommand;
  bool pollForCommand(BERGCloudMessageBuffer& buffer, String &commandName);
  using BERGCloudBase::sendEvent;
  bool sendEvent(String& eventName, BERGCloudMessageBuffer& buffer);
#endif
  void begin(BERGCloudWLANConfig& WLANConfig);
  void begin(void);
  virtual void loop(void);
protected:
  bool sendJSON(aJsonObject* root);
  bool receiveJSON(aJsonObject** obj);
  virtual bool sendDeviceEvent(uint8_t *header, uint16_t headerSize, uint8_t *data, uint16_t dataSize);
  virtual bool pollForDeviceCommand(void);
  virtual bool sendDeviceCommandResponse(uint32_t command_id, uint8_t returnCode);
  bool connectToNetwork(void);
  virtual bool nvRamRead(uint8_t *data, uint8_t size);
  virtual bool nvRamWrite(uint8_t *data, uint8_t size);
  bool isNonZero(uint8_t *data, uint8_t dataSize);
  void MAC48toEUI64(uint8_t *mac48, uint8_t *eui64);
  WebSocketClient webSocket;
private:
  void timerReset(void);
  uint32_t timerRead_mS(void);
  bool resetNVData(void);
  bool readNVData(void);
  bool updateNVData(void);
  char toClaimcodeChar(uint8_t n);
  bool _sendEvent(uint16_t eventCode, uint8_t *eventBuffer, uint16_t eventSize);
  void bytecpy(uint8_t *dst, uint8_t *src, uint16_t size);
  virtual bool sendConnectEvent(void);
  virtual uint8_t randomByte(void);
  void arrayToString(String& string, uint8_t *array, uint8_t items);
  uint8_t getResetSource(void);
  Adafruit_CC3000 *cc3000;
  CC3000Client wlan;
  BERGCloudWLANConfig _WLANConfig;
  BC_NVRAM nvram;
  uint32_t resetTime;
  uint8_t numberOfPings;
};

#ifdef BERGCLOUD_PACK_UNPACK

class BERGCloudMessage : public BERGCloudMessageBase
{
  public:
  BERGCloudMessage()
  {
  }
  BERGCloudMessage(uint16_t size)
    : BERGCloudMessageBase(size)
  {
  }
  using BERGCloudMessageBase::pack;
  using BERGCloudMessageBase::unpack;
  /* Pack a 4-byte double */
  bool pack(double& n);
  /* Methods using Arduino string class */
  bool pack(String& s);
  bool unpack(String& s);
  /* Methods using Arduino boolean type */
  bool pack_boolean(boolean n);
  bool unpack_boolean(boolean &n);
};

#endif // #ifdef BERGCLOUD_PACK_UNPACK

extern BERGCloudCC3000 BERGCloud;