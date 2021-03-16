// SecuriCAP - Capacative touch triggered security camera
//
// Designed to register an incident when concecutive touches are detected
// Upon registering an incident, details of the incident are recorded and a photo is captured before being sent to the cloud
#include "secret.h"

int touchThresh = 40;
int incidentThresh = 300;

#include <WiFiClientSecure.h>
#include <MQTTClient.h>
#include <ArduinoJson.h>
#include "WiFi.h"
#include "esp_camera.h"
#include "Base64.h"

boolean incidentFlag()
{
  static int touchVal = 0;
  if (touchRead(T6)<touchThresh) {        // if a touch has occurred
    touchVal = touchVal + 10;             // we want the touch value to increase, thereby approaching the threshold
    Serial.println(touchVal);
    delay(100);                           // delay here is short so that the touch value increases at a faster rate than it decreases
  } else {                                // if a touch has occurred
    if (touchVal > 10) {
      touchVal = touchVal - 10;           // we want the touch value to decrease
      Serial.println(touchVal);
      delay(1000);                        // delay here is long so that the touch value d3ecreases at a slower rate than it increases
    }
  }

  if (touchVal > incidentThresh) {        // if above threshold
    return true;
  } else {
    return false;
  }
}

// The MQTT topics that this device should publish/subscribe
#define AWS_IOT_PUBLISH_TOPIC   "esp32/pub"
#define AWS_IOT_SUBSCRIBE_TOPIC "esp32/sub"

WiFiClientSecure net = WiFiClientSecure();
MQTTClient client = MQTTClient(256);

void connectAWS()
{
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  Serial.println("Connecting to Wi-Fi");

  while (WiFi.status() != WL_CONNECTED){
    delay(500);
    Serial.print(".");
  }

  // Configure WiFiClientSecure to use the AWS IoT device credentials
  net.setCACert(AWS_CERT_CA);
  net.setCertificate(AWS_CERT_CRT);
  net.setPrivateKey(AWS_CERT_PRIVATE);

  // Connect to the MQTT broker on the AWS endpoint we defined earlier
  client.begin(AWS_IOT_ENDPOINT, 8883, net);

  // Create a message handler
  client.onMessage(messageHandler);

  Serial.print("Connecting to AWS IOT");

  while (!client.connect(THINGNAME)) {
    Serial.print(".");
    delay(100);
  }

  if(!client.connected()){
    Serial.println("AWS IoT Timeout!");
    return;
  }

  // Subscribe to a topic
  client.subscribe(AWS_IOT_SUBSCRIBE_TOPIC);

  Serial.println("AWS IoT Connected!");
}

void publishMessage()
{
  StaticJsonDocument<200> doc;
  doc["touchvalue"] = touchRead(T6);
  doc["INTRUDER"];
  char jsonBuffer[512];
  serializeJson(doc, jsonBuffer); // print to client


  client.publish(AWS_IOT_PUBLISH_TOPIC, jsonBuffer);
}

void messageHandler(String &topic, String &payload) {
  Serial.println("incoming: " + topic + " - " + payload);
if (payload == "{message: securiCAP_ReactNative_button}"){
  Serial.println("Picture/Video stored\n\r");
  Serial.println("URL: http://s3.xxxxx.xxxxxx.xxxxx./image.jpg\n\r");
}

//  StaticJsonDocument<200> doc;
//  deserializeJson(doc, payload);
//  const char* message = doc["message"];
}

// initializing and setting up initial values
void setup()
{
  Serial.begin(115200);
  Serial.println("------SecuriCAP------");
  Serial.setDebugOutput(true);
  connectAWS();
}

// Main code to run repeatedly: 
void loop()
{
  if(incidentFlag()){
    // Placeholder
    Serial.println("INCIDENT!");
    publishMessage();
    Serial.println("Message send func to AWS finished");
  }
  client.loop();
  delay(1000);
}
