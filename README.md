# Control WS2812B LED (strip) from ESP8266 

Commands are received on a UDP port

## My DIY lamp 

The project is configured for the setup of my DIY Lamp/Qi Charger/Head-phone stand.

-- Insert image --

On the back there is xx leds and on the bottom of the top part there is yy leds. That is the reasone for the NUM
definitions:

```c
#define NUM_LEDS 18
#define NUM_GROUP1 14
#define NUM_GROUP2 4
```

Currently, three commands can be send to the lamp

1. One color.

2. Two colors, one for the first groupd and one for the second group.

3. A program.