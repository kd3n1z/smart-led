#ifndef SMART_LED_SETTINGS
#define SMART_LED_SETTINGS

#include <Arduino.h>

struct SettingsData {
    int testNumber;
    uint8_t solidRed;
    uint8_t solidGreen;
    uint8_t solidBlue;
    uint8_t brightness;
};

extern SettingsData settings;
extern bool ledsUnsynced;

void setupSettings();
void updateSettings();
void markSettingsDirty();

#endif
