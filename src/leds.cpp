#include "FastLED.h"
#include "config.h"
#include "controls.h"
#include "settings.h"

CRGB solidLeds[NUM_LEDS];

void syncWithSettings() {
    Serial.print("Syncing LEDs with settings... ");

    for (int i = 0; i < NUM_LEDS; i++) {
        solidLeds[i].setRGB(settings.solidRed, settings.solidGreen,
                            settings.solidBlue);
    }

    Serial.println("done");
}

void setupLeds() {
    Serial.print("Initializing FastLED... ");

    FastLED.addLeds<WS2812B, DATA_PIN, GRB>(solidLeds, NUM_LEDS);

    Serial.println("done");
}

void updateLeds() {
    if (ledsUnsynced) {
        syncWithSettings();
        ledsUnsynced = false;
    }

    FastLED.setBrightness((uint32_t)settings.brightness *
                          getPhysicalBrightness() / 255);
    FastLED.show();
}
