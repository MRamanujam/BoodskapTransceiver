#ifndef _BOODSKAP_COMMUNICATOR
#define _BOODSKAP_COMMUNICATOR

#include <ArduinoJson.h>
#include "BoodskapTransceiver.h"

class BoodskapCommunicator {
  public:
    virtual void setup() = 0;
    virtual void loop() = 0;
    virtual bool sendData(JsonObject& data) = 0;

};

#endif //_BOODSKAP_COMMUNICATOR
