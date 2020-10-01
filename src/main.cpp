#include <Arduino.h>
#include <ESP8266Wifi.h>
#include <WiFiUdp.h>
#include <FastLed.h>

#include "network.h"

#define LED_PIN 5
#define COLOR_ORDER GRB
#define CHIPSET WS2812B
#define NUM_LEDS 1

#define BRIGHTNESS 250
#define FRAMES_PER_SECOND 10

const uint16 udp_port = 25000;

WiFiUDP udp;


CRGB leds[NUM_LEDS];

void setup()
{
  Serial.begin(115200);
  WiFi.hostname(host_name);
  WiFi.begin(ssid, password);

  // put your setup code here, to run once:
  delay(3000); // sanity delay
  FastLED.addLeds<CHIPSET, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
  FastLED.setBrightness(BRIGHTNESS);

   while (WiFi.status() != WL_CONNECTED) { // Wait for the Wi-Fi to connect
    delay(500);
    Serial.print('.');
  }
 
  Serial.println('\n');
  Serial.println("Connection established!");  
  Serial.print("IP address:\t");
  Serial.println(WiFi.localIP());

  udp.begin(udp_port);
  Serial.println('\n');
  Serial.print("Started listening on UDP port : ");  
  Serial.println(udp_port);
  
}

int hue = 0;
unsigned char buffer[1024];

void loop()
{
  int packet_size = udp.parsePacket();
  if (packet_size > 0) {
    Serial.println("Got packet");
    int length = udp.read(buffer, 1024);
    if (length >= 3) {
      leds[0] = CRGB(buffer[0], buffer[1], buffer[2]);
      FastLED.show();
    }

  }
  delay(50);  
}