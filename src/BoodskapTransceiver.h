#ifndef _BOODSKAP_TRANSCEIVER
#define _BOODSKAP_TRANSCEIVER

#define ESP8266
//#define ESP32

#include <ArduinoJson.h>
#include "Storage.h"

#ifdef ESP8266
#include <ESP8266WiFi.h>
#include <ESP8266WiFiAP.h>
#include <ESP8266WiFiGeneric.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266WiFiScan.h>
#include <ESP8266WiFiSTA.h>
#include <ESP8266WiFiType.h>
#include <WiFiClient.h>
#include <WiFiClientSecure.h>
#include <WiFiServer.h>
#include <ESP8266httpUpdate.h>
#endif //ESP8266

typedef enum {
  UDP,
  MQTT,
  HTTP
} TransceiverMode;

// **** System Message IDs ******
#define MSG_PING 1
#define MSG_ACK 2
#define MSG_SEND_CFG 4
#define MSG_SAVE_CFG 5
#define MSG_FACTORY_RESET 97
#define MSG_OTA 98
#define MSG_REBOOT 99

// **** Constants ****
#define MQTT_CLIENT_ID_LEN 25
#define MQTT_USER_ID_LEN 40
#define MQTT_PASSWD_LEN 25
#define MQTT_IN_TOPIC_LEN 80
#define MQTT_OUT_TOPIC_LEN 128

#define CONNECT_TIMEOUT 60000
#define CONFIG_SIZE 1024
#define MESSAGE_BUFF_SIZE 512
#define API_URL_LEN 256
#define WIFI_MAX_TRIES 15
#define UDP_LOCAL_PORT 0

#define DEBUG Serial

class BoodskapTransceiver {

  public:

    BoodskapTransceiver(TransceiverMode mode = MQTT);

    void setup(JsonObject& config);

    void setHandleData(void (*callback)(byte* data)) { handleData = callback; }

    void setHandleMessage(bool (*callback)(uint16_t messageId, JsonObject& header, JsonObject& data)) { handleMessage = callback; }

    void loop();

    bool isConnected() { return (WiFi.status() == WL_CONNECTED); }

    void sendHeartbeat();

    void sendMessage(int messageId, JsonObject& data);

    void doOTA(String model, String version);

    void parseIncoming(byte *data);

    String apiBasePath() { return _apiBasePath; }

    String apiFingerPrint() { return _apiFingerprint; }

    String udpHost() { return _udpHost; }

    uint16_t udpPort() { return _udpPort; }

    String mqttHost() { return _mqttHost; }

    uint16_t mqttPort() { return _mqttPort; }

    String domainKey() {return _domainKey;}

    String apiKey() {return _apiKey;}

    String deviceId() {return _deviceId;}

    String deviceModel() {return _deviceModel;}

    String firmwareVersion() {return _firmwareVersion;}

  private:

    void connect();

    void sendAck(JsonObject& header, uint32_t corrId, int ack);

    void runConfigServer();

  private:
    TransceiverMode _mode;
    String _ssid;                 // ssid
    String _psk;                  // psk
    String _domainKey;            // domain_key
    String _apiKey;               // api_key
    String _deviceId;              // dev_id
    String _deviceModel;          // dev_model
    String _firmwareVersion;      // fw_ver
    uint16_t _heartbeatInterval;  // heartbeat
    String _apiBasePath;          // api_path
    String _apiFingerprint;       // api_fp
    String _udpHost;              // udp_host
    uint16_t _udpPort;            // udp_port
    String _mqttHost;             // mqtt_host
    uint16_t _mqttPort;           // mqtt_port
    bool _factoryResetRequested;
    bool _rebootRequested;
    bool _otaRequested;
    String _otaModel;
    String _otaVersion;
    uint32_t _lastSent;
    uint32_t _lastConnected;
    bool apiHttps;
    void (*handleData)(byte* data);
    bool (*handleMessage)(uint16_t messageId, JsonObject& header, JsonObject& data);
    Storage flash;
};

#endif //_BOODSKAP_TRANSCEIVER
