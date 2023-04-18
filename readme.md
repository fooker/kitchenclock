# Kitchen Clock

A DIY clock using ws2812 (neopixel) matrix panels in a 3D printed case controlled by a ESP8266.

## Description

Display consists of 24x8 RGB pixels. To defuse the pixels, the case is printed with a white front panel.
The LEDs are controlled by the ESP8266 controller which fetches the current time using NTP.
To get a clear display at day and night, a LDR is used to sense the surrounding illumination and manages brightness accordingly.

## Hardware
- 3x 8x8 Neopixel matrix panels
- Wemos D1 mini
- 10x photoresistor

## Configuration
The project is configured by creating a `config.h` file in the root of the project.
It must contain the following defines:
```
#define WIFI_SSID "my-wifi-ssid"
#define WIFI_PASS "my-wifi-password"
```

