/*

A client class for CC3000 that inherits from Arduino's Client class.

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

#include "Adafruit_CC3000.h"
#include "Client.h"

// To avoid making many single byte writes we buffer them and write
// only when the buffer is full or flush_tx() is called.
#define TX_BUFFER_SIZE_BYTES 64

// For use with CC3000 SEND_NON_BLOCKING option:
// #define WRITE_ATTEMPTS_100MS 10

class CC3000Client : public Client
{
  public:
    CC3000Client();
    virtual int connect(IPAddress ip, uint16_t port);
    virtual int connect(const char *host, uint16_t port);
    virtual size_t write(uint8_t );
    virtual size_t write(const uint8_t *buf, size_t size);
    virtual int available();
    virtual int read();
    virtual int read(uint8_t *buf, size_t size);
    virtual int peek();
    virtual void flush();
    virtual void stop();
    virtual uint8_t connected();
    virtual operator bool();
    Adafruit_CC3000_Client client;
  protected:
    void flush_tx();
    size_t retry_write(const uint8_t *buffer, size_t size);
#ifdef TX_BUFFER_SIZE_BYTES
    uint8_t tx_buffer[TX_BUFFER_SIZE_BYTES];
    uint8_t buffer_used;
#endif
};
