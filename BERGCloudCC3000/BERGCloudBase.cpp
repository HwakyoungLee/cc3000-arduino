/*

BERGCloud Cloud library common API

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

#define __STDC_LIMIT_MACROS /* Include C99 stdint defines in C++ code */
#include <stdint.h>
#include <stddef.h>
#include <string.h> /* For memcpy() */

#include "BERGCloudBase.h"

/* MessagePack for named commands and events */
#define _MP_FIXRAW_MIN      0xa0
#define _MP_FIXRAW_MAX      0xbf
#define _MAX_FIXRAW         (_MP_FIXRAW_MAX - _MP_FIXRAW_MIN)

uint8_t BERGCloudBase::nullKey[BC_KEY_SIZE_BYTES] = {0};

bool BERGCloudBase::pollForCommand(uint8_t *commandBuffer, uint16_t commandBufferSize, uint16_t& commandSize, char *commandName, uint8_t commandNameMaxSize)
{
  /* Returns TRUE if a valid command has been received */
  uint8_t commandNameSize;
  uint8_t originalCommandNameSize;
  uint8_t msgPackByte;
  uint16_t cmd;
  bool result = false;

  if ((commandName == NULL) || (commandNameMaxSize < 2))
  {
    return false;
  }

  /* Defaults */
  *commandName = '\0';
  commandSize = 0;

  if (command.available)
  {
    command.available = false;
    commandSize = command.size - BC_COMMAND_HEADER_SIZE_BYTES;

    if (commandBufferSize >= commandSize)
    {
      /* Get command */
      cmd = command.data[3];
      cmd <<= 8;
      cmd |= command.data[2];
      
      if (cmd == BC_COMMAND_NAMED_PACKED)
      {
        /* Copy command data */
        memcpy(commandBuffer, &command.data[BC_COMMAND_HEADER_SIZE_BYTES], commandSize);
              
        /* Get command name string size */
        msgPackByte = *commandBuffer;

        /* Check for valid command name size */
        if ((msgPackByte >= _MP_FIXRAW_MIN) && (msgPackByte <= _MP_FIXRAW_MAX))
        {
          commandNameSize = originalCommandNameSize = msgPackByte - _MP_FIXRAW_MIN;

          /* Limit to the size of the buffer provided */
          if (commandNameSize > (commandNameMaxSize-1)) /* -1 for null terminator */
          {
            commandNameSize = (commandNameMaxSize-1);
          }

          /* Copy command name string as a null-terminated C string */
          bytecpy((uint8_t *)commandName, (commandBuffer+1), commandNameSize); /* +1 for messagePack fixraw byte */
          commandName[commandNameSize] = '\0';

          /* Move up remaining packed data, update size */
          commandSize -= (originalCommandNameSize + 1); /* +1 for messagePack fixraw byte */
          bytecpy(commandBuffer, commandBuffer + (originalCommandNameSize + 1), commandSize);

          /* Success */
          result = true;
        }
      }
    }

    /* Send response */
    sendDeviceCommandResponse(command.id, result ? 0x00 : 0xff);

    /* Free command buffer */
    free(command.data);
    command.data = NULL;
  }

  return result;
}

