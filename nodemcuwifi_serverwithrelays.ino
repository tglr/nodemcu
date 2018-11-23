#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include "DHT.h"

#define DHTTYPE DHT11   // DHT 11
// DHT Sensor
const int DHTPin = 5;
const int PIRPin = 4;
// Initialize DHT sensor.
DHT dht(DHTPin, DHTTYPE);
byte mac[6];
const char* ssid = "Guler";
const char* password = "melisglr2403$1";
char mainserver[] = "192.168.1.100";
int updtmr = 0;
int srvupdtmr = 0;
String cmd;
String devicemac;
ESP8266WebServer server(80);
WiFiClient client;
int val;
int val2;
const int output1 = 14;
const int output2 = 12;
const int output3 = 13;
const int output4 = 15;

// Temporary variables
static char celsiusTemp[7];
static char humidityTemp[7];
boolean relay1 = false;
boolean relay2 = false;
boolean relay3 = false;
boolean relay4 = false;

void handleRoot() {
  //digitalWrite(led, 1);
  //server.send(200, "text/plain", "hello from esp8266!");
  //digitalWrite(led, 0);
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  float h = dht.readHumidity();
  // Read temperature as Celsius (the default)
  float t = dht.readTemperature();
  // Check if any reads failed and exit early (to try again).
  if (isnan(h) || isnan(t) ) {
    Serial.println("Failed to read from DHT sensor!");
    strcpy(celsiusTemp, "Failed");
    strcpy(humidityTemp, "Failed");
  }
  else {
    // Computes temperature values in Celsius + Fahrenheit and Humidity
    float hic = dht.computeHeatIndex(t, h, false);
    dtostrf(hic, 6, 2, celsiusTemp);
    dtostrf(h, 6, 2, humidityTemp);
  }

  cmd = "{";

  if (relay1) {
    cmd += "\"relay1\":\"on\"";
  }
  else {
    cmd += "\"relay1\":\"off\"";
  }

  if (relay2) {
    cmd += ",\"relay2\":\"on\"";
  }
  else {
    cmd += ",\"relay2\":\"off\"";
  }

  if (relay3) {
    cmd += ",\"relay3\":\"on\"";
  }
  else {
    cmd += ",\"relay3\":\"off\"";
  }

  if (relay4) {
    cmd += ",\"relay4\":\"on\"";
  }
  else {
    cmd += ",\"relay4\":\"off\"";
  }
  cmd += ",\"temperature\" :\"";
  cmd += celsiusTemp;
  cmd += "\"";
  cmd += ",\"humidity\" :\"";
  cmd += humidityTemp;
  cmd += "\"";

  int sensorValue = analogRead(A0);   // read the input on analog pin 0
  float voltage = sensorValue * (3.3 / 1023.0);   // Convert the analog reading (which goes from 0 - 1023) to a voltage (0 - 5V)
  //Serial.println(voltage);
  cmd += ",\"ldrvoltage\" :\"";
  cmd += voltage;
  cmd += "\"}";
  server.send(200, "application/json", cmd);
  //sendtoserver();
}

void handleNotFound() {
  //digitalWrite(led, 1);
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
  //digitalWrite(led, 0);
}

