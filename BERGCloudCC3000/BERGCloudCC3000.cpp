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

#include "BERGCloudCC3000.h"
#include "utility/socket.h" // For mdnsAdvertiser()

BERGCloudCC3000 BERGCloud;

void BERGCloudCC3000::begin(BERGCloudWLANConfig& WLANConfig)
{
  /* Store WLan config */
  _WLANConfig = WLANConfig;
  
  begin();
}

void BERGCloudCC3000::begin(void)
{
  /* Seed PRNG */
  randomSeed(analogRead(BC_UNUSED_AIN));
  /* Call parent class method */
  BERGCloudBase::begin();
}

bool BERGCloudCC3000::connectToNetwork(void)
{
  uint32_t host_ip;
  uint16_t dhcpTimeout = DHCP_TIMEOUT_100MS;
  bool result;
  uint8_t smartConfigAttempts = 0;
  uint8_t major = 0, minor = 0;
  uint8_t MACAddress[6];
  uint32_t ipAddress, netmask, gateway, dhcpserv, dnsserv;
  uint8_t dnsAttempts;
  
  _LOG("Connecting to WiFi network...");
  
  if (_WLANConfig.smartConfig)
  {
    _LOG("Using SmartConfig.");
  }
       
  /* Connect to WLan */
  cc3000 = new Adafruit_CC3000(ADAFRUIT_CC3000_CS, ADAFRUIT_CC3000_IRQ, ADAFRUIT_CC3000_VBAT, SPI_CLOCK_DIVIDER);
  
  if (!cc3000->begin(0, _WLANConfig.smartConfig && _WLANConfig.smartConfigReconnect))
  {
    /* Don't return here - begin() can return false if useSmartConfigData is true */
  }

  if(!cc3000->getFirmwareVersion(&major, &minor))
  {
     return false; 
  }

#ifdef BERGCLOUD_LOG
  Serial.print(F("BERGCloud: CC3000 firmware version "));
  Serial.print(major, DEC);
  Serial.print(F("."));
  Serial.println(minor, DEC);
#endif

  if (!cc3000->getMacAddress(MACAddress))
  {
    return false;
  }

#ifdef BERGCLOUD_LOG
  Serial.print(F("BERGCloud: CC3000 MAC address "));
  Serial.print(MACAddress[0], HEX);
  Serial.print(F(":"));
  Serial.print(MACAddress[1], HEX);
  Serial.print(F(":"));
  Serial.print(MACAddress[2], HEX);
  Serial.print(F(":"));
  Serial.print(MACAddress[3], HEX);
  Serial.print(F(":"));
  Serial.print(MACAddress[4], HEX);
  Serial.print(F(":"));
  Serial.println(MACAddress[5], HEX);
#endif

  if (!isNonZero(MACAddress, sizeof(MACAddress)))
  {
    return false;
  }

  /* Create EUI64 */
  MAC48toEUI64(MACAddress, hardwareAddress);

  if (!cc3000->checkConnected())
  {
    /* We have not reconnected using a stored profile */
    if (_WLANConfig.smartConfig)
    {
      /* Attempt to connect using Smart Config */
      do {
        result = cc3000->startSmartConfig(_WLANConfig.smartConfigDeviceName, _WLANConfig.smartConfigKey);
        } while (!result && (++smartConfigAttempts < SMARTCONFIG_ATTEMPTS));
      
        if (!result)
        {
          return false; 
        }
    }
    else
    {
      /* Attempt to connect using the supplied SSID and password */
      if (!cc3000->connectToAP(_WLANConfig.ssid, _WLANConfig.pass, _WLANConfig.secmode))
      {
        return false;
      }
    }
  }

  /* Wait for DHCP */
  _LOG("Waiting for DHCP...");
  while (!cc3000->checkDHCP())
  {
    delay(100);
      
    if (--dhcpTimeout == 0)
    {
      _LOG("Timeout during DHCP.");
      return false;
    }
  }

  if(!cc3000->getIPAddress(&ipAddress, &netmask, &gateway, &dhcpserv, &dnsserv))
  {
    _LOG("Unable to get DHCP settings from CC3000.");
    return false;
  }
  else
  {
#ifdef BERGCLOUD_LOG
    Serial.print(F("BERGCloud: IP address:  ")); cc3000->printIPdotsRev(ipAddress);
    Serial.print(F("\nBERGCloud: Netmask:     ")); cc3000->printIPdotsRev(netmask);
    Serial.print(F("\nBERGCloud: Gateway:     ")); cc3000->printIPdotsRev(gateway);
    Serial.print(F("\nBERGCloud: DHCP server: ")); cc3000->printIPdotsRev(dhcpserv);
    Serial.print(F("\nBERGCloud: DNS server:  ")); cc3000->printIPdotsRev(dnsserv);
    Serial.println();
#endif
  }

  /* Possible workaround for MDNS / UDP issues. See also retries of DNS lookup below. */
  /* Ref: http://e2e.ti.com/support/wireless_connectivity/f/851/t/342177.aspx */
  cc3000->getHostByName((char *)"localhost", &host_ip);

  if (_WLANConfig.smartConfig)
  {
    /* Complete Smart Config */
    mdnsAdvertiser(1, (char *) _WLANConfig.smartConfigDeviceName, strlen(_WLANConfig.smartConfigDeviceName));
  }

  
  if ((IPAddress)BC_WEBSOCKET_HOST_IP == INADDR_NONE)
  {
    /* Use DNS */
    host_ip = 0;
    dnsAttempts = DNS_RESOLVE_ATTEMPTS;

#ifdef BERGCLOUD_LOG
    Serial.print(F("BERGCloud: Looking up host: "));
    Serial.println(BC_WEBSOCKET_HOST_NAME);
#endif

    do {

      if (!cc3000->getHostByName((char *)BC_WEBSOCKET_HOST_NAME, &host_ip))
      {
        _LOG("Can't resolve host IP address.");
        return false;
      }

      if (host_ip == 0)
      {
        delay(100);
      }

      /* Retry if an invalid IP address is returned */
    } while ((host_ip == 0) && (--dnsAttempts > 0));


    if (host_ip == 0)
    {
      /* Failed */
      _LOG("Can't resolve host IP address.");
      return false;
    }
  }
  else
  {
    /* Use IP address */
    host_ip  = BC_WEBSOCKET_HOST_IP[0];
    host_ip <<= 8;
    host_ip |= BC_WEBSOCKET_HOST_IP[1];
    host_ip <<= 8;
    host_ip |= BC_WEBSOCKET_HOST_IP[2];
    host_ip <<= 8;
    host_ip |= BC_WEBSOCKET_HOST_IP[3];
  }

#ifdef BERGCLOUD_LOG
  Serial.print(F("BERGCloud: Host IP address: ")); cc3000->printIPdotsRev(host_ip);
  Serial.println();
#endif

  wlan.client = cc3000->connectTCP(host_ip, BC_WEBSOCKET_PORT);
  
  /* Create websocket client */
  webSocket.host = (char *)BC_WEBSOCKET_HOST_NAME;
  webSocket.path = (char *)BC_WEBSOCKET_PATH;
  webSocket.protocol = (char *)BC_WEBSOCKET_PROTOCOL;
  
  if (!webSocket.handshake(wlan))
  {
    _LOG("Unable to establish a WebSocket connection.");
    return false;
  }

  /* Connected */
  eventConnected();

  /* Reset WebSocket 'ping' count & periodic timer - see loop() */
  numberOfPings = 0;
  timerReset();
  
  return true; 
}