#ifdef BERGCLOUD_PACK_UNPACK
bool BERGCloudBase::pollForCommand(BERGCloudMessageBuffer& buffer, char *commandName, uint8_t commandNameMaxSize)
{
  /* Returns TRUE if a valid command has been received */

  uint16_t dataSize = 0;
  uint8_t commandNameSize;
  uint16_t originalCommandNameSize;
  uint8_t msgPackByte;
  uint16_t cmd;
  uint16_t commandSize;
  bool result = false;
  
  if ((commandName == NULL) || (commandNameMaxSize < 2))
  {
    return false;
  }

  /* Defaults */
  buffer.clear();
  *commandName = '\0';

  if (command.available)
  {
    command.available = false;
    commandSize = command.size - BC_COMMAND_HEADER_SIZE_BYTES;
    dataSize = commandSize;
    
    if (buffer.size() >= commandSize)
    {
      /* Get command */
      cmd = command.data[3];
      cmd <<= 8;
      cmd |= command.data[2];
      
      if (cmd == BC_COMMAND_NAMED_PACKED)
      {
        /* Copy command data */
        memcpy(buffer.ptr(), &command.data[BC_COMMAND_HEADER_SIZE_BYTES], commandSize);

        /* Get command name string size */
        msgPackByte = *buffer.ptr();

        /* Check for valid command name size */
        if ((msgPackByte >= _MP_FIXRAW_MIN) && (msgPackByte <= _MP_FIXRAW_MAX))
        {
          commandNameSize = originalCommandNameSize = msgPackByte - _MP_FIXRAW_MIN;

          /* Limit to the size of the buffer provided */
          if (commandNameSize > (commandNameMaxSize-1)) /* -1 for null terminator */
          {
            commandNameSize = (commandNameMaxSize-1);
          }

          /* Copy command name string as a null-terminated C string */
          bytecpy((uint8_t *)commandName, (buffer.ptr()+1), commandNameSize); /* +1 for messagePack fixraw byte */
          commandName[commandNameSize] = '\0';

          /* Move up remaining packed data, update size */
          dataSize -= (originalCommandNameSize + 1); /* +1 for messagePack fixraw byte */
          bytecpy(buffer.ptr(), buffer.ptr() + (originalCommandNameSize + 1), dataSize);

          buffer.used(dataSize);

          /* Success */
          result =  true;
        }
      }
    }

    /* Send response */
    sendDeviceCommandResponse(command.id, result ? 0x00 : 0xff);

    /* Free command buffer */
    free(command.data);
    command.data = NULL;
  }

  return result;
}
#endif

bool BERGCloudBase::_sendEvent(uint16_t eventCode, uint8_t *eventBuffer, uint16_t eventSize)
{
  /* Returns TRUE if the event is sent successfully */
  uint32_t commandInvocationId;
  uint32_t eventPayloadLength;
    
  /* Create header */
  uint8_t header[sizeof(eventCode) + sizeof(commandInvocationId) + sizeof(eventPayloadLength)];

  commandInvocationId = 0;
  eventPayloadLength = eventSize;

  header[0] = eventCode;
  header[1] = eventCode >> 8;
  header[2] = commandInvocationId; 
  header[3] = commandInvocationId >> 8;
  header[4] = commandInvocationId >> 16;
  header[5] = commandInvocationId >> 24;
  header[6] = eventPayloadLength;
  header[7] = eventPayloadLength >> 8;
  header[8] = eventPayloadLength >> 16;
  header[9] = eventPayloadLength >> 24;
   
  return sendDeviceEvent(header, sizeof(header), eventBuffer, eventSize);
}

bool BERGCloudBase::sendEvent(const char *eventName, uint8_t *eventBuffer, uint16_t eventSize, bool packed)
{
  /* Returns TRUE if the event is sent successfully */

  uint8_t *temp;
  uint16_t tempUsed = 0;
  bool result;

  if (!packed)
  {
    /* We only support packed data now */
    return false;
  }

  if ((eventName == NULL) || (eventName[0] == '\0'))
  {
    _LOG("Event name must be at least one character.");
    return false;
  }
  
  if (strlen(eventName) > _MAX_FIXRAW)
  {
    _LOG("Event name is too long.");
    return false;
  }
    
  /* Create temporary buffer for event name and event data */
  temp = new uint8_t[1 + _MAX_FIXRAW + eventSize]; /* 1 for MessagePack byte */

  /* Create string header in messagePack format */
  temp[0] = _MP_FIXRAW_MIN;
  tempUsed++;
  
  while (*eventName != '\0')
  {
    /* Copy string, update messagePack byte */
    temp[0]++;
    temp[tempUsed++] = *eventName++;
  }

  /* Copy event data */
  memcpy(&temp[tempUsed], eventBuffer, eventSize);
  tempUsed += eventSize;
  
  result = _sendEvent(BC_EVENT_NAMED_PACKED, temp, tempUsed);

  /* Free temporary buffer */
  delete temp;
  
  return result;
}  

