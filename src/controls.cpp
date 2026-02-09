#include "config.h"
#include <Arduino.h>

bool readSwitch() { return !digitalRead(SWITCH_PIN); }

unsigned long fadeStartTime;
bool isOn;

void setupControls() {
    Serial.print("Initializing controls... ");

    pinMode(SWITCH_PIN, INPUT_PULLUP);

    if (readSwitch()) {
        isOn = true;
        fadeStartTime = millis();
    } else {
        isOn = false;
        fadeStartTime = millis() - FADE_DURATION;
    }

    Serial.println("done");
}

void updateControls() {
    static unsigned long stateChangeTime = millis();
    static bool lastReading = false;
    bool currentReading = readSwitch();

    if (currentReading != lastReading) {
        lastReading = currentReading;
        stateChangeTime = millis();
    }

    if (isOn != currentReading &&
        millis() - stateChangeTime >= SWITCH_DEBOUNCE_TIME) {
        isOn = currentReading;
        fadeStartTime = millis();
    }
}

uint8_t getPhysicalBrightness() {
    unsigned long timePassed = millis() - fadeStartTime;

    if (timePassed >= FADE_DURATION) {
        return isOn ? 255 : 0;
    }

    uint8_t value = (uint32_t)timePassed * 255 / FADE_DURATION;
    return isOn ? value : 255 - value;
}
