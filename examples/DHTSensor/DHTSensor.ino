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
#include <DHT.h>

#define DHTPIN D2 // what digital pin we're connected to
#define CONFIG_SIZE 512
#define REPORT_INTERVAL 15000
#define MESSAGE_ID 1000 //Message defined in the platform

// Uncomment whatever type you're using!
#define DHTTYPE DHT11   // DHT 11
//#define DHTTYPE DHT22   // DHT 22  (AM2302), AM2321
//#define DHTTYPE DHT21   // DHT 21 (AM2301)

/**
 * ***** PLEASE CHANGE THE BELOW SETTINGS MATCHING YOUR ENVIRONMENT *****
*/
#define DEF_WIFI_SSID "your_wifi_ssid"  //Your WiFi SSID
#define DEF_WIFI_PSK "your_wifi_psk" //Your WiFi password
#define DEF_DOMAIN_KEY "your_domain_key" //your DOMAIN Key
#define DEF_API_KEY "your_api_key" //Your API Key
#define DEF_DEVICE_MODEL "BSKP-DHTSR" //Your device model
#define DEF_FIRMWARE_VER "1.0.0" //Your firmware version

BoodskapTransceiver Boodskap(UDP); //MQTT, UDP, HTTP
DHT dht(DHTPIN, DHTTYPE);
uint32_t lastReport = 0;

void sendReading();

void setup() {

  Serial.begin(115200);

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
  dht.begin();
}

void loop() {

  Boodskap.loop();

  if ((millis() - lastReport) >= REPORT_INTERVAL) {
    sendReading();
    lastReport = millis();
  }
}

void sendReading() {

  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  float h = dht.readHumidity();
  // Read temperature as Celsius (the default)
  float t = dht.readTemperature();
  // Read temperature as Fahrenheit (isFahrenheit = true)
  float f = dht.readTemperature(true);

  // Check if any reads failed and exit early (to try again).
  if (isnan(h) || isnan(t) || isnan(f)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }

  // Compute heat index in Fahrenheit (the default)
  float hif = dht.computeHeatIndex(f, h);
  // Compute heat index in Celsius (isFahreheit = false)
  float hic = dht.computeHeatIndex(t, h, false);

  Serial.print("Humidity: ");
  Serial.print(h);
  Serial.print(" %\t");
  Serial.print("Temperature: ");
  Serial.print(t);
  Serial.print(" *C ");
  Serial.print(f);
  Serial.print(" *F\t");
  Serial.print("Heat index: ");
  Serial.print(hic);
  Serial.print(" *C ");
  Serial.print(hif);
  Serial.println(" *F");

  /**
   * You should have message 1000 defined in the platform,
   * you could execute the below script in the "Script Console" to define the message
   * 
   * def fields = ['h': 'FLOAT', 't': 'FLOAT', 'f': 'FLOAT', 'hif': 'FLOAT', 'hic': 'FLOAT'];
   * domain.defineMessage("DHT Sensor Message", 1000, fields);
   */
  StaticJsonBuffer<128> buffer;
  JsonObject &data = buffer.createObject();
  data["h"] = h;
  data["t"] = t;
  data["f"] = f;
  data["hif"] = hif;
  data["hic"] = hic;

  Boodskap.sendMessage(MESSAGE_ID, data);
}

