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
#include "Storage.h"

#include <FS.h>

bool Storage::open() {
  return SPIFFS.begin();
}

void Storage::close() {
  SPIFFS.end();
}

bool Storage::format() {
  return SPIFFS.format();
}

bool Storage::exists(const char* file) {
  return SPIFFS.exists(file);
}

bool Storage::remove(const char* file){
  return SPIFFS.remove(file);
}

size_t Storage::writeFile(const char* file, String data) {
  return writeFile(file, (uint8_t*)data.c_str(), data.length());
}

size_t Storage::writeFile(const char* file, uint8_t* data, size_t length) {
  int16_t size = -1;
  File f = SPIFFS.open(file, "w");
  if (f) {
    size = f.write(data, length);
  f.flush();
    f.close();
  }
  return size;
}

String Storage::readFile(const char* file, size_t* read) {
  String s = "";
  *read = 0;
  File f = SPIFFS.open(file, "r");
  if (f) {
    while (f.available()) {
      s += (char) f.read();
      ++*read;
    }
    f.close();
  }
  return s;
}

size_t Storage::readFile(const char* file, uint8_t* data, size_t length) {
  memset(data, 0, length+1);
  size_t read = 0;
  File f = SPIFFS.open(file, "r");
  if (f) {
    read = f.readBytes((char*)data, length);
    f.close();
  }
  return read;
}

/**
int16_t Storage::size(const char* file) {
  int16_t size = -1;
  File f = SPIFFS.open(file, "r");
  if (f) {
    size = f.size();
    f.close();
  }
  return size;
}
*/