bool BERGCloudCC3000::sendJSON(aJsonObject* root)
{
  /* Caller must delete aJson root object after use. */

  char *tx_data = aJson.print(root);
  webSocket.sendData((const char *)tx_data);
  free(tx_data);
  
  return true;
}

bool BERGCloudCC3000::receiveJSON(aJsonObject **obj)
{
  /* Caller must delete aJson root object after use. */
  
  String rxData;
  uint8_t opcode;
  bool result;

  /* Process incoming data */
  do {
    result = webSocket.getData(rxData, &opcode);
    
    if (result)
    {
      if (opcode == WS_OPCODE_PING)
      {
        webSocket.sendData(rxData, WS_OPCODE_PONG);
        numberOfPings++;
      }
      
      if (opcode == WS_OPCODE_TEXT)
      {
        /* JSON data */
        *obj = aJson.parse((char *)rxData.c_str());
        return true;
      }
    }
  } while (result == true);
  
  /* No JSON data */
  return false;
}

bool BERGCloudCC3000::isNonZero(uint8_t *data, uint8_t dataSize)
{
  while (dataSize-- > 0)
  {
    if (*data++ != 0)
    {
      /* Not all zeros */
      return true;
    }
  }
  
  /* All zeros */
  return false;
}

void BERGCloudCC3000::MAC48toEUI64(uint8_t *mac48, uint8_t *eui64)
{
  /* Create an EUI64 from MAC48 as per RFC2373 */
  
  eui64[0] = mac48[0] ^ 2; /* Invert universal/local bit */
  eui64[1] = mac48[1];
  eui64[2] = mac48[2];
  eui64[3] = 0xff;
  eui64[4] = 0xfe;
  eui64[5] = mac48[3];
  eui64[6] = mac48[4];
  eui64[7] = mac48[5];
}

