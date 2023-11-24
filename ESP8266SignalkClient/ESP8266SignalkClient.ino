/**
   ESP8266SignalkClient.ino
    Marco Bergman
    Created on: 24-NOV-2023

*/

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>

// User configuration starts here
String wifiSsid        =  "openplotter";
String wifiPassword    =  "12345678";
String signalkIpString =  "10.10.10.1";
int    signalkUdpPort  =  30330;
String signalkSource   =  "ESP8266SignalkClient";
// User configuration ends here

WiFiClient client;
WiFiUDP udp;

IPAddress signalkIp;
bool x = signalkIp.fromString(signalkIpString);

int i = 1; // metadata counter


void setup() {
  Serial.begin(115200);
  startWifi();
  udp.begin(33333);
}


void startWifi() {
  Serial.print("\nConnecting");

  WiFi.mode(WIFI_STA);
  WiFi.begin(wifiSsid, wifiPassword);

  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
}


void signalkSendValue (String path, String value, String units) {
  String message = "{\"updates\":[{\"$source\": \""+ signalkSource + "\", \"values\":[{\"path\":\"" + path + "\",\"value\":" + value + "}]}]}";

  i -= 1; if (i == 0) {  // to start, then periodically, update the 'units' metadata:
    message = "{\"updates\":[{\"meta\":[{\"path\":\"" + path + "\",\"value\":{\"units\":\"" + units + "\"}}]}]}";
    i = 100;
  }
  Serial.println(message);

  udp.beginPacket(signalkIp, signalkUdpPort);
  udp.write(message.c_str());
  udp.endPacket();  
}


void loop() {
  float value = float(analogRead(A0))/30 ;
  signalkSendValue("cell.voltage", String(value), "V");
  delay(1000);
}