#ifdef BERGCLOUD_PACK_UNPACK
bool BERGCloudBase::sendEvent(const char *eventName, BERGCloudMessageBuffer& buffer)
{
  /* Returns TRUE if the event is sent successfully */
  return sendEvent(eventName, buffer.ptr(), buffer.used(), true /* Packed */);
}
#endif

bool BERGCloudBase::getConnectionState(uint8_t& state)
{
  if (connected)
  {
    if (receivedDeviceAddress)
    {
      state = BC_CONNECT_STATE_CONNECTED;
    }
    else
    {
      state = BC_CONNECT_STATE_CONNECTING;
    }
  }
  else
  {
    state = BC_CONNECT_STATE_DISCONNECTED;
  }

  return true;
}

bool BERGCloudBase::getClaimingState(uint8_t& state)
{
  state = nvram.isClaimed ? BC_CLAIM_STATE_CLAIMED : BC_CLAIM_STATE_NOT_CLAIMED;
  return true;
}

bool BERGCloudBase::readNVData(void)
{
  /* Read and validate non-volatile storage */
  uint16_t calc_crc = 0xffff;
  uint8_t i;
  uint8_t *pNvram = (uint8_t *)&nvram;

  /* Fetch data */
  if (!nvRamRead(pNvram, sizeof(nvram)))
  {
    return false;
  }

  /* Check size */
  if (nvram.size != sizeof(nvram))
  {
    _LOG("NVRAM size invalid.");
    return false;
  }
  
  /* Calculate CRC16 */
  for (i=0; i < (sizeof(nvram) - sizeof(nvram.crc)); i++)
  {
    calc_crc = Crc16(pNvram[i], calc_crc);
  }
  
  /* Validate CRC */
  if (nvram.crc != calc_crc)
  {
    _LOG("NVRAM CRC invalid.");
    return false;
  }
  
  /* Check hardware address (EUI64) matches */
  if (memcmp(nvram.eui64, hardwareAddress, sizeof(nvram.eui64)) != 0)
  {
    _LOG("EUI64 has changed.");
    return false;
  }  
  
  /* Check project key matches */  
  if (_key == NULL)
  {
    _LOG("Invalid project key.");
    return false;
  }

  if (strlen(_key) != sizeof(nvram.project))
  {
    _LOG("Invalid project key.");
    return false;
  }
        
  if (memcmp(nvram.project, _key, sizeof(nvram.project)) != 0)
  {
    _LOG("Project key has changed.");

#ifdef PROJECT_KEY_CHANGE_REQUIRES_RECLAIM
    return false;
#else
    memcpy(nvram.project, _key, sizeof(nvram.project));
    updateNVData();
#endif

  }

  /* Success */
  return true;
}

bool BERGCloudBase::resetClaimcode(void)
{
  return resetNVData();
}

bool BERGCloudBase::updateNVData(void)
{
  uint16_t calc_crc = 0xffff;
  uint8_t *pNvram = (uint8_t *)&nvram;
  uint8_t i;
  
  /* Calculate CRC16 */
  for (i=0; i < (sizeof(nvram) - sizeof(nvram.crc)); i++)
  {
    calc_crc = Crc16(pNvram[i], calc_crc);
  }
  
  /* Add CRC */
  nvram.crc = calc_crc;
  
  /* Store */
  return nvRamWrite(pNvram, sizeof(nvram)); 
}

bool BERGCloudBase::deviceAddressUpdated(void)
{
  /* Connected */
  receivedDeviceAddress = true;

  /* If not already claimed, mark as claimed */
  if (!nvram.isClaimed)
  {
    nvram.isClaimed = true;
    
    /* Store */
    return updateNVData();
  }
  
  /* Already claimed */
  return true;
}