char hexTable[16] = {'0','1','2','3','4','5','6','7','8','9','a','b','c','d','e','f'};

void BERGCloudCC3000::arrayToString(String& string, uint8_t *array, uint8_t items)
{
  uint8_t index = 0;
  string = "";
  while (items-- > 0)
  {
    string += hexTable[array[index] >> 4];
    string += hexTable[array[index] & 0xf];
    index++;
  }
}

bool BERGCloudCC3000::sendConnectEvent(void)
{
  bool result = false;
  String addressString;
  String secretString;
  
  arrayToString(addressString, hardwareAddress, sizeof(hardwareAddress));
  getClaimcode(secretString, false /* No hyphens */);
  
  if (_key == NULL)
  {
    return false;
  }
  
  aJsonObject* root = aJson.createObject();
  if (root == NULL)
  {
    return false;
  }

  aJson.addItemToObject(root, "type", aJson.createItem("WifiEvent"));
  aJson.addItemToObject(root, "name", aJson.createItem("connect"));
  aJson.addItemToObject(root, "client_hardware_type", aJson.createItem("CC3000"));
  aJson.addItemToObject(root, "client_library_version", aJson.createItem((uint32_t)BERGCLOUD_LIB_VERSION));
  aJson.addItemToObject(root, "hardware_address", aJson.createItem(addressString.c_str()));
  aJson.addItemToObject(root, "reset_description_code", aJson.createItem((uint32_t)getResetSource()));
  aJson.addItemToObject(root, "developer_project_key", aJson.createItem(_key));
  aJson.addItemToObject(root, "developer_version", aJson.createItem((uint32_t)_version));
  aJson.addItemToObject(root, "secret", aJson.createItem(secretString.c_str()));

  result = sendJSON(root);
  aJson.deleteItem(root);

  return result;
}

