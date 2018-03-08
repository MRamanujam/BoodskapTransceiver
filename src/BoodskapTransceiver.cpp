#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <WiFiManager.h>

#include "BoodskapTransceiver.h"
#include "BoodskapUdpCommunicator.h"

extern "C" {
#include "user_interface.h"
  uint16 readvdd33(void);
  bool wifi_set_sleep_type(sleep_type_t);
  sleep_type_t wifi_get_sleep_type(void);
}

bool saveConfig = false;
BoodskapCommunicator* _comm;


BoodskapTransceiver::BoodskapTransceiver(TransceiverMode mode) : _mode(mode), _lastSent(0), _lastConnected(0)
{
  switch (mode) {
    case UDP:
    default:
      _comm = new BoodskapUdpCommunicator(this);
      break;
  }
}

void BoodskapTransceiver::setup(JsonObject& config) {

  wifi_set_sleep_type(NONE_SLEEP_T);

  _ssid = config["ssid"].as<String>();
  _psk = config["psk"].as<String>();
  _domainKey = config["domain_key"].as<String>();
  _apiKey = config["api_key"].as<String>();
  _deviceId = config["dev_id"].as<String>();
  _deviceModel = config["dev_model"].as<String>();
  _firmwareVersion = config["fw_ver"].as<String>();
  _apiBasePath = config["api_path"].as<String>();
  _apiFingerprint = config["api_fp"].as<String>();
  _udpHost = config["udp_host"].as<String>();
  _udpPort = config["udp_port"];
  _mqttHost = config["mqtt_host"].as<String>();
  _mqttPort = config["mqtt_port"];
  _heartbeatInterval = config["heartbeat"];

  DEBUG.printf("ssid: %s\n", _ssid.c_str());
  DEBUG.printf("psk: %s\n", _psk.c_str());
  DEBUG.printf("domain_key: %s\n", _domainKey.c_str());
  DEBUG.printf("api_key: %s\n", _apiKey.c_str());
  DEBUG.printf("dev_id: %s\n", _deviceId.c_str());
  DEBUG.printf("dev_model: %s\n", _deviceModel.c_str());
  DEBUG.printf("fw_ver: %s\n", _firmwareVersion.c_str());
  DEBUG.printf("api_path: %s\n", _apiBasePath.c_str());
  DEBUG.printf("api_fp: %s\n", _apiFingerprint.c_str());
  DEBUG.printf("udp_host: %s\n", _udpHost.c_str());
  DEBUG.printf("udp_port: %d\n", _udpPort);
  DEBUG.printf("mqtt_host: %s\n", _mqttHost.c_str());
  DEBUG.printf("mqtt_port: %d\n", _mqttPort);
  DEBUG.printf("heartbeat: %d\n", _heartbeatInterval);

}

void saveConfigCallback()
{
  DEBUG.println("User configuation updating...");
  saveConfig = true;
}

