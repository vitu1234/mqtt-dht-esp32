/*
   MQTT Sensor - Temperature and Humidity (DHT22) for Home-Assistant - NodeMCU (ESP8266)
   https://home-assistant.io/components/sensor.mqtt/

   Libraries :
    - ESP8266 core for Arduino : https://github.com/esp8266/Arduino
    - PubSubClient : https://github.com/knolleary/pubsubclient
    - DHT : https://github.com/adafruit/DHT-sensor-library
    - ArduinoJson : https://github.com/bblanchon/ArduinoJson

   Sources :
    - File > Examples > ES8266WiFi > WiFiClient
    - File > Examples > PubSubClient > mqtt_auth
    - File > Examples > PubSubClient > mqtt_esp8266
    - File > Examples > DHT sensor library > DHTtester
    - File > Examples > ArduinoJson > JsonGeneratorExample
    - http://www.jerome-bernard.com/blog/2015/10/04/wifi-temperature-sensor-with-nodemcu-esp8266/

   Schematic :
    - https://github.com/mertenats/open-home-automation/blob/master/ha_mqtt_sensor_dht22/Schematic.png
    - DHT22 leg 1 - VCC
    - DHT22 leg 2 - D1/GPIO5 - Resistor 4.7K Ohms - GND
    - DHT22 leg 4 - GND
    - D0/GPIO16 - RST (wake-up purpose)

   Configuration (HA) :
    sensor 1:
      platform: mqtt
      state_topic: 'office/sensor1'
      name: 'Temperature'
      unit_of_measurement: '°C'
      value_template: '{{ value_json.temperature }}'
    
    sensor 2:
      platform: mqtt
      state_topic: 'office/sensor1'
      name: 'Humidity'
      unit_of_measurement: '%'
      value_template: '{{ value_json.humidity }}'

   Samuel M. - v1.1 - 08.2016
   If you like this example, please add a star! Thank you!
   https://github.com/mertenats/open-home-automation
*/

#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include "DHT.h"
#include <ArduinoJson.h>

#define MQTT_VERSION MQTT_VERSION_3_1_1

// Wifi: SSID and password
const char* WIFI_SSID = "onap_wifi";
const char* WIFI_PASSWORD = "87654321";

// MQTT: ID, server IP, port, username and password
char MQTT_CLIENT_ID[16]; // Buffer to store the client ID (max length is 23 characters)
const PROGMEM char* MQTT_SERVER_IP = "192.168.12.225";
const PROGMEM uint16_t MQTT_SERVER_PORT = 1883;
const PROGMEM char* MQTT_USER = "flotta";
const PROGMEM char* MQTT_PASSWORD = "flotta";
String device_id = "esp8266-0XP3XQ07";

// MQTT: topic
const PROGMEM char* MQTT_SENSOR_TOPIC = "device/edge/upstream/wifi";
const PROGMEM char* WILL_TOPIC = "device/edge/upstream/wifi/availability";
int WILL_QoS= 0;
bool WILL_Retain= true;
const PROGMEM char* WILL_Message= "offline";


// sleeping time
const PROGMEM uint16_t SLEEPING_TIME_IN_SECONDS = 60; // 10 minutes x 60 seconds

// DHT - D1/GPIO5
#define DHTPIN 5
#define DHTTYPE DHT11

DHT dht(DHTPIN, DHTTYPE);
WiFiClient wifiClient;
PubSubClient client(wifiClient);

