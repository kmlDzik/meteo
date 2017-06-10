
#include <SPI.h>
#include <Ethernet.h>
#include <DHT.h>
#include <Wire.h>
#include "Adafruit_BMP085.h" // This is the version 1 library
#define DHTPIN A0
#define DHTTYPE 22// The Temperature/Humidity sensor
Adafruit_BMP085 bmp;
DHT dht(DHTPIN, DHTTYPE);

// Local Network Settings
byte mac[] = { 0xD4, 0x28, 0xB2, 0xFF, 0xA0, 0xA1 }; // Must be unique on local network
// ThingSpeak Settings
char thingSpeakAddress[] = "api.thingspeak.com";
String writeAPIKey = "GHAGS5F1IPQCATXG";
const int updateThingSpeakInterval = 16 * 1000; // Time interval in milliseconds to update ThingSpeak (number of seconds * 1000 = interval)
// Variable Setup
long lastConnectionTime = 0;
boolean lastConnected = false;
int failedCounter = 0;
// Initialize Arduino Ethernet Client
EthernetClient client;
void setup()
{
  // Start Serial for debugging on the Serial Monitor
  Serial.begin(9600);
  // Start Ethernet on Arduino
  startEthernet();
}
void loop()
{
  

  if (client.available())
  {
    char c = client.read();
    Serial.print(c);
  }


delay(500);
//------DHT11--------
int chk = dht.read(DHTPIN);
char t_buffer[10];
char h_buffer[10];
float t=dht.readTemperature();
String temp=dtostrf(t,0,5,t_buffer);
//Serial.println(temp);
//Serial.print(" ");
float h = dht.readHumidity();
String humid = dtostrf(h, 0, 2, h_buffer);
Serial.println(humid);

//-----BMP180-----------
bmp.begin();
float p = (bmp.readPressure() / 100.0 + 4.14); //this is for pressure in hectoPascal
//float t2 = (bmp.readTemperature());

char p_buffer[15];
//char t2_buffer[15];
String pres = dtostrf(p, 0, 2, p_buffer);
//String temp2 = dtostrf(t2, 0, 2, t2_buffer);

Serial.println(pres);
Serial.println(t + 0.1);
Serial.println("------------------------");
//         }
//----------------
delay(500);
// Disconnect from ThingSpeak
if (!client.connected() && lastConnected)
{
  Serial.println("...disconnected");
  Serial.println();
  client.stop();
}
// Update ThingSpeak
if (!client.connected() && (millis() - lastConnectionTime > updateThingSpeakInterval))
{
  updateThingSpeak("field1=" + temp + "&field2=" + humid + "&field3=" + pres);
}
// Check if Arduino Ethernet needs to be restarted
if (failedCounter > 3 ) {
  startEthernet();
}
lastConnected = client.connected();
}
void updateThingSpeak(String tsData)
{
  if (client.connect(thingSpeakAddress, 80))
  {
       client.print("POST /update HTTP/1.1\n");
    client.print("Host: api.thingspeak.com\n");
    client.print("Connection: close\n");
    client.print("X-THINGSPEAKAPIKEY: " + writeAPIKey + "\n");
    client.print("Content-Type: application/x-www-form-urlencoded\n");
    client.print("Content-Length: ");
    client.print(tsData.length());
    client.print("\n\n");
    client.print(tsData);
    lastConnectionTime = millis();
    
    
    if (client.connected())
    {
      Serial.println("Connecting to ThingSpeak...");
      Serial.println();
      failedCounter = 0;
    }
    else
    {
      failedCounter++;
      Serial.println("Connection to ThingSpeak failed (" + String(failedCounter, DEC) + ")");
      Serial.println();
    }
  }
  else
  {
    failedCounter++;
    Serial.println("Connection to ThingSpeak Failed (" + String(failedCounter, DEC) + ")");
    Serial.println();
    lastConnectionTime = millis();
  }
}
void startEthernet()
{
  client.stop();
  Serial.println("Connecting Arduino to network...");
  Serial.println();
  delay(1000);
  // Connect to network amd obtain an IP address using DHCP
  if (Ethernet.begin(mac) == 0)
  {
    Serial.println("DHCP Failed, reset Arduino to try again");
    Serial.println();
  }
  else
  {
    Serial.println("Arduino connected to network using DHCP");
    Serial.println();
  }
  delay(1000);
}