void setup(void) {
  Serial.begin(9600);
  Serial.println("started");
  pinMode(output1, OUTPUT);
  pinMode(output2, OUTPUT);
  pinMode(output3, OUTPUT);
  pinMode(output4, OUTPUT);

  digitalWrite(output1, LOW);
  digitalWrite(output2, LOW);
  digitalWrite(output3, LOW);
  digitalWrite(output4, LOW);
  dht.begin();

  WiFi.begin(ssid, password);
  Serial.println("");
  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");

  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  if (MDNS.begin("esp8266")) {
    Serial.println("MDNS responder started");
  }

  server.on("/", handleRoot);

  server.on("/relay1=1", []() {
    server.send(200, "application/json", "{\"relay1\" : \"on\"}");
    digitalWrite(output1, HIGH);
    relay1 = true;
  });

  server.on("/relay1=0", []() {
    server.send(200, "application/json", "{\"relay1\" : \"off\"}");
    digitalWrite(output1, LOW);
    relay1 = false;
  });

  server.on("/relay2=1", []() {
    server.send(200, "application/json", "{\"relay2\" : \"on\"}");
    digitalWrite(output2, HIGH);
    relay2 = true;
  });

  server.on("/relay2=0", []() {
    server.send(200, "application/json", "{\"relay2\" : \"off\"}");
    digitalWrite(output2, LOW);
    relay2 = false;
  });

  server.on("/relay3=1", []() {
    server.send(200, "application/json", "{\"relay3\" : \"on\"}");
    digitalWrite(output3, HIGH);
    relay3 = true;
  });

  server.on("/relay3=0", []() {
    server.send(200, "application/json", "{\"relay3\" : \"off\"}");
    digitalWrite(output3, LOW);
    relay3 = false;
  });

  server.on("/relay4=1", []() {
    server.send(200, "application/json", "{\"relay4\" : \"on\"}");
    digitalWrite(output4, HIGH);
    relay4 = true;
  });

  server.on("/relay4=0", []() {
    server.send(200, "application/json", "{\"relay4\" : \"off\"}");
    digitalWrite(output4, LOW);
    relay4 = false;
  });

  server.onNotFound(handleNotFound);
  server.begin();
  Serial.println("HTTP server started");
  updateserver();
}
void sendtoserver() {
  //digitalWrite(led, 1);
  //server.send(200, "text/plain", "hello from esp8266!");
  //digitalWrite(led, 0);
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  float h = dht.readHumidity();
  // Read temperature as Celsius (the default)
  float t = dht.readTemperature();
  // Check if any reads failed and exit early (to try again).
  if (isnan(h) || isnan(t) ) {
    Serial.println("Failed to read from DHT sensor!");
    strcpy(celsiusTemp, "Failed");
    strcpy(humidityTemp, "Failed");
  }
  else {
    // Computes temperature values in Celsius + Fahrenheit and Humidity
    float hic = dht.computeHeatIndex(t, h, false);
    dtostrf(hic, 6, 2, celsiusTemp);
    dtostrf(h, 6, 2, humidityTemp);
  }
  WiFi.macAddress(mac);
  cmd = "deviceMAC=";
  cmd += devicemac;
  cmd += "&";
  if (relay1) {
    cmd += "relay1=on";
  }
  else {
    cmd += "relay1=off";
  }
  cmd += "&";
  if (relay2) {
    cmd += "relay2=on";
  }
  else {
    cmd += "relay2=off";
  }
  cmd += "&";
  if (relay3) {
    cmd += "relay3=on";
  }
  else {
    cmd += "relay3=off";
  }
  cmd += "&";
  if (relay4) {
    cmd += "relay4=on";
  }
  else {
    cmd += "relay4=off";
  }
  cmd += "&temperature=";
  cmd += celsiusTemp;
  cmd += "&humidity=";
  cmd += humidityTemp;
  int sensorValue = analogRead(A0);   // read the input on analog pin 0
  float voltage = sensorValue * (3.3 / 1023.0);   // Convert the analog reading (which goes from 0 - 1023) to a voltage (0 - 5V)
  cmd += "&ldrvoltage=";
  cmd += voltage;
  if (client.connect(mainserver, 1880)) {
    Serial.println("Connected to server");
    client.println("POST /homedata HTTP/1.1");
    client.print("Content-Length: ");
    client.println (cmd.length());
    client.println("Content-Type: application/x-www-form-urlencoded");
    client.println();
    client.println(cmd);
  };
}
void updateserver() {
  IPAddress ip = WiFi.localIP();
  WiFi.macAddress(mac);
  Serial.println("Registering to server");
  if (client.connect(mainserver, 1880)) {
    Serial.println("Connected to server");
    // Make a HTTP request
    devicemac = String(mac[5], HEX);
    devicemac += "-";
    devicemac += String(mac[4], HEX);
    devicemac += "-";
    devicemac += String(mac[3], HEX);
    devicemac += "-";
    devicemac += String(mac[2], HEX);
    devicemac += "-";
    devicemac += String(mac[1], HEX);
    devicemac += "-";
    devicemac += String(mac[0], HEX);
    cmd = "deviceMAC=";
    cmd += devicemac;
    cmd += "&deviceIP=";
    cmd += ip[0];
    cmd += ".";
    cmd += ip[1];
    cmd += ".";
    cmd += ip[2];
    cmd += ".";
    cmd += ip[3];
    Serial.println (cmd);
    client.println("POST /homeregister HTTP/1.1");
    client.print("Content-Length: ");
    client.println(cmd.length());
    client.println("Content-Type: application/x-www-form-urlencoded");
    client.println();
    client.print(cmd);
    client.println();
  }
}

void sendevent(String command) {
  if (client.connect(mainserver, 1880)) {
    Serial.println("Connected to server");
    cmd = "deviceMAC=";
    cmd += devicemac;
    cmd +="&";
    cmd += command;
    Serial.println (cmd);
    client.println("POST /homeevent HTTP/1.1");
    client.print("Content-Length: ");
    client.println(cmd.length());
    client.println("Content-Type: application/x-www-form-urlencoded");
    client.println();
    client.print(cmd);
    client.println();
  }
}

void loop(void) {
  server.handleClient();
  updtmr++;
  if (updtmr >= 6000000) { //send sensor and relay data to server 6.000.000 for  ~1 min
    updtmr = 0;
    srvupdtmr++;
    sendtoserver();
  }
  if (srvupdtmr >= 15) { //send IP & MAC to server
    srvupdtmr = 0;
    updateserver();
  }
  val = digitalRead(PIRPin); //read state of the PIR
  if (val != val2) {
    if (val == LOW) {
      sendevent("event=motion&status=stop");
    }
    else {
      sendevent("event=motion&status=alert");
    }
  }
  val2 = val;
}