bool BERGCloudCC3000::sendDeviceEvent(uint8_t *header, uint16_t headerSize, uint8_t *data, uint16_t dataSize)
{
  bool result = false;
  uint8_t binaryData[headerSize + dataSize];
  int8_t encodedData[base64_enc_len(headerSize) + base64_enc_len(dataSize) + 1]; /* +1 for null terminator */
  String addressString;
  uint8_t state;

  if (!getConnectionState(state))
  {
    return false;
  }

  if (state == BC_CONNECT_STATE_CONNECTING)
  {
    /* Not yet claimed */
    return false;
  }

  if (state == BC_CONNECT_STATE_DISCONNECTED)
  {
    /* Start to reconnect; can't block here */
    reconnect();
    return false;
  }

  /* Copy header and data */
  memcpy(&binaryData[0], header, headerSize);
  memcpy(&binaryData[headerSize], data, dataSize);
  
  /* Base64 encode - uses the base64 library included with the WebSockets library */
  base64_encode((char *)encodedData, (char *)binaryData, sizeof(binaryData));
  
  aJsonObject* root = aJson.createObject();
  if (root == NULL)
  {
    return false;
  }
  
  arrayToString(addressString, hardwareAddress, sizeof(hardwareAddress));

  aJson.addItemToObject(root, "type", aJson.createItem("DeviceEvent"));
  aJson.addItemToObject(root, "bridge_address", aJson.createItem(addressString.c_str()));
  aJson.addItemToObject(root, "device_address", aJson.createItem(addressString.c_str()));
  aJson.addItemToObject(root, "binary_payload", aJson.createItem((char *)encodedData));
  aJson.addItemToObject(root, "timestamp", aJson.createItem((uint32_t)0));
  
  result = sendJSON(root);
  
  aJson.deleteItem(root);
  
  return result;
}

void BERGCloudCC3000::loop(void)
{
  uint8_t state;
  
  if (!getConnectionState(state))
  {
    return;
  }
  
  if (state != BC_CONNECT_STATE_DISCONNECTED)
  {
    /* Update the connected state */
    if (!cc3000->checkConnected())
    {
      eventDisconnected();
      _LOG("Disconnected from WLAN");
    }
  
    if (!wlan.connected())
    {
      eventDisconnected();
      _LOG("Disconnected (Socket closed)");
      return;
    }

    /* Check periodic timer events */
    if (timerRead_mS() > ((uint32_t)1000 * 60))
    {
      /* > one minute tick */

      /* Check WebSocket pings */
      if (numberOfPings == 0)
      {
        eventDisconnected();
        _LOG("Disconnected (No WebSocket pings)");
        return;
      }

      timerReset();
      numberOfPings = 0;
    }
  }
  
  /* Check for incoming commands */
  pollForDeviceCommand();
  
  BERGCloudBase::loop();
}