// function called to publish the temperature and the humidity
void publishData(float p_temperature, float p_humidity) {
  // create a JSON object
  // doc : https://github.com/bblanchon/ArduinoJson/wiki/API%20Reference
  // StaticJsonBuffer<256> jsonBuffer;
  // JsonObject& root = jsonBuffer.createObject();

  //   // Add device information
  // // JsonObject& device_info = root.createNestedObject("device");
  // root["name"] = "DHT";
  // root["manufacturer"] = "DCN";
  // root["model"] = "ESP32";
  // // root["sw_version"] = "1.0.0";
  // root["id"] = "0XP3XQ07";
  // // root["protocol"] = "MQTT";
  // // root["connection"] = "Wi-Fi";

  // JsonArray& deviceProperties = root.createNestedArray("properties");

  // JsonObject& sensor_info1 = deviceProperties.createNestedObject();
  
  // JsonObject& sensor_info2 = deviceProperties.createNestedObject();
  // sensor_info2["id"] = "2xj24";
  // sensor_info2["mode"] = "r";
  // sensor_info2["name"] = "Hum.";
  // sensor_info2["read"] = String(p_humidity) + "%";
  
  // sensor_info1["name"] = "Temp.";
  // sensor_info1["read"] =  String(p_temperature) + "°C";
  // sensor_info1["id"] = "dh3jm";
  // sensor_info1["mode"] = "r";


 

  
  
  // // INFO: the data must be converted into a string; a problem occurs when using floats...
  // sensor_info1["temperature"] = (String)p_temperature;
  // sensor_info1["humidity"] = (String)p_humidity;



  // Print the JSON payload without the trailing comma
  // char data[256];
  // size_t payloadLength = root.printTo(data, sizeof(data));
  // if (data[payloadLength - 1] == ',') {
  //   data[payloadLength - 1] = '\0'; // Remove the trailing comma
  // }
  
//  root.prettyPrintTo(Serial);
//  Serial.println("");
  /*
     {
        "temperature": "23.20" ,
        "humidity": "43.70"
     }
  */

  String jsonString = "{\"name\":\"DHT\",\"manufacturer\":\"DCN\",\"model\":\"ESP32\",\"id\":\""+device_id+"\",\"properties\":[{\"id\":\"55j64\",\"mode\":\"Read\",\"name\":\"Temperature\",\"read\":\"" + String(p_temperature) + "°C\"},{\"id\":\"2xj24\",\"mode\":\"Read\",\"name\":\"Humidity\",\"read\":\"" + String(p_humidity) + "%\"}]}";

//  char data[200];
//  root.printTo(data, root.measureLength() + 1);
  client.publish(MQTT_SENSOR_TOPIC, jsonString.c_str(), true);
  yield();
}

// function called when a MQTT message arrived
void callback(char* p_topic, byte* p_payload, unsigned int p_length) {
}



void setup() {
// init the serial
  Serial.begin(115200);

  dht.begin();

  // init the WiFi connection
  Serial.println();
  Serial.println();
  Serial.print("INFO: Connecting to ");
  WiFi.mode(WIFI_STA);
  Serial.println(WIFI_SSID);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("INFO: WiFi connected");
  Serial.println("INFO: IP address: ");
  Serial.println(WiFi.localIP());

  // init the MQTT connection
  client.setServer(MQTT_SERVER_IP, MQTT_SERVER_PORT);
  client.setCallback(callback);

  // Check if this is a wake-up from deep sleep
  if (WiFi.status() == WL_CONNECTED && WiFi.getMode() == WIFI_STA && client.connected()) {
    // If it is a wake-up from deep sleep and the MQTT connection was already established, return from setup
    return;
  }
  
  // If not a wake-up from deep sleep or MQTT connection was not established, reconnect to MQTT
  reconnect();
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  float h = dht.readHumidity();
  // Read temperature as Celsius (the default)
  float t = dht.readTemperature();

  if (isnan(h) || isnan(t)) {
    Serial.println("ERROR: Failed to read from DHT sensor!");
    return;
  } else {
    //Serial.println(h);
    //Serial.println(t);
    publishData(t, h);
  }

  Serial.println("INFO: Closing the MQTT connection");
  client.disconnect();

  Serial.println("INFO: Closing the Wifi connection");
  WiFi.disconnect();

  
  ESP.deepSleep(SLEEPING_TIME_IN_SECONDS * 10, WAKE_RF_DEFAULT);
  delay(10000); // wait for deep sleep to happen
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
     WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
    }
  
    Serial.println("");
    Serial.println("INFO: WiFi connected");
    Serial.println("INFO: IP address: ");
    Serial.println(WiFi.localIP());
    delay(500);
    
    
    Serial.println("INFO: Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect(MQTT_CLIENT_ID, MQTT_USER, MQTT_PASSWORD, WILL_TOPIC, WILL_QoS,WILL_Retain, WILL_Message)) {
      Serial.println("INFO: connected");
    } else {
      Serial.print("ERROR: failed, rc=");
      Serial.print(client.state());
      Serial.println("DEBUG: try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}
