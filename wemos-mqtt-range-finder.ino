#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <PubSubClient.h>

const int TRIGPIN = D3;
const int ECHOPIN = D4;
const float SPEED_OF_SOUND = .0344;

String devID = "esp8266-range-finder";
const char* parkDistanceTopic = "/garage/park/distance";
const char* logInfoTopic = "/log/info";
int timer = 0;

WiFiClient wifiClient;
PubSubClient client("192.168.1.239", 1883, wifiClient);

void findKnownWiFiNetworks() {
  ESP8266WiFiMulti wifiMulti;
  WiFi.mode(WIFI_STA);
  wifiMulti.addAP("BYU-WiFi", "");
  wifiMulti.addAP("Hancock2.4G", "Arohanui");
  Serial.println("");
  Serial.print("Connecting to Wifi");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    wifiMulti.run();
    delay(1000);
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(WiFi.SSID().c_str());
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

void pubDistance(String distance) {
  if (millis() > timer + 1000) {
    timer = millis();
    client.publish(parkDistanceTopic, distance.c_str());
  }
}

void pubDebug(String message) {
  client.publish("/log/debug", message.c_str());
}

void reconnectMQTT() {
  while(!client.connected()) {
    if (client.connect((char*) devID.c_str(), "mqtt", "mymqttpassword")) {
      Serial.println("Connected to MQTT server");
      String message = devID;
      message += ": ";
      message += WiFi.localIP().toString();
      if (client.publish(logInfoTopic, message.c_str())) {
        Serial.println("published successfully");
      } else {
        Serial.println("failed to publish");
      }
    } else {
      Serial.println("MQTT connection failed");
      delay(5000);
    }
  }
}

void setup(void) {
  Serial.begin(115200);
  findKnownWiFiNetworks();
  pinMode(TRIGPIN, OUTPUT);
  pinMode(ECHOPIN, INPUT);
}

void loop(void) {
  if (!client.connected()) {
    reconnectMQTT();
  }
  client.loop();
  Serial.println("working");
  long duration, distance;
  digitalWrite(TRIGPIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIGPIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIGPIN, LOW);
  duration = pulseIn(ECHOPIN, HIGH);
  distance = (duration/2) * SPEED_OF_SOUND;
  pubDistance(String(distance));
}
