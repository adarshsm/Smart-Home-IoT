/**
 IBM IoT Foundation managed Device

 Author: Ant Elder
 License: Apache License v2
*/
#include <ESP8266WiFi.h>
#include <PubSubClient.h> // https://github.com/knolleary/pubsubclient/releases/tag/v2.3
#include <ArduinoJson.h> // https://github.com/bblanchon/ArduinoJson/releases/tag/v5.0.7
#include <Servo.h>

Servo myservo;
#include "DHT.h"  //U need to add DHT sensor library 
#define DHTPIN D2   // what pin we're connected to 
#define DHTTYPE DHT11   // define type of sensor DHT 11
DHT dht (DHTPIN, DHTTYPE);
//-------- Customise these values -----------
const char* ssid = "SmartBridge WIFI";
const char* password = "SmartBridge";
float t;
int h,flame;
int redPin = D0;
int greenPin = D1; 
int bluePin = D8;
#define BUZZER D5
#define SERVO D6
#define led D3
#define ORG "q91plx"
#define DEVICE_TYPE "device"
#define DEVICE_ID "mydevice123"
#define TOKEN "1C*z4L0e-tNyZNtYRi"
//-------- Customise the above values --------

char server[] = ORG ".messaging.internetofthings.ibmcloud.com";
char authMethod[] = "use-token-auth";
char token[] = TOKEN;
char clientId[] = "d:" ORG ":" DEVICE_TYPE ":" DEVICE_ID;
//String value;
const char publishTopic[] = "iot-2/evt/Automation/fmt/json";
const char responseTopic[] = "iotdm-1/response";
const char manageTopic[] = "iotdevice-1/mgmt/manage";
const char updateTopic[] = "iotdm-1/device/update";
const char rebootTopic[] = "iotdm-1/mgmt/initiate/device/reboot";
const char subTopic[] = "iot-2/cmd/surya/fmt/json";
void wifiConnect();
void mqttConnect();
void initManagedDevice();
void publishData();
void handleUpdate(byte* payload) ;
void callback(char* topic, byte* payload, unsigned int payloadLength);
WiFiClient wifiClient;
PubSubClient client(server, 1883, callback, wifiClient);

int publishInterval = 5000; // 30 seconds
long lastPublishMillis;


void setup() {
  Serial.begin(115200);

  dht.begin(); 
  Serial.println();
pinMode(D3,OUTPUT);
pinMode(D4,OUTPUT);
pinMode(D5,OUTPUT);
pinMode(redPin,OUTPUT);
pinMode(greenPin,OUTPUT);
pinMode(bluePin,OUTPUT);
myservo.attach(D6);
  wifiConnect();
  mqttConnect();
  initManagedDevice();
}

void loop() {

  
  if (millis() - lastPublishMillis > publishInterval) {
    publishData();
    lastPublishMillis = millis();
  }

  if (!client.loop()) {
    mqttConnect();
    initManagedDevice();
  }
}

