/**
  MIT License

  Copyright (c) 2017 Boodskap Inc

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
  SOFTWARE.
*/
#include <Arduino.h>

#ifndef _STORAGE_H_
#define _STORAGE_H_

#define BSKP_CONFIG_FILE "/config.json"
#define DEVICE_CONFIG_FILE "/device.json"

class Storage {

  public:
    virtual bool open();
    virtual void close();
    virtual bool format();
    virtual bool exists(const char* file);
    virtual bool remove(const char* file);
    virtual size_t writeFile(const char* file, String data);
    virtual size_t writeFile(const char* file, uint8_t* data, size_t length);
    virtual String readFile(const char* file, size_t* read);
    virtual size_t readFile(const char* file, uint8_t* data, size_t length);
    //virtual int16_t size(const char* file);
};

#endif //_STORAGE_H_