void BoodskapTransceiver::runConfigServer() {

  saveConfig = false;

  WiFiManager wifiManager;
  //wifiManager.resetSettings();
  wifiManager.setTimeout(120);
  wifiManager.setConfigPortalTimeout(300);
  wifiManager.setConnectTimeout(30);
  wifiManager.setSaveConfigCallback(saveConfigCallback);

  WiFiManagerParameter _p_dkey("dkey", "Domain Key", _domainKey.c_str(), 15);
  WiFiManagerParameter _p_akey("akey", "API Key", _apiKey.c_str(), 15);
  WiFiManagerParameter _p_did("did", "Device ID", _deviceId.c_str(), 25);
  WiFiManagerParameter _p_api_url("api_url", "API Base URL", _apiBasePath.c_str(), 40);
  WiFiManagerParameter _p_api_fp("api_fp", "API HTTPS Fingerprint", _apiFingerprint.c_str(), 60);
  WiFiManagerParameter _p_udp_host("udp_host", "UDP Host/IP", _udpHost.c_str(), 40);
  WiFiManagerParameter _p_udp_port("udp_port", "UDP Port", String(_udpPort).c_str(), 8);
  WiFiManagerParameter _p_mqtt_host("mqtt_host", "MQTT Host/IP", _mqttHost.c_str(), 40);
  WiFiManagerParameter _p_mqtt_port("mqtt_port", "MQTT Port", String(_mqttPort).c_str(), 8);
  WiFiManagerParameter _p_heartbeat("heartbeat", "Heartbeat (seconds)", String(_heartbeatInterval).c_str(), 6);

  wifiManager.addParameter(&_p_dkey);
  wifiManager.addParameter(&_p_akey);
  wifiManager.addParameter(&_p_did);
  wifiManager.addParameter(&_p_api_url);
  wifiManager.addParameter(&_p_api_fp);
  wifiManager.addParameter(&_p_udp_host);
  wifiManager.addParameter(&_p_udp_port);
  wifiManager.addParameter(&_p_mqtt_host);
  wifiManager.addParameter(&_p_mqtt_port);
  wifiManager.addParameter(&_p_heartbeat);

  wifiManager.setAPStaticIPConfig(IPAddress(10, 0, 1, 99), IPAddress(10, 0, 1, 1), IPAddress(255, 255, 255, 0));

  if (!wifiManager.startConfigPortal(_deviceId.c_str(), "boodskap"))
  {
    DEBUG.println("*** Configuration timeout, re-looping... ***");
    return;
  }

  if (saveConfig)
  {

    StaticJsonBuffer<CONFIG_SIZE> jsonBuffer;
    JsonObject &root = jsonBuffer.createObject();

    root["ssid"] = WiFi.SSID();
    root["psk"] = WiFi.psk();
    root["domain_key"] = _p_dkey.getValue();
    root["api_key"] = _p_akey.getValue();
    root["dev_id"] = _p_did.getValue();
    root["api_path"] = _p_api_url.getValue();
    root["api_fp"] = _p_api_fp.getValue();
    root["udp_host"] = _p_udp_host.getValue();
    root["udp_port"] = String(_p_udp_port.getValue()).toInt();
    root["mqtt_host"] = _p_mqtt_host.getValue();
    root["mqtt_port"] = String(_p_mqtt_port.getValue()).toInt();
    root["heartbeat"] = String(_p_heartbeat.getValue()).toInt();

    String jsonStr;
    root.printTo(jsonStr);
    flash.open();
    flash.writeFile(BSKP_CONFIG_FILE, jsonStr);
    flash.close();

    ESP.restart();
  }
}

void BoodskapTransceiver::connect() {

  if ((millis() - _lastConnected) >= CONNECT_TIMEOUT) {
    runConfigServer();
    return;
  }

  WiFi.mode(WIFI_STA);
  WiFi.disconnect();

  int tries = 0;
  bool found = false;
  int n = WiFi.scanNetworks();

  if (n == 0) {
    DEBUG.println("no networks found");
    return;
  }

  DEBUG.print(n);
  DEBUG.println(" networks found");

  for (int i = 0; i < n; ++i)
  {
    DEBUG.print(i + 1);
    DEBUG.print(": ");
    String ssid = WiFi.SSID(i);
    if (_ssid.equals(ssid)) {
      found = true;
    }
    DEBUG.print(ssid);
    DEBUG.print(" (");
    DEBUG.print(WiFi.RSSI(i));
    DEBUG.print(")");
    DEBUG.println((WiFi.encryptionType(i) == ENC_TYPE_NONE) ? " " : "*");
    delay(10);
  }

  if (!found) {
    DEBUG.printf("Acess Point: %s not found\n", _ssid.c_str());
    return;
  }

  DEBUG.printf("Joining Acess Point %s ", _ssid.c_str());

  WiFi.begin(_ssid.c_str(), _psk.c_str());

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    DEBUG.print(".");
    if (++tries >= WIFI_MAX_TRIES) break;
  }

  DEBUG.println();

  if (WiFi.status() == WL_CONNECTED) {
    _lastConnected = millis();
    DEBUG.println("");
    DEBUG.println("WiFi connected");
    DEBUG.println("IP address: ");
    DEBUG.println(WiFi.localIP());

    _comm->setup();

  } else {
    DEBUG.printf("Unable to join %s ...\n", _ssid.c_str());
    return;
  }
}

