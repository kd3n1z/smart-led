#include "config.h"
#include "leds.h"
#include <Arduino.h>

bool readSwitch() { return !digitalRead(SWITCH_PIN); }

void setupControls() {
    Serial.print("Initializing controls... ");

    pinMode(SWITCH_PIN, INPUT_PULLUP);
    setIsOn(readSwitch());

    Serial.println("done");
}

void updateControls() {
    bool currentReading = readSwitch();

    static unsigned long stateChangeTime = millis();
    static bool lastReading = currentReading;

    if (currentReading != lastReading &&
        millis() - stateChangeTime >= SWITCH_DEBOUNCE_TIME) {
        lastReading = currentReading;
        setIsOn(currentReading);
    }
}
