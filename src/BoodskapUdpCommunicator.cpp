#include <WiFiUdp.h>
#include "BoodskapUdpCommunicator.h"
#include "BoodskapTransceiver.h"

WiFiUDP Udp;
char UDP_PACKET[MESSAGE_BUFF_SIZE];

BoodskapUdpCommunicator::BoodskapUdpCommunicator(BoodskapTransceiver* comm) : _comm(comm) {

}

void BoodskapUdpCommunicator::setup() {

  int ret = Udp.begin(UDP_LOCAL_PORT);

  if (ret == 1) {
    DEBUG.printf("UDP is Now listening at IP %s, PORT %d\n", WiFi.localIP().toString().c_str(), Udp.localPort());
  } else {
    DEBUG.println("*** UDP Failed to Listen ***");
  }
}

void BoodskapUdpCommunicator::loop() {

  int packetSize = Udp.parsePacket();

  if (packetSize)
  {
    DEBUG.printf("UDP Received %d bytes from %s, port %d\n", packetSize, Udp.remoteIP().toString().c_str(), Udp.remotePort());
    int len = Udp.read(UDP_PACKET, MESSAGE_BUFF_SIZE);
    if (len > 0)
    {
      UDP_PACKET[len] = 0; //Null terminate
      _comm->parseIncoming((byte *)UDP_PACKET);
    }
  }
}

bool BoodskapUdpCommunicator::sendData(JsonObject& data) {

  String jsonStr;
  data.printTo(jsonStr);

  int ret = Udp.beginPacket(_comm->udpHost().c_str(), _comm->udpPort());
  if (ret != 1) {
    DEBUG.println("UDP failed to begin packet");
    return false;
  }

  ret = Udp.write(jsonStr.c_str());
  if (ret != jsonStr.length()) {
    DEBUG.printf("UDP wrote %d bytes, expected %d bytes\n", ret, jsonStr.length());
    return false;
  }

  ret = Udp.endPacket();
  if (ret != 1) {
    DEBUG.println("UDP failed to end packet");
    return false;
  }

  return true;
}