bool BERGCloudCC3000::pollForDeviceCommand(void)
{
  uint8_t *binaryData;
  int binaryDataSize;
  uint16_t cmd = 0;
  uint8_t i;
  aJsonObject *root = NULL;
  uint8_t state;

  if (!getConnectionState(state))
  {
    return false;
  }

  if (state == BC_CONNECT_STATE_DISCONNECTED)
  {
    /* Start to reconnect; can't block here */
    reconnect();
    return false;
  }

  if (!receiveJSON(&root))
  {
    return false;
  }
  
  if (root == NULL)
  {
    return false;
  }
  
  #ifdef JSON_DEBUG_PRINT
  /* Print JSON */
  char *text = aJson.print(root);
  if (text != NULL)
  {
    Serial.println(F("Command JSON:"));
    Serial.println(text);
    free(text);
  }
  #endif
  
  /* Check message type */
  aJsonObject* type = aJson.getObjectItem(root, "type");
  
  if (type == NULL)
  {
    aJson.deleteItem(root);
    return false;
  }
  
  if (strcmp((const char *)type->valuestring, "DeviceCommand") != 0)
  {
    aJson.deleteItem(root);
    return false;
  }
  
  /* Get payload */
  aJsonObject* payload = aJson.getObjectItem(root, "binary_payload");
  
  if (payload == NULL)
  {
    aJson.deleteItem(root);
    return false;
  }
  
  /* Get command_id */
  aJsonObject* command_id = aJson.getObjectItem(root, "command_id");
  
  if (command_id == NULL)
  {
    aJson.deleteItem(root);
    return false;
  }

  /* Remove spurious '\n' */
  char *in, *out;
  in = out = payload->valuestring;

  do {
    if (*in != '\n')
    {
      *out++ = *in;
    }
    
  } while (*in++ != '\0');
  
  /* Get decoded size & allocate memory */
  binaryDataSize = base64_dec_len(payload->valuestring, strlen(payload->valuestring));
  
  if (binaryDataSize < BC_COMMAND_HEADER_SIZE_BYTES)
  {
    aJson.deleteItem(root);
    return false;
  }
  
  binaryData = (uint8_t *)malloc(binaryDataSize);
  
  if (binaryData == NULL)
  {
    aJson.deleteItem(root);
    return false;
  }
  
  /* Decode from Base64 */
  base64_decode((char *)binaryData, payload->valuestring, strlen(payload->valuestring));
  
  /* Get command */
  cmd = binaryData[3];
  cmd <<= 8;
  cmd |= binaryData[2];

  /* Must be claimed */
  if (state == BC_CONNECT_STATE_CONNECTED)
  {
    if ( ((cmd & BC_COMMAND_FORMAT_MASK) == BC_COMMAND_START_RAW) || ((cmd & BC_COMMAND_FORMAT_MASK) == BC_COMMAND_START_PACKED) )
    {
		

	  /* command.data should be NULL at this point, check: */
	  if (command.data != NULL)
	  {
	    free(command.data);
	  }	
	      /* Command received */
      command.available = true;
      command.id = command_id->valueint;
      command.data = binaryData;
      command.size = (uint32_t)binaryDataSize;
      // Command processing code must free(command.data)

      aJson.deleteItem(root);
      return true;
    }
  }

  if (cmd == BC_COMMAND_SET_ADDRESS)
  {
    /* Validate data size */
    if ((unsigned long)binaryDataSize >=  (BC_COMMAND_HEADER_SIZE_BYTES + sizeof(deviceID)))
    {
      /* Copy address - change endian */
      for (i=0; i<sizeof(deviceID); i++)
      {
        deviceID[i] = binaryData[BC_COMMAND_HEADER_SIZE_BYTES + sizeof(deviceID) - i - 1];
      }
      
      free(binaryData);
      
      
      if (isNonZero(deviceID, sizeof(deviceID)))
      {
        /* Update connect and claiming states */
        deviceIDUpdated();
        
        /* Send response - success */
        sendDeviceCommandResponse(command_id->valueint, 0);
      }
      else
      {
        /* Send response - failed */
        sendDeviceCommandResponse(command_id->valueint, 0xff);
      }
      
      aJson.deleteItem(root);
      return true;
    }
  }

  /* Send response - failed */
  sendDeviceCommandResponse(command_id->valueint, 0xff);

  free(binaryData);
  
  aJson.deleteItem(root);
  return false;
}

bool BERGCloudCC3000::sendDeviceCommandResponse(uint32_t command_id, uint8_t returnCode)
{
  bool result = false;
  String addressString;

  arrayToString(addressString, hardwareAddress, sizeof(hardwareAddress));
  
  aJsonObject* root = aJson.createObject();
  if (root == NULL)
  {
    return false;
  }

  aJson.addItemToObject(root, "type", aJson.createItem("DeviceCommandResponse"));
  aJson.addItemToObject(root, "bridge_address", aJson.createItem(addressString.c_str()));
  aJson.addItemToObject(root, "device_address", aJson.createItem(addressString.c_str()));
  aJson.addItemToObject(root, "command_id", aJson.createItem(command_id));
  aJson.addItemToObject(root, "return_code", aJson.createItem((uint32_t)returnCode));
  aJson.addItemToObject(root, "timestamp", aJson.createItem((uint32_t)0));
  
  result = sendJSON(root);
  
  aJson.deleteItem(root);
  
  return result;
}