void wifiConnect() {
  Serial.print("Connecting to "); Serial.print(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.print("nWiFi connected, IP address: "); Serial.println(WiFi.localIP());
}

void mqttConnect() {
  if (!!!client.connected()) {
    Serial.print("Reconnecting MQTT client to "); Serial.println(server);
    while (!!!client.connect(clientId, authMethod, token)) {
      Serial.print(".");
      delay(500);
    }
    Serial.println();
  }
}

void initManagedDevice() {
  if (client.subscribe("iotdm-1/response")) {
    Serial.println("subscribe to responses OK");
  } else {
    Serial.println("subscribe to responses FAILED");
  }

  if (client.subscribe(rebootTopic)) {
    Serial.println("subscribe to reboot OK");
  } else {
    Serial.println("subscribe to reboot FAILED");
  }

  if (client.subscribe("iotdm-1/device/update")) {
    Serial.println("subscribe to update OK");
  } else {
    Serial.println("subscribe to update FAILED");
  }
  if (client.subscribe(subTopic)) {
    Serial.println("subscribe to subtopic OK");
  } else {
    Serial.println("subscribe to update FAILED");
  }


  StaticJsonBuffer<300> jsonBuffer;
  JsonObject& root = jsonBuffer.createObject();
  JsonObject& d = root.createNestedObject("d");
  JsonObject& metadata = d.createNestedObject("metadata");
  metadata["publishInterval"] = publishInterval;
  JsonObject& supports = d.createNestedObject("supports");
  supports["deviceActions"] = true;

  char buff[300];
  root.printTo(buff, sizeof(buff));
  Serial.println("publishing device metadata:"); Serial.println(buff);
  if (client.publish(manageTopic, buff)) {
    Serial.println("device Publish ok");
  } else {
    Serial.print("device Publish failed:");
  }
}

void publishData() {
  
    h = dht.readHumidity();
    t = dht.readTemperature();
    flame = analogRead(A0);
    flame = map(flame,0,1024,0,100);
    
    
  String payload = "{\"d\":{\"Temp\":";
  payload += t;
  payload += ",""\"Humidity\":";
  payload += h;
  payload += ",""\"Flame\":";
  payload += flame;
  payload += "}}";

  Serial.print("Sending payload: "); Serial.println(payload);

  if (client.publish(publishTopic, (char*) payload.c_str())) {
    Serial.println("Publish OK");
  } else {
    Serial.println("Publish FAILED");
  }
}

void callback(char* topic, byte* payload, unsigned int payloadLength) {
  Serial.print("callback invoked for topic: "); Serial.println(topic);

  if (strcmp (responseTopic, topic) == 0) {
    return; // just print of response for now
  }

  if (strcmp (rebootTopic, topic) == 0) {
    Serial.println("Rebooting...");
    ESP.restart();
  }

  if (strcmp (updateTopic, topic) == 0) {
    handleUpdate(payload);
  }
  if (strcmp (subTopic, topic) == 0) {
    Serial.print("Subscribed");
    Serial.println((char*)payload);
    handleUpdate(payload);
  }
}
void setColor(int red, int green, int blue)
{

    
  analogWrite(redPin, red);
  analogWrite(greenPin, green);
  analogWrite(bluePin, blue);  
  delay(500);
}
void handleUpdate(byte* payload) {
  StaticJsonBuffer<300> jsonBuffer;
  JsonObject& root = jsonBuffer.parseObject((char*)payload);
  if (!root.success()) {
    Serial.println("handleUpdate: payload parse FAILED");
    return;
  }
String value=root["command"];
  Serial.print("data:");
  Serial.println(value);
  if(value=="HLIGHTON")
  {
    digitalWrite(D3,HIGH);
    Serial.println("HALL LIGHT ON");
    
  }
  if(value=="HLIGHTOFF")
  {
    digitalWrite(D3,LOW);
    Serial.println("HALL LIGHT OFF");
  }
  if(value=="KLIGHTON")
  {
    digitalWrite(D4,HIGH);
    Serial.println("KITCHEN LIGHT ON");
    
  }
    if(value=="KLIGHTOFF")
  {
    digitalWrite(D4,LOW);
    Serial.println("KITCHEN LIGHT OFF");
  }
  if(value=="255,0,0")
  { 
    setColor(255,0,0);  // red
    Serial.println("RED"); 
  }
  if(value=="0,0,255")
  { 
    setColor(0,0,255);  
    Serial.println("BLUE"); 
  }
  if(value=="255,255,0")
  { 
    setColor(255,255,0);  
    Serial.println("YELLOW"); 
  }
  if(value=="0,128,0")
  { 
    setColor(0,128,0); 
    Serial.println("GREEN"); 
  }
  if(value=="127,255,212")
  { 
    setColor(127,255,212); 
    Serial.println("AQUA MARINE"); 
  }
  if(value=="65,105,225")
  { 
    setColor(65,105,225); 
    Serial.println("ROYAL BLUE"); 
  }
  if(value=="0,255,127")
  { 
    setColor(0,255,127); 
    Serial.println("SPRING GREEN"); 
  }
  if(value=="128,0,128")
  { 
    setColor(128,0,128);  
    Serial.println("PURPLE"); 
  }
  if(value=="250,128,114")
  { 
    setColor(250,128,114);  // red
    Serial.println("SALMON"); 
  }
  if(value=="0,255,255")
  { 
    setColor(0,255,255);  // red
    Serial.println("CYAN"); 
  }
  if(value=="238,130,238")
  { 
    setColor(255,0,0);  // red
    Serial.println("VIOLET"); 
  }
  if(value=="255,165,0")
  { 
    setColor(255,165,0);  // red
    Serial.println("ORANGE"); 
  }
  if(value=="245,222,179")
  { 
    setColor(245,222,179);  // red
    Serial.println("WHEAT"); 
  }
  if(value=="255,255,255")
  { 
    setColor(255,255,255);  // red
    Serial.println("WHITE"); 
  }
  if(value=="255,228,225")
  { 
    setColor(255,228,225);  // red
    Serial.println("MISTY ROSE"); 
  }
  if(value=="0,128,128")
  { 
    setColor(0,128,128);  // red
    Serial.println("TEAL"); 
  }
  if(value=="255,192,203")
  { 
    setColor(255,192,203);  // red
    Serial.println("PINK"); 
  }
  if(value=="230,230,250")
  {     setColor(230,230,250);  // red
    Serial.println("LAVENDER"); 
  }
  if(value == "BUZZERON")
  {
    digitalWrite(D5,HIGH);
    Serial.println("BUZZER ON");
    delay(5000);
    digitalWrite(D5,LOW);
  }
  if(value == "DOOROPEN")
  {
    myservo.write(90);
  }
  if(value == "DOORCLOSE")
  {
    myservo.write(0);
  }
  value="";
  Serial.println("handleUpdate payload:"); root.prettyPrintTo(Serial); Serial.println();

  JsonObject& d = root["d"];
  JsonArray& fields = d["fields"];
  for (JsonArray::iterator it = fields.begin(); it != fields.end(); ++it) {
    JsonObject& field = *it;
    const char* fieldName = field["field"];
    if (strcmp (fieldName, "metadata") == 0) {
      JsonObject& fieldValue = field["value"];
      if (fieldValue.containsKey("publishInterval")) {
        publishInterval = fieldValue["publishInterval"];
        Serial.print("publishInterval:"); Serial.println(publishInterval);
      }
    }
  }
}

