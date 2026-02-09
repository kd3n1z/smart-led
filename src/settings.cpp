#include "settings.h"
#include "config.h"
#include <Arduino.h>
#include <EEPROM.h>

#define TEST_NUMBER 2

SettingsData settings;
bool ledsUnsynced = true;

void setupSettings() {
    Serial.print("Reading settings from EEPROM... ");
    EEPROM.begin(sizeof(settings));
    EEPROM.get(0, settings);

    if (settings.testNumber != TEST_NUMBER) {
        Serial.println("invalid test number (got " +
                       String(settings.testNumber) + ", expected " +
                       String(TEST_NUMBER) + "), setting defaults");
        settings.testNumber = TEST_NUMBER;
        settings.solidRed = 255;
        settings.solidGreen = 255;
        settings.solidBlue = 255;
        settings.brightness = 255;

        markSettingsDirty();
    } else {
        Serial.println("done");
    }
}

bool isDirty = false;
unsigned long lastDirtyTime = 0;

void updateSettings() {
    if (isDirty && millis() > lastDirtyTime + EEPROM_DEBOUNCE_TIME) {
        Serial.print("Putting settings into EEPROM... ");

        EEPROM.put(0, settings);
        EEPROM.commit();
        isDirty = false;

        Serial.println("done");
    }
}

void markSettingsDirty() {
    lastDirtyTime = millis();
    isDirty = true;
    ledsUnsynced = true;
}