bool BERGCloudCC3000::nvRamRead(uint8_t *data, uint8_t size)
{
  uint8_t i;
  
  for (i=0; i<size; i++)
  {
    data[i] = EEPROM.read(BC_EEPROM_OFFSET + i);
  }
  
  return true;
}

bool BERGCloudCC3000::nvRamWrite(uint8_t *data, uint8_t size)
{
  uint8_t i;
  
  for (i=0; i<size; i++)
  {
    EEPROM.write(BC_EEPROM_OFFSET + i, data[i]);
  }
  
  return true;
}

void BERGCloudCC3000::timerReset(void)
{
  resetTime = millis();
}

uint32_t BERGCloudCC3000::timerRead_mS(void)
{
  return millis() - resetTime;
}

bool BERGCloudCC3000::getClaimcode(String& claimcode, boolean hyphens)
{
  char cc[BC_CLAIMCODE_SIZE_BYTES];
  
  if (!getClaimcode(cc, hyphens))
  {
    return false;
  }
  
  claimcode = String(cc);
  return true;
}

bool BERGCloudCC3000::getDeviceID(String &address)
{
  uint8_t adr[BC_DEVICE_ID_SIZE_BYTES];
  
  if (!getDeviceID(adr))
  {
    return false;
  }
  
  arrayToString(address, adr, BC_DEVICE_ID_SIZE_BYTES);
  return true;
}

uint8_t BERGCloudCC3000::getResetSource(void)
{
  uint8_t s = MCUSR;
  /* Clear */
  MCUSR = 0;

  return s;
}

uint8_t BERGCloudCC3000::randomByte(void)
{
  return random(0, 256);
}

#ifdef BERGCLOUD_PACK_UNPACK

bool BERGCloudMessage::pack(double& n)
{
  /* For 16-bit Arduino platforms we can treat a double literal */
  /* value as as float; this is so pack(1.234) will work. */

  if (sizeof(double) == sizeof(float))
  {
    return pack((float)n);
  }

  _LOG("Pack: 8-byte double type is not supported.\r\n");
  return false;
}

bool BERGCloudMessage::pack(String& s)
{
  uint16_t strLen = s.length();
  uint32_t i = 0;

  /* Add header */
  if (!pack_raw_header(strLen))
  {
    return false;
  }

  /* Add data */
  while (strLen-- > 0)
  {
    add((uint8_t)s.charAt(i++));
  }

  return true;
}

bool BERGCloudMessage::unpack(String& s)
{
  uint16_t sizeInBytes;

  if (!unpack_raw_header(&sizeInBytes))
  {
    return false;
  }

  if (!remaining(sizeInBytes))
  {
    _LOG_UNPACK_ERROR_NO_DATA;
    return false;
  }

  s = ""; /* Empty string */
  while (sizeInBytes-- > 0)
  {
    s += (char)read();
  }

  return true;
}

bool BERGCloudMessage::pack_boolean(boolean n)
{
  return pack((bool)n);
}

bool BERGCloudMessage::unpack_boolean(boolean &n)
{
  bool a;
  if (!unpack(a))
  {
    return false;
  }

  n = (boolean)a;
  return true;
}

bool BERGCloudCC3000::pollForCommand(BERGCloudMessageBuffer& buffer, String &commandName)
{
  bool result = false;
  char tmp[31 + 1]; /* +1 for null terminator */

  commandName = ""; /* Empty string */
  result = pollForCommand(buffer, (char *)tmp, (uint8_t)sizeof(tmp));

  if (result)
  {
    commandName = String(tmp);
  }

  return result;
}

bool BERGCloudCC3000::sendEvent(String& eventName, BERGCloudMessageBuffer& buffer)
{
  uint8_t temp[eventName.length() + 1]; /* +1 for null terminator */
  eventName.getBytes(temp, sizeof(temp));
  return sendEvent((const char *)temp, buffer);
}

#endif // #ifdef BERGCLOUD_PACK_UNPACK
