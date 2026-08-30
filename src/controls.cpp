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
    unsigned long currentMillis = millis();

    static bool stableReading = currentReading;
    static bool previousReading = currentReading;
    static unsigned long stateChangeTime = currentMillis;

    if (currentReading != previousReading) {
        previousReading = currentReading;
        stateChangeTime = currentMillis;
    }

    if (stableReading != currentReading &&
        currentMillis - stateChangeTime >= SWITCH_DEBOUNCE_TIME) {
        stableReading = currentReading;
        setIsOn(stableReading);
    }
}
