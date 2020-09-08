/*

  RS485_HalfDuplex.pde - example using ModbusMaster library to communicate
  with EPSolar LS2024B controller using a half-duplex RS485 transceiver.

  This example is tested against an EPSolar LS2024B solar charge controller.
  See here for protocol specs:
  http://www.solar-elektro.cz/data/dokumenty/1733_modbus_protocol.pdf

  Library:: ModbusMaster
  Author:: Marius Kintel <marius at kintel dot net>

  Copyright:: 2009-2016 Doc Walker

  Licensed under the Apache License, Version 2.0 (the "License");
  you may not use this file except in compliance with the License.
  You may obtain a copy of the License at

      http://www.apache.org/licenses/LICENSE-2.0

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

*/

#include <ModbusMaster.h>
#include <AntaresESP32MQTT.h>

#define ACCESSKEY "a08c0faa1b988b11:9ae97d4dee93a723"
#define WIFISSID "takkusangka"
#define PASSWORD "anakrantau"

#define projectName "5f0e7893658acc00108aa821"
#define deviceName "1598795191613845242"

AntaresESP32MQTT antares(ACCESSKEY);

#include <WiFi.h>
#include <WiFiClient.h>

#define MAX485_DE      18
#define MAX485_RE_NEG  19

// instantiate ModbusMaster object
ModbusMaster node;

// Your WiFi credentials.
// Set password to "" for open networks.
char ssid[] = "takkusangka";
char pass[] = "anakrantau";

bool state = true;
uint8_t result;
float bat1,bat2,temp1,temp2;

void preTransmission()
{
  digitalWrite(MAX485_RE_NEG, 1);
  digitalWrite(MAX485_DE, 1);
}

void postTransmission()
{
  digitalWrite(MAX485_RE_NEG, 0);
  digitalWrite(MAX485_DE, 0);
}

void setup()
{
  Serial.begin(19200);
  WiFi.begin(ssid, pass);
  int wifi_ctr = 0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("WiFi connected");  

  antares.setDebug(true);
  antares.wifiConnection(WIFISSID, PASSWORD);
  antares.setMqttServer();
  
  pinMode(MAX485_RE_NEG, OUTPUT);
  pinMode(MAX485_DE, OUTPUT);
  // Init in receive mode
  digitalWrite(MAX485_RE_NEG, 0);
  digitalWrite(MAX485_DE, 0);  

  // Modbus slave ID 1
  node.begin(1, Serial);
  // Callbacks allow us to configure the RS485 transceiver correctly
  node.preTransmission(preTransmission);
  node.postTransmission(postTransmission);
  
}


void loop()
{ 
  result = node.readInputRegisters(0x30003, 4); //voltage
  
  if (result == node.ku8MBSuccess)
  {
    bat1=node.getResponseBuffer(0)/1000.0f;
    bat2=node.getResponseBuffer(1)/1000.0f;
    temp1=node.getResponseBuffer(2)/10.0f;
    temp2=node.getResponseBuffer(3)/10.0f;
  }
  
  antares.checkMqttConnection();
  
  Serial.print("Battery 1= ");
  Serial.println(bat1);
  Serial.print("Battery 2= ");
  Serial.println(bat2);
  Serial.print("Battery Temperature 1= ");
  Serial.println(temp1);
  Serial.print("Battery Temperature 2= ");
  Serial.println(temp2);
  
  antares.add("T",temp1);
  antares.add("V",bat1);
  antares.publish(projectName, deviceName);
  
  delay(60000);
}
