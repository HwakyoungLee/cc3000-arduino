/*

BERGCloud CC3000 common API

Copyright (c) 2013 Berg Cloud Limited http://bergcloud.com/

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


#ifndef BERGCLOUDBASE_H
#define BERGCLOUDBASE_H

#include "BERGCloudConfig.h"
#include "BERGCloudConst.h"
#include "BERGCloudLogPrint.h"

#ifdef BERGCLOUD_PACK_UNPACK
#include "BERGCloudMessageBuffer.h"
#endif

#define BERGCLOUD_LIB_VERSION (0x0202)
#define PROJECT_KEY_CHANGE_REQUIRES_RECLAIM

typedef struct {
  bool available;
  uint8_t *data;
  uint32_t size;
  uint32_t id;
} COMMAND_TYPE;

typedef struct {
  uint8_t version;
  uint8_t reserved0;
  uint16_t size;
  uint8_t isClaimed;
  uint8_t reserved1;
  uint8_t reserved2;
  uint8_t reserved3;
  uint8_t secret[BC_SECRET_SIZE_BYTES];
  char project[BC_KEY_SIZE_BYTES * 2]; /* Stored as text */
  uint8_t eui64[BC_EUI64_SIZE_BYTES];
  uint16_t crc;
} BC_NVRAM;

class BERGCloudBase
{
public:
  /* Check for a command */
  bool pollForCommand(uint8_t *commandBuffer, uint16_t commandBufferSize, uint16_t& commandSize, char *commandName, uint8_t commandNameMaxSize);
#ifdef BERGCLOUD_PACK_UNPACK
  bool pollForCommand(BERGCloudMessageBuffer& buffer, char *commandName, uint8_t commandNameMaxSize);
#endif
  /* Send an event */
  bool sendEvent(const char *eventName, uint8_t *eventBuffer, uint16_t eventSize, bool packed = true);
#ifdef BERGCLOUD_PACK_UNPACK
  bool sendEvent(const char *eventName, BERGCloudMessageBuffer& buffer);
#endif
  /* Get the connection state */
  bool getConnectionState(uint8_t& state);
  /* Get the claiming state */
  bool getClaimingState(uint8_t& state);
  /* Connect */
  bool connect(const char *key, uint16_t version);
  /* Get the current claimcode */
  virtual bool getClaimcode(char (&claimcode)[BC_CLAIMCODE_SIZE_BYTES], boolean hyphens = true);
  /* Generate a new claimcode */
  bool resetClaimcode(void);
  /* Get the Device Address */
  virtual bool getDeviceAddress(uint8_t (&address)[BC_DEVICE_ADDRESS_SIZE_BYTES]);
  /* NULL project key */
  static uint8_t nullKey[BC_KEY_SIZE_BYTES];

  virtual void begin(void);
  virtual void end(void);
  virtual void loop(void);
  
  /* Internal methods & variables */
protected:
  uint16_t Crc16(uint8_t data, uint16_t crc);
  virtual void timerReset(void) = 0;
  virtual uint32_t timerRead_mS(void) = 0;
  virtual bool connectToNetwork(void) = 0;
  virtual bool nvRamRead(uint8_t *data, uint8_t size) = 0;
  virtual bool nvRamWrite(uint8_t *data, uint8_t size) = 0;
  virtual bool sendDeviceEvent(uint8_t *header, uint16_t headerSize, uint8_t *data, uint16_t dataSize) =0;
  virtual bool pollForDeviceCommand(void) =0;
  virtual bool sendDeviceCommandResponse(uint32_t command_id, uint8_t returnCode) =0;
  bool deviceAddressUpdated(void);
  bool reconnect(void);
  void eventDisconnected(void);
  void eventConnected(void);
  const char *_key;
  uint16_t _version;
  uint8_t deviceAddress[BC_DEVICE_ADDRESS_SIZE_BYTES];
  uint8_t hardwareAddress[BC_EUI64_SIZE_BYTES];
  COMMAND_TYPE command;
private:
  bool resetNVData(void);
  bool readNVData(void);
  bool updateNVData(void);
  char toClaimcodeChar(uint8_t n);
  bool _sendEvent(uint16_t eventCode, uint8_t *eventBuffer, uint16_t eventSize);
  void bytecpy(uint8_t *dst, uint8_t *src, uint16_t size);
  virtual bool sendConnectEvent(void) = 0;
  BC_NVRAM nvram;
  bool connected;
  bool receivedDeviceAddress;
};

#endif // #ifndef BERGCLOUDBASE_H
