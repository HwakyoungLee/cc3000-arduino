/*

Berg constant definitions

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

/*
 * Sizes of things
 */

#define BC_MAC48_SIZE_BYTES             6
#define BC_EUI64_SIZE_BYTES             8
#define BC_DEVICE_ADDRESS_SIZE_BYTES    8
#define BC_CLAIMCODE_SIZE_BYTES         20
#define BC_CLAIMCODE_BASE32_SIZE_BYTES  16
#define BC_SECRET_SIZE_BYTES            8
#define BC_KEY_SIZE_BYTES               16
#define BC_COMMAND_HEADER_SIZE_BYTES    16

/*
 * NVRAM
 */

#define BC_EEPROM_SIZE_BYTES            (4*1024) // Arduino Mega 2560
#define BC_EEPROM_RESERVED_BYTES        256
#define BC_EEPROM_OFFSET                (BC_EEPROM_SIZE_BYTES - BC_EEPROM_RESERVED_BYTES)

/* 
 * Unused analogue in, used to seed random number generator
 */

#define BC_UNUSED_AIN                   0

/*
 *  Network
 */

/* Note: Hostname must be valid even if an IP address is used. */
/* For HOST_IP use IPAddress(a,b,c,d) or set to INADDR_NONE to use DNS */
#define BC_WEBSOCKET_HOST_NAME          "bridge.bergcloud.com"
#define BC_WEBSOCKET_HOST_IP            INADDR_NONE
#define BC_WEBSOCKET_PORT               80
#define BC_WEBSOCKET_PATH               "/api/v1/connection"
#define BC_WEBSOCKET_PROTOCOL           "bergcloud-bridge-v1"

/*
 * Network commands
 */

#define BC_EVENT_ANNOUNCE               0xA000
#define BC_COMMAND_SET_ADDRESS          0xB000

#define BC_COMMAND_START_RAW            0xC000
#define BC_COMMAND_START_PACKED         0xC100
#define BC_COMMAND_NAMED_PACKED         0xC17F
#define BC_COMMAND_ID_MASK              0x00FF
#define BC_COMMAND_FORMAT_MASK          0xFF00

#define BC_COMMAND_DISPLAY_IMAGE        0xD000
#define BC_COMMAND_DISPLAY_TEXT         0xD001

#define BC_EVENT_START_RAW              0xE000
#define BC_EVENT_START_PACKED           0xE100
#define BC_EVENT_NAMED_PACKED           0xE17F
#define BC_EVENT_ID_MASK                0x00FF
#define BC_EVENT_FORMAT_MASK            0xFF00

#define BC_COMMAND_FIRMWARE_ARDUINO     0xF010
#define BC_COMMAND_FIRMWARE_MBED        0xF020

/* Connection state */
#define BC_CONNECT_STATE_CONNECTED      0x00
#define BC_CONNECT_STATE_CONNECTING     0x01
#define BC_CONNECT_STATE_DISCONNECTED   0x02

/* Claiming state */
#define BC_CLAIM_STATE_CLAIMED          0x00
#define BC_CLAIM_STATE_NOT_CLAIMED      0x01
