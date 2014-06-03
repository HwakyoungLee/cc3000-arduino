/*

Simple message buffer

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

#include "BERGCloudMessageBuffer.h"
#include "string.h" // For memcpy()

BERGCloudMessageBuffer::BERGCloudMessageBuffer(uint16_t size)
{
  bufferSize = size;
  buffer = new uint8_t[bufferSize];
  
  if (buffer == 0)
  {
    bufferSize = 0;
  }
  
  clear();
}

BERGCloudMessageBuffer::BERGCloudMessageBuffer(const BERGCloudMessageBuffer& parent)
{
  bufferSize = parent.bufferSize;
  bytesWritten = parent.bytesWritten;
  bytesRead = parent.bytesRead;

  /* Create a new buffer */
  buffer = new uint8_t[bufferSize];
  
  if (buffer == 0)
  {
    bufferSize = 0;
  }
  
  /* Copy buffer contents */
  memcpy(buffer, parent.buffer, bufferSize);
}

BERGCloudMessageBuffer::~BERGCloudMessageBuffer(void)
{
  if (buffer != 0)
  {
    delete buffer;
  }
}

void BERGCloudMessageBuffer::clear(void)
{
  bytesWritten = 0; /* Number of bytes written */
  bytesRead = 0;    /* Number of bytes read */
}

void BERGCloudMessageBuffer::restart(void)
{
  /* Restart reading from the beginning */
  bytesRead = 0;
}

uint16_t BERGCloudMessageBuffer::size(void)
{
  /* Get total size of the buffer */
  return bufferSize;
}

uint16_t BERGCloudMessageBuffer::used(void)
{
  /* Get number of bytes used in the buffer */
  return bytesWritten;
}

void BERGCloudMessageBuffer::used(uint16_t used)
{
  /* Set number of bytes used in the buffer */
  bytesWritten = used;
}

uint16_t BERGCloudMessageBuffer::available(void)
{
  /* Get space available in the buffer */
  return bufferSize - bytesWritten;
}

bool BERGCloudMessageBuffer::available(uint16_t required)
{
  /* Test if space is available for the number of bytes required */
  return (bufferSize - bytesWritten) >= required;
}

void BERGCloudMessageBuffer::add(uint8_t data)
{
  /* Write a byte to the buffer; no checks */
    buffer[bytesWritten++] = data;
}

bool BERGCloudMessageBuffer::peek(uint8_t *data)
{
  /* Peek at the next byte in the buffer */
  if (bytesRead >= bytesWritten)
  {
    /* No more data */
    return false;
  }

  
  *data = buffer[bytesRead]; /* No increment */
  return true;
}

uint8_t BERGCloudMessageBuffer::read(void)
{
  /* Read the next byte from the buffer; no checks */
  return buffer[bytesRead++];
}

uint16_t BERGCloudMessageBuffer::remaining(void)
{
  /* Returns the number of bytes that have been written but not read */
  return bytesWritten - bytesRead;
}

bool BERGCloudMessageBuffer::remaining(uint16_t required)
{
  /* Test if data is remaining for the number of bytes required */
  return (bytesWritten - bytesRead) >= required;
}

uint8_t *BERGCloudMessageBuffer::ptr(void)
{
  return buffer;
}
