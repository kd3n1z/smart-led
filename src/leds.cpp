#include "FastLED.h"
#include "config.h"
#include "controls.h"
#include "settings.h"

CRGB solidLeds[NUM_LEDS];

bool isOn;
unsigned long fadeStartTime;

void setupLeds() {
    Serial.print("Initializing LEDs... ");

    FastLED.addLeds<WS2812B, DATA_PIN, GRB>(solidLeds, NUM_LEDS);

    fadeStartTime = isOn ? millis() : millis() - FADE_DURATION;

    Serial.println("done");
}

void setIsOn(bool value) {
    if (isOn == value) {
        return;
    }

    isOn = value;
    fadeStartTime = millis();
}

bool getIsOn() { return isOn; }

uint8_t getBrightness() {
    unsigned long timePassed = millis() - fadeStartTime;

    if (timePassed >= FADE_DURATION) {
        return isOn ? 255 : 0;
    }

    uint8_t value = (uint32_t)timePassed * 255 / FADE_DURATION;
    return isOn ? value : 255 - value;
}

void syncWithSettings() {
    Serial.print("Syncing LEDs with settings... ");

    for (int i = 0; i < NUM_LEDS; i++) {
        solidLeds[i].setRGB(settings.solidRed, settings.solidGreen,
                            settings.solidBlue);
    }

    Serial.println("done");
}

void updateLeds() {
    if (ledsUnsynced) {
        syncWithSettings();
        ledsUnsynced = false;
    }

    FastLED.setBrightness((uint32_t)settings.brightness * getBrightness() /
                          255);
    FastLED.show();
}