bool BERGCloudBase::resetNVData(void)
{
  uint8_t *pNvram = (uint8_t *)&nvram;
  uint8_t i;

  /* Zero data */
  memset(pNvram, 0x00, sizeof(nvram));
  
  /* Set version */
  nvram.version = 1;
  
  /* Set size */
  nvram.size = sizeof(nvram);
    
  /* Generate random secret */
  randomSeed(analogRead(BC_UNUSED_AIN));
  
  /* Set secret */
  for (i = 0; i< sizeof(nvram.secret); i++)
  {
    nvram.secret[i] = random(0, 256);
  }

  /* Set project key */
  if (_key != NULL)
  {
    if (strlen(_key) == sizeof(nvram.project))
    {
      memcpy(nvram.project, _key, sizeof(nvram.project));
    }
  }

  /* Set EUI64 */
  memcpy(nvram.eui64, hardwareAddress, sizeof(nvram.eui64));

  /* Store */
  return updateNVData();
}

bool BERGCloudBase::connect(const char *key, uint16_t version)
{
  /* Copy key string pointer and version number */
  _key = key;
  _version = version;
  
  return reconnect();
}  
  
bool BERGCloudBase::reconnect(void)
{
  /* Clear any pending command */
  if (command.data != NULL)
  {
      free(command.data);
  }
  memset(&command, 0x00, sizeof(command));

  if (!connectToNetwork())
  {
    return false;
  }

  /* Read non-volatile data */
  if (!readNVData())
  {
    /* Stored data was invalid */
    resetNVData();

    /* Retry */
    if (!readNVData())
    {
      _LOG("NVRAM fault.\r\n");
      return false;
    }
  }

  return sendConnectEvent();
}

void BERGCloudBase::eventConnected(void)
{
  connected = true;
}

void BERGCloudBase::eventDisconnected(void)
{
  connected = false;
  receivedDeviceAddress = false;
}

char base32[32] = {
  '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'b', 'c', 'd',
  'e', 'f', 'g', 'h', 'j', 'k', 'm', 'n', 'o', 'p', 'q',
  'r', 's', 't', 'v', 'w', 'x', 'y', 'z'
};

#define CLAIMCODE_DATA_BYTES 10
#define CLAIMCODE_SIZE_BASE32 16 

char BERGCloudBase::toClaimcodeChar(uint8_t n)
{
  if (n < sizeof(base32))
  {
    return base32[n];
  }
  
  /* Invalid */
  return '!';
}

