/**
   ESP8266SignalkClient.ino

    Created on: 22-NOV-2023

*/

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#include "ArduinoJson.h"
#include <EEPROM.h>

// User configuration starts here
String SSID = "openplotter";
String wifiPassPhrase = "Pandan123!";
String zeroConfHostPort = "http://10.10.10.1:3000"; // Reminder to write a proper zeroconf client
String deviceDescription = "ESP8266SignalKClient";
// User configuration ends here

HTTPClient http;
WiFiClient client;

int signalkConnection = 0;
String signalkHostPort = "";
String signalkToken = "";
String signalkAccessRequestUrl = "";


/** Store signalk config to EEPROM */
void saveConfig() {
  char a[100]; char b[200]; char c[100];
  signalkHostPort.toCharArray(a, signalkHostPort.length() + 1);
  signalkToken.toCharArray(b, signalkToken.length() + 1);
  signalkAccessRequestUrl.toCharArray(c, signalkAccessRequestUrl.length() + 1);
  EEPROM.begin(1024);
  EEPROM.put(0, a);
  EEPROM.put(200, b);
  EEPROM.put(400, c);
  EEPROM.commit();
  EEPROM.end();
  loadConfig();
}


/** Load signalk config from EEPROM */
void loadConfig() {
  char a[100]; char b[200]; char c[100];
  EEPROM.begin(1024);
  EEPROM.get(0, a);
  EEPROM.get(200, b);
  EEPROM.get(400, c);
  signalkHostPort = String(a);
  signalkToken = String(b);
  signalkAccessRequestUrl = String(c);
  Serial.println("CONFIG: ");
  Serial.println("   signalkHostPort: " + signalkHostPort);
  Serial.println("   signalkToken: " + signalkToken);
  Serial.println("   signalkAccessRequestUrl: " + signalkAccessRequestUrl);
  EEPROM.end();
 
}

void setup() {
  Serial.begin(115200);

  loadConfig();
}


void startWifi() {
  Serial.print("\nConnecting");

  WiFi.mode(WIFI_STA);
  WiFi.begin(SSID, wifiPassPhrase);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.printf("\nConnected to WIFI; local IP ");
  Serial.println(WiFi.localIP());
}


void signalkRequestAccess() {
  Serial.println("requesting access");
  http.begin(client, String(signalkHostPort) + "/signalk/v1/access/requests");
  http.addHeader("Content-Type", "application/json");
  String httpRequestData = "{\"clientId\": \"ESP_" + String(ESP.getChipId()) + "\", \"description\": \"" + deviceDescription + "\"}";
  int httpCode = http.POST(httpRequestData);
  if (httpCode > 0) {
    if (httpCode == 202 || httpCode == 400) {
      String payload = http.getString();
      Serial.println(payload);
      DynamicJsonDocument parsed(1024);
      deserializeJson(parsed, payload);
      signalkAccessRequestUrl = String(parsed["href"]);
      Serial.println(signalkAccessRequestUrl);
      signalkToken = "";
      saveConfig(); //store the AccessRequestUrl in EEPROM
    }
  } else {
    Serial.printf("Error requesting access: %s\n", http.errorToString(httpCode).c_str());
  }
  http.end();
}


void signalkGetZeroConf() {
  Serial.println("signalkZeroConf");
  signalkHostPort = zeroConfHostPort;
  saveConfig();
}


void signalkCheckAccessRequest() {
  Serial.println("check access request");
  Serial.println(String(signalkHostPort) + String(signalkAccessRequestUrl));
  http.begin(client, signalkHostPort + signalkAccessRequestUrl);
  http.addHeader("Content-Type", "application/json");
  int httpCode = http.GET();
  if (httpCode == 200) {
    String payload = http.getString();
    Serial.println(payload);
    DynamicJsonDocument parsed(1024);
    deserializeJson(parsed, payload);
    String state = String(parsed["state"]);
    if (state == "COMPLETED") {
      String permission = String(parsed["accessRequest"]["permission"]);
      Serial.printf("accessRequest.permission %s \n", permission.c_str());
      if (permission == "APPROVED") {
        signalkToken = String(parsed["accessRequest"]["token"]);
        Serial.printf("accessRequest.signalkToken %s \n", signalkToken.c_str());
      }
      signalkAccessRequestUrl = "";
      saveConfig(); // Store the token in EEPROM
    }
  }
  http.end();
}


void signalkPutValue (String path, String value) {
  String mypath = path;
  mypath.replace(".", "/");
  String httpUrl = signalkHostPort + "/signalk/v1/api/vessels/self/" + mypath;
  String httpPutData = "{\"value\": \"" + value + "\"}";
  Serial.println ("signalkPutValue");
  Serial.println ("   httpUrl: " + httpUrl);
  Serial.println ("   httpPutData: " + httpPutData);

  http.begin(client, httpUrl);
  http.addHeader("Content-Type", "application/json");
  http.addHeader("Authorization", "Bearer " + signalkToken);
  
  int httpCode = http.PUT(httpPutData);
  if (httpCode > 0) {
    Serial.println ("   httpCode: " + String(httpCode));
    if (httpCode == 401) {
      signalkToken = "";
      saveConfig();
      Serial.println("*** Requesting new access token. Remember to approve with Read/Write permissions.");
    }
  }
  else {
    Serial.printf("Error during the HTTP PUT: %s\n", http.errorToString(httpCode).c_str());
    signalkHostPort = "";
  }
  http.end();
}

void signalkSendValue(String path, String value) {
  Serial.println("----------------------");
  if (WiFi.status() != WL_CONNECTED)
  startWifi();

  if (signalkHostPort == "") {
    signalkGetZeroConf();
  }
  else {
    if (signalkToken == "") {
      if (signalkAccessRequestUrl == "") {
        signalkRequestAccess();
      }
      else {
        signalkCheckAccessRequest();
      } 
    }
    else {
      signalkPutValue (path, value);
    }
  }
} 


void loop() {
  float value = float(analogRead(A0)) / 330;
  signalkSendValue("cell.voltage", String(value));
  delay(1000);
}