void BoodskapTransceiver::loop() {

  if (_factoryResetRequested)
  {
    DEBUG.println("*** Performing factory reset *****");

    flash.format();
    WiFiManager wifiManager;
    wifiManager.resetSettings();

    _factoryResetRequested = false;
    _rebootRequested = true;
  }

  if (_rebootRequested)
  {
    DEBUG.println("*** Rebooting *****");
    ESP.restart();
    _rebootRequested = false;
  }

  if (_otaRequested)
  {
    doOTA(_otaModel, _otaVersion);
    _otaRequested = false;
  }

  if (WiFi.status() != WL_CONNECTED) {

    connect();

  } else {

    _lastConnected = millis();

    if ( (millis() - _lastSent) >= (_heartbeatInterval * 1000)) {
      sendHeartbeat();
    }

    _comm->loop();
  }

}

void BoodskapTransceiver::sendAck(JsonObject& header, uint32_t corrId, int ack)
{

  StaticJsonBuffer<MESSAGE_BUFF_SIZE> jsonBuffer;
  JsonObject &root = jsonBuffer.createObject();
  JsonObject &data = jsonBuffer.createObject();

  header["key"] = _domainKey.c_str();
  header["api"] = _apiKey.c_str();
  header["did"] = _deviceId.c_str();
  header["dmdl"] = _deviceModel.c_str();
  header["fwver"] = _firmwareVersion.c_str();
  header["mid"] = MSG_ACK;

  data["acked"] = ack;

  root["header"] = header;
  root["data"] = data;

  String str;
  root.printTo(str);
  DEBUG.printf("Sending ACK %s\n", str.c_str());

  if (_comm->sendData(root)) {
    _lastSent = millis();
  }

}

void BoodskapTransceiver::sendMessage(int messageId, JsonObject& data) {

  StaticJsonBuffer<MESSAGE_BUFF_SIZE> jsonBuffer;
  JsonObject &root = jsonBuffer.createObject();
  JsonObject &header = jsonBuffer.createObject();

  header["key"] = _domainKey.c_str();
  header["api"] = _apiKey.c_str();
  header["did"] = _deviceId.c_str();
  header["dmdl"] = _deviceModel.c_str();
  header["fwver"] = _firmwareVersion.c_str();
  header["mid"] = messageId;

  root["header"] = header;
  root["data"] = data;

  String type = (messageId < 100) ? "SYS" : "USR";
  String str;
  root.printTo(str);
  DEBUG.printf("Sending %s message: %s\n", type.c_str(), str.c_str());

  if (_comm->sendData(root)) {
    _lastSent = millis();
  }
}

void BoodskapTransceiver::sendHeartbeat()
{
  StaticJsonBuffer<MESSAGE_BUFF_SIZE> jsonBuffer;
  JsonObject &data = jsonBuffer.createObject();
  data["uptime"] = millis();
  data["freeheap"] = ESP.getFreeHeap();

  sendMessage(MSG_PING, data);
}

void BoodskapTransceiver::doOTA(String model, String version)
{

  char API_URL[API_URL_LEN];
  t_httpUpdate_return ret;

  sprintf(API_URL, "%s/mservice/esp8266/ota?dkey=%s&akey=%s&dmodel=%s&fwver=%s", _apiBasePath.c_str(), _domainKey.c_str(), _apiKey.c_str(), model.c_str(), version.c_str());

  bool https = String(_apiBasePath).startsWith("https");

  DEBUG.printf("New firmware from %s URL: ", https ? "ENCRYPTED" : "PLAIN");
  DEBUG.println(API_URL);

  if (https) {
    ret = ESPhttpUpdate.update(API_URL, "", _apiFingerprint.c_str());
  } else {
    ret = ESPhttpUpdate.update(API_URL, "");
  }

  switch (ret)
  {
    default:
    case HTTP_UPDATE_FAILED:
      DEBUG.printf("HTTP_UPDATE_FAILED Error (%d): %s\n", ESPhttpUpdate.getLastError(), ESPhttpUpdate.getLastErrorString().c_str());
      break;

    case HTTP_UPDATE_NO_UPDATES:
      DEBUG.println("HTTP_UPDATE_NO_UPDATES");
      break;

    case HTTP_UPDATE_OK:
      DEBUG.println("HTTP_UPDATE_OK");
      break;
  }
}



void BoodskapTransceiver::parseIncoming(byte *data)
{

  DEBUG.println("\n**** Message received ****");
  DEBUG.println((char *)data);
  DEBUG.println();

  StaticJsonBuffer<MESSAGE_BUFF_SIZE> jsonBuffer;
  JsonObject &root = jsonBuffer.parseObject(data);

  bool invalid = !root.success();

  invalid = invalid || (!root.containsKey("header"));
  invalid = (invalid || !root.containsKey("data"));

  if (invalid)
  {
    DEBUG.println("Not a JSON message");
    if (handleData) {
      handleData(data);
    }
    return;
  }

  JsonObject &header = root["header"];
  JsonObject &message = root["data"];

  invalid = (!header.containsKey("key") || !header["key"].is<const char *>());
  invalid = (invalid || !header.containsKey("api") || !header["api"].is<const char *>());
  invalid = (invalid || !header.containsKey("did") || !header["did"].is<const char *>());
  invalid = (invalid || !header.containsKey("mid") || !header["mid"].is<uint32_t>());

  if (invalid)
  {
    DEBUG.println("Invalid message received");
    DEBUG.println((char *)data);
    return;
  }

  String dkey = header["key"];
  if (dkey != _domainKey)
  {
    DEBUG.println("Invalid message received, DOMAIN_KEY mismatch");
    return;
  }

  String akey = header["api"];
  if (akey != _apiKey)
  {
    DEBUG.println("Invalid message received, API_KEY mismatch");
    return;
  }

  String did = header["did"];
  if (did != _deviceId)
  {
    DEBUG.println("Invalid message received, DEVICE_ID mismatch");
    return;
  }

  bool shouldAck = header.containsKey("corrid");
  uint32_t messageId = header["mid"].as<uint32_t>();

  bool ack = false;

  if (messageId >= 100 || messageId < 0)
  { //User defined commands
    DEBUG.printf("*** User Defined Message : %d ***\n", messageId);
    if (handleMessage) {
      ack = handleMessage(messageId, header, message);
    }
  }
  else
  { //System commands

    switch (messageId)
    {
      case MSG_PING: //ping
        shouldAck = false;
        sendHeartbeat();
        break;
      case MSG_ACK:
        ack = true;
        DEBUG.println("Message Delivery Confirmed");
        break;
      case MSG_FACTORY_RESET:
        ack = _factoryResetRequested = true;
        break;
      case MSG_OTA:

        _otaRequested = message.containsKey("model");
        _otaRequested = _otaRequested && message.containsKey("version");

        if (_otaRequested)
        {
          _otaModel = message["model"].as<String>();
          _otaVersion = message["version"].as<String>();
        }

        ack = _otaRequested;
        break;

      case MSG_REBOOT:
        _rebootRequested = ack = true;
        break;
      default:
        DEBUG.printf("*** Unimplemented Message: %d ***\n", messageId);
    }
  }

  if (shouldAck)
  {
    uint32_t corrId = header["corrid"];
    sendAck(header, corrId, ack ? 1 : 0);
  }
}

