# smart-led

Firmware for controlling smart LED strip. Initially developed for NodeMCU v3.

## Hardware

-   NodeMCU v3 (or any other controller on ESP8266)
-   WS2812B LED strip (you can use any other, but you will have to adjust the [leds.cpp](./src/leds.cpp))
-   5V 2-3A Power supply (each LED consumes ~60 milliamps at its whitest color, so you might need to increase the current)

    **If the controller is connected via USB, disconnect VIN from the power supply. Keep GND common to the strip and the controller; otherwise, the control pin may become erratic.**

<img src="images/schematics.png"/>

## Usage

-   Define your Wi-Fi SSID & password in the [secrets.h](./include/secrets.h)
