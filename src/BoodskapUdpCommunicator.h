#ifndef _BOODSKAP_UDP_COMMUNICATOR
#define _BOODSKAP_UDP_COMMUNICATOR

#include <ArduinoJson.h>
#include "BoodskapCommunicator.h"
#include "BoodskapTransceiver.h"

class BoodskapUdpCommunicator : public BoodskapCommunicator {

  public:
    BoodskapUdpCommunicator(BoodskapTransceiver* comm);
    virtual void setup();
    virtual void loop();
    virtual bool sendData(JsonObject& data);

  private:

    BoodskapTransceiver* _comm;

};

#endif //_BOODSKAP_UDP_COMMUNICATOR