bool BERGCloudBase::getClaimcode(char (&claimcode)[BC_CLAIMCODE_SIZE_BYTES], boolean hyphens)
{
  uint16_t crc = 0xffff;
  uint8_t tmp[CLAIMCODE_SIZE_BASE32];
  uint8_t in_idx, in_bit, out_idx, out_bit;
  uint8_t data[sizeof(nvram.secret) + sizeof(crc)];
  bool isNullClaimcode = true;

  /* Create data with crc */
  for (in_idx=0; in_idx<sizeof(nvram.secret); in_idx++)
  {
    if (nvram.secret[in_idx] != 0)
    {
      isNullClaimcode = false;
    }
    data[in_idx] = nvram.secret[in_idx];
    crc = Crc16(data[in_idx], crc);
  }

  if (isNullClaimcode)
  {
    /* Zero claimcode and return false */
    memset(claimcode, 0x00, sizeof(claimcode));
    return false;
  }

  data[sizeof(nvram.secret)] = crc;
  data[sizeof(nvram.secret)+1] = crc >> 8;

  /* Convert to base32 in tmp */
  out_idx = 0;
  out_bit = 0;

  memset(tmp, 0x00, sizeof(tmp));

  for (in_idx=0; in_idx < sizeof(data); in_idx++)
  {
    for (in_bit=0; in_bit < 8; in_bit++)  /* Eight bits in byte */
    {
      if (data[in_idx] & (1<<in_bit))
      {
        tmp[out_idx] |= (1<<out_bit);
      }

      out_bit++;

      if (out_bit == 5) /* Five bits for each base32 character */
      {
        out_bit = 0;
        out_idx++;
      }
    }
  }
  
  /* Convert to ASCII characters */
  
  if (hyphens)
  {
    claimcode[ 0] = toClaimcodeChar(tmp[15]);
    claimcode[ 1] = toClaimcodeChar(tmp[14]);
    claimcode[ 2] = toClaimcodeChar(tmp[13]);
    claimcode[ 3] = toClaimcodeChar(tmp[12]);
    claimcode[ 4] = '-';
    claimcode[ 5] = toClaimcodeChar(tmp[11]);
    claimcode[ 6] = toClaimcodeChar(tmp[10]);
    claimcode[ 7] = toClaimcodeChar(tmp[9]);
    claimcode[ 8] = toClaimcodeChar(tmp[8]);
    claimcode[ 9] = '-';
    claimcode[10] = toClaimcodeChar(tmp[7]);
    claimcode[11] = toClaimcodeChar(tmp[6]);
    claimcode[12] = toClaimcodeChar(tmp[5]);
    claimcode[13] = toClaimcodeChar(tmp[4]);
    claimcode[14] = '-';
    claimcode[15] = toClaimcodeChar(tmp[3]);
    claimcode[16] = toClaimcodeChar(tmp[2]);
    claimcode[17] = toClaimcodeChar(tmp[1]);
    claimcode[18] = toClaimcodeChar(tmp[0]);
    claimcode[19] = '\0';
  }
  else
  {
    claimcode[ 0] = toClaimcodeChar(tmp[15]);
    claimcode[ 1] = toClaimcodeChar(tmp[14]);
    claimcode[ 2] = toClaimcodeChar(tmp[13]);
    claimcode[ 3] = toClaimcodeChar(tmp[12]);
    claimcode[ 4] = toClaimcodeChar(tmp[11]);
    claimcode[ 5] = toClaimcodeChar(tmp[10]);
    claimcode[ 6] = toClaimcodeChar(tmp[9]);
    claimcode[ 7] = toClaimcodeChar(tmp[8]);
    claimcode[ 8] = toClaimcodeChar(tmp[7]);
    claimcode[ 9] = toClaimcodeChar(tmp[6]);
    claimcode[10] = toClaimcodeChar(tmp[5]);
    claimcode[11] = toClaimcodeChar(tmp[4]);
    claimcode[12] = toClaimcodeChar(tmp[3]);
    claimcode[13] = toClaimcodeChar(tmp[2]);
    claimcode[14] = toClaimcodeChar(tmp[1]);
    claimcode[15] = toClaimcodeChar(tmp[0]);
    claimcode[16] = '\0';
    claimcode[17] = '\0';
    claimcode[18] = '\0';
    claimcode[19] = '\0';
  }  
  
  return true;
}

bool BERGCloudBase::getDeviceAddress(uint8_t (&address)[BC_DEVICE_ADDRESS_SIZE_BYTES])
{
  memcpy(address, deviceAddress, BC_DEVICE_ADDRESS_SIZE_BYTES);
  return true;
}

uint16_t BERGCloudBase::Crc16(uint8_t data, uint16_t crc)
{
  /* CRC16 CCITT (0x1021) */

  uint8_t s;
  uint16_t t;

  s = data ^ (crc >> 8);
  t = s ^ (s >> 4);
  return (crc << 8) ^ t ^ (t << 5) ^ (t << 12);
}

void BERGCloudBase::begin(void)
{
  _key = NULL;
  _version = 0;
  connected = false;
  receivedDeviceAddress = false;
  memset((uint8_t *)&nvram, 0x00, sizeof(nvram));
  memset(deviceAddress, 0x00, sizeof(deviceAddress));
  memset(hardwareAddress, 0x00, sizeof(hardwareAddress));
  memset(&command, 0x00, sizeof(command));
}

void BERGCloudBase::end(void)
{
}

void BERGCloudBase::loop(void)
{
}

void BERGCloudBase::bytecpy(uint8_t *dst, uint8_t *src, uint16_t size)
{
  /* memcpy() cannot be used when buffers overlap */
  while (size-- > 0)
  {
    *dst++ = *src++;
  }
}
