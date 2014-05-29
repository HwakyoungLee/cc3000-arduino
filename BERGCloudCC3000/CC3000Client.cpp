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

#define __STDC_LIMIT_MACROS /* Include C99 stdint defines in C++ code */
#include <stdint.h>
#include <stddef.h>

#include "CC3000Client.h"

CC3000Client::CC3000Client(void)
{
#ifdef TX_BUFFER_SIZE_BYTES
  buffer_used = 0;
#endif
}

int CC3000Client::connect(IPAddress ip, uint16_t port)
{
  // Not implemented
  return false;
}

int CC3000Client::connect(const char *host, uint16_t port)
{
  // Not implemented
  return false;
}

size_t CC3000Client::write(uint8_t a)
{
#ifdef TX_BUFFER_SIZE_BYTES
  if (buffer_used < TX_BUFFER_SIZE_BYTES)
  {
    tx_buffer[buffer_used++] = a;
    
    if (buffer_used == TX_BUFFER_SIZE_BYTES)
    {
      /* Send */
      flush_tx();
    }
    
    /* Success */
    return 1;
  }
  
  /* Failed */
  return 0;
  
#else
  return retry_write((const uint8_t *)&a, 1);
#endif
}

size_t CC3000Client::write(const uint8_t *buf, size_t size)
{
  if (size > UINT16_MAX)
  {
    return 0;
  }

#ifdef TX_BUFFER_SIZE_BYTES
  flush_tx();
#endif

  return retry_write(buf, (uint16_t)size);
}

int CC3000Client::available()
{
#ifdef TX_BUFFER_SIZE_BYTES
  flush_tx();
#endif

  return client.available();
}

int CC3000Client::read()
{
  if (client.available() == 0)
  {
    return -1;
  }
  
  return client.read();  
}

int CC3000Client::read(uint8_t *buf, size_t size)
{
  if (size > UINT16_MAX)
  {
    return -1;
  }

  if (client.available() == 0)
  {
    return -1;
  }
    
  return client.read(buf, (uint16_t)size, 0);
}

int CC3000Client::peek()
{
  // Not implemented
  return 0;
}

void CC3000Client::flush_tx()
{
#ifdef TX_BUFFER_SIZE_BYTES
  if (buffer_used > 0)
  {
    retry_write(tx_buffer, buffer_used);
    buffer_used = 0;
  }
#endif
}

size_t CC3000Client::retry_write(const uint8_t *buffer, size_t size)
{
  int sent;

#ifdef WRITE_ATTEMPTS_100MS
  /* For use with CC3000 SEND_NON_BLOCKING option */
  uint8_t attempts = WRITE_ATTEMPTS_100MS;

  while (attempts-- > 0)
  {
    sent = client.write(buffer, size);
    
    if (sent >= 0)
    {
      /* Success */
      return sent;
    }
    
    delay(100); /* 100mS */
  }
  
  /* Failed */
  Serial.println("BERGCloud: Timed out writing to CC3000.");

#else
  sent = client.write(buffer, size);
  if (sent == -3) /* See SEND_TIMEOUT_MS is socket.cpp */
  {
    Serial.println("BERGCloud: Timed out writing to CC3000.");
  }
#endif

  return sent;
}

void CC3000Client::flush()
{
  // Rx flush. Not implemented
}

void CC3000Client::stop()
{
#ifdef TX_BUFFER_SIZE_BYTES
  flush_tx();
#endif
  client.close();
}

uint8_t CC3000Client::connected()
{
  return client.connected();
}

CC3000Client::operator bool()
{
  return client.connected();
}
