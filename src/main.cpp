#include "controls.h"
#include "leds.h"
#include "server.h"
#include "settings.h"
#include <Arduino.h>

void setup() {
    Serial.begin(115200);

    setupServer();
    setupSettings();
    setupControls();
    setupLeds();

    Serial.println("Setup finished");
}

void loop() {
    updateServer();
    updateSettings();
    updateControls();
    updateLeds();
}
