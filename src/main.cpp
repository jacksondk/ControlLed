#include <Arduino.h>
#include <ESP8266Wifi.h>
#include <WiFiUdp.h>
#include <FastLed.h>

#include "network.h"
#include "program.h"

#define LED_PIN 5
#define COLOR_ORDER GRB
#define CHIPSET WS2812B

#define WIFI_CONNECTED_LED 2
#define SETUP_RUNNING_LED 16

#define NUM_LEDS 18
#define NUM_GROUP1 14
#define NUM_GROUP2 4

#define BRIGHTNESS 250
#define FRAMES_PER_SECOND 10

const uint16 udp_port = 25000;

WiFiUDP udp;
IPAddress local_ip;
CRGB leds[NUM_LEDS];

uint8 command;

unsigned char buffer[1024];
Program program;

void setup()
{
  Serial.begin(115200);
  WiFi.hostname(host_name);
  WiFi.begin(ssid, password);

  pinMode(WIFI_CONNECTED_LED, OUTPUT);
  pinMode(SETUP_RUNNING_LED, OUTPUT);
  digitalWrite(WIFI_CONNECTED_LED, HIGH);
  digitalWrite(SETUP_RUNNING_LED, HIGH);
  // put your setup code here, to run once:
  delay(3000); // sanity delay

  FastLED.addLeds<CHIPSET, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
  FastLED.setBrightness(BRIGHTNESS);
  FastLED.showColor(CRGB(0, 0, 0));

  int count = 0;
  while (WiFi.status() != WL_CONNECTED)
  { // Wait for the Wi-Fi to connect
    delay(500);
    Serial.print('.');
    digitalWrite(WIFI_CONNECTED_LED, (count % 2) == 0 ? LOW : HIGH);
    count++;
  }

  digitalWrite(WIFI_CONNECTED_LED, HIGH); // The LED is inverted
  digitalWrite(SETUP_RUNNING_LED, HIGH);  // The LED is inverted

  Serial.println('\n');
  Serial.println("Connection established!");
  Serial.print("IP address:\t");
  Serial.println(WiFi.localIP());
  local_ip = WiFi.localIP();

  udp.begin(udp_port);
  Serial.println('\n');
  Serial.print("Started listening on UDP port : ");
  Serial.println(udp_port);

  // Test program
  for (int i = 0; i < 32; i++) {
    program.AddProgramSetting(200, CHSV((i*10)%256, 200, 200), CHSV((i*10+100)%256, 200, 200));
  }
  command = 3;
}

void setAllSameColor(byte red, byte green, byte blue)
{
  FastLED.showColor(CRGB(red, green, blue));
}

void setColorsTwoSets(CRGB color1, CRGB color2)
{
  for (int index = 0; index < NUM_LEDS; index++)
  {
    leds[index] = (index < NUM_GROUP1) ? color1 : color2;
  }
  FastLED.show();
}


uint32 convertToUInt32(uint8 *buffer)
{
  return buffer[0] * 0x01000000 +
         buffer[1] * 0x00010000 +
         buffer[2] * 0x00000100 +
         buffer[3];
}


void handleNetwork()
{
  int packet_size = udp.parsePacket();
  if (packet_size > 0)
  {
    digitalWrite(WIFI_CONNECTED_LED, LOW);
    int length = udp.read(buffer, 1024);
    IPAddress remote = udp.remoteIP();
    command = buffer[0];
    // Debug output
    Serial.print("Got command ");
    Serial.print(command);
    Serial.print(" length ");
    Serial.println(length);

    switch (command)
    {
    case 0:
    {
      udp.beginPacket(remote, udp_port);
      udp.write((const char *)&(local_ip.v4()), 4);
      udp.endPacket();
    }
    case 1:
      if (length == 4)
      {
        setAllSameColor(buffer[1], buffer[2], buffer[3]);
      }
      break;
    case 2:
      if (length == 7)
      {
        setColorsTwoSets(CRGB(buffer[1], buffer[2], buffer[3]),
                         CRGB(buffer[4], buffer[5], buffer[6]));
      }
      break;
    case 3: // Program
    {
      uint32 elements = convertToUInt32(buffer + 1);
      // Debug output
      Serial.print("Program with ");
      Serial.print(elements);
      Serial.print(" elements");

      program.Reset();
      for (uint32 index = 0; index < elements; index++)
      {
        int buffer_index = 4 + index * (4 + 3 + 3);

        uint32 ms = convertToUInt32(buffer + buffer_index);
        CRGB color1 = CRGB(buffer[buffer_index + 4], buffer[buffer_index + 5], buffer[buffer_index + 6]);
        CRGB color2 = CRGB(buffer[buffer_index + 7], buffer[buffer_index + 8], buffer[buffer_index + 9]);
        program.AddProgramSetting(ms, color1, color2);
      }
    }
    break;
    }
    digitalWrite(WIFI_CONNECTED_LED, HIGH);
  }
}

void runProgram(int tick_ms)
{
  if (command == 3)
  {
    if (program.Tick(tick_ms))
    {
      setColorsTwoSets(program.Color1(), program.Color2());
    }
  }
}

#define TICK 50
void loop()
{
  handleNetwork();
  runProgram(TICK);
  delay(TICK);
}