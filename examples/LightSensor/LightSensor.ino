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

#include <ArduinoJson.h>
#include <BoodskapCommunicator.h>

#define CONFIG_SIZE 512
#define REPORT_INTERVAL 60000
#define SENSE_INTERVAL 50
#define MESSAGE_ID 101 //Message defined in the platform

#define THRESHOLD_VAL 900  // maximum threshold value
#define ANALOOG A0
#define LED D4


 /*
 * ***** PLEASE CHANGE THE BELOW SETTINGS MATCHING YOUR ENVIRONMENT *****
*/
#define DEF_WIFI_SSID "your_wifi_ssid"  //Your WiFi SSID
#define DEF_WIFI_PSK "your_wifi_psk" //Your WiFi password
#define DEF_DOMAIN_KEY "your_domain_key" //your DOMAIN Key
#define DEF_API_KEY "your_api_key" //Your API Key
#define DEF_DEVICE_MODEL "BSKP_LIGHT_DETECTOR" //Your device model
#define DEF_FIRMWARE_VER "1.0.0" //Your firmware version


BoodskapTransceiver Boodskap(UDP); //MQTT, UDP, HTTP

bool current_state = LOW;
bool previous_state = LOW;
uint32_t sensor_val;


uint32_t lastReport = 0;
uint32_t lastSense = 0;
void sendReading();
bool state;

void setup() {

  Serial.begin(115200);
  pinMode (LED, OUTPUT) ;// define LED as output interface
  pinMode (ANALOOG, INPUT) ;// output interface defines the flame sensor_val
  digitalWrite(LED,LOW);
  StaticJsonBuffer<CONFIG_SIZE> buffer;
  JsonObject &config = buffer.createObject();
  config["ssid"] = DEF_WIFI_SSID;
  config["psk"] = DEF_WIFI_PSK;
  config["domain_key"] = DEF_DOMAIN_KEY;
  config["api_key"] = DEF_API_KEY;
  config["dev_model"] = DEF_DEVICE_MODEL;
  config["fw_ver"] = DEF_FIRMWARE_VER;
  config["dev_id"] = String("ESP8266-") + String(ESP.getChipId()); //Your unique device ID

  /**
     If you have setup your own Boodskap IoT Platform, then change the below settings matching your installation
     Leave it for default Boodskap IoT Cloud Instance
  */
  
  config["api_path"] = "https://api.boodskap.io"; //HTTP API Base Path Endpoint
  config["api_fp"] = "B9:01:85:CE:E3:48:5F:5E:E1:19:74:CC:47:A1:4A:63:26:B4:CB:32"; //In case of HTTPS enter your server fingerprint (https://www.grc.com/fingerprints.htm)
  config["udp_host"] = "udp.boodskap.io"; //UDP Server IP
  config["udp_port"] = 5555; //UDP Server Port
  config["mqtt_host"] = "mqtt.boodskap.io"; //MQTT Server IP
  config["mqtt_port"] = 1883; //MQTT Server Port
  config["heartbeat"] = 45; //seconds

  Boodskap.setup(config);
  
}
void loop() {
  Boodskap.loop();
  

  if((millis() - lastSense) >= SENSE_INTERVAL) {
     sensor_val = analogRead(ANALOOG);
     lastSense = millis();   
     Serial.println(sensor_val);
  }

  if ((millis() - lastReport) >= REPORT_INTERVAL) {
    sendReading();
    lastReport = millis();
  }
  
  
 
  if (sensor_val < THRESHOLD_VAL) // When the sensor_val detects Light, LED flashes
  {
    digitalWrite (LED, HIGH);
    state = true;
    current_state = HIGH;
  }
  else if (sensor_val > THRESHOLD_VAL) { // when the sensor_val state change and not detect any light LED off
    digitalWrite (LED, LOW);
    state = false;
    current_state = LOW;
  }
  
  if (current_state != previous_state) { // message will be sent to the platform only if state change occur
   
    sendReading();
    Serial.println(sensor_val);
    Serial.println(state);
    
  }
  previous_state = current_state;
}

void sendReading() {    // sending message to the platform
  
  StaticJsonBuffer<128> buffer;
  JsonObject &data = buffer.createObject();

  data["state"] = state;
  
  Boodskap.sendMessage(MESSAGE_ID, data);

}
