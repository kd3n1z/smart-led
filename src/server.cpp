#include "config.h"
#include "controls.h"
#include "frontend.h"
#include "leds.h"
#include "secrets.h"
#include "settings.h"
#include <ESP8266WebServer.h>
#include <ESP8266WiFi.h>

ESP8266WebServer server(HTTP_SERVER_PORT);

IPAddress local_ip(WIFI_IP);
IPAddress gateway(WIFI_GATEWAY);
IPAddress subnet(WIFI_SUBNET);

void handleApiRequest();

void setupServer() {
    Serial.print("Initializing server & Wi-Fi... ");

    server.on("/api", handleApiRequest);

    setupFrontend();

    WiFi.mode(WIFI_STA);
    WiFi.config(local_ip, gateway, subnet);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    WiFi.setAutoReconnect(true);

    Serial.println("done");
}

void updateServer() {
    static bool wasConnected = false;

    bool isConnected = WiFi.status() == WL_CONNECTED;

    if (isConnected) {
        if (!wasConnected) {
            Serial.println("Wi-Fi connected, server started");
            server.begin();
            wasConnected = true;
        }
        server.handleClient();
    } else if (wasConnected) {
        Serial.println("Wi-Fi disconnected, server stopped");
        server.stop();
        wasConnected = false;
    }
}

enum Cmd : uint8_t {
    CMD_GET_COLOR = 0,
    CMD_GET_BRIGHTNESS = 1,
    CMD_GET_IS_ON = 2,

    CMD_SET_COLOR = 255,
    CMD_SET_BRIGHTNESS = 254,
    CMD_SET_IS_ON = 253
};

#define REQUIRE_ARGS(name, count)                                              \
    if (i + count >= len) {                                                    \
        statusCode = 3;                                                        \
        goto end;                                                              \
    }                                                                          \
    uint8_t name[count];                                                       \
    for (int k = 0; k < count; k++)                                            \
        name[k] = data[++i];

void handleApiRequest() {
    WiFiClient client = server.client();

    server.sendHeader("Access-Control-Allow-Origin", "*", true);
    server.setContentLength(CONTENT_LENGTH_UNKNOWN);
    server.send(200, "application/octet-stream", "");

    // 0 - ok
    // 1 - no data provided
    // 2 - unknown command
    // 3 - not enough args
    uint8_t statusCode = 0;

    String data;
    int len;

    if (!server.hasArg("plain")) {
        statusCode = 1;
        goto end;
    }

    data = server.arg("plain");
    len = data.length();

    for (int i = 0; i < len; i++) {
        uint8_t cmd = data[i];

        switch (cmd) {
        case CMD_GET_COLOR: {
            uint8_t resp[] = {settings.solidRed, settings.solidGreen,
                              settings.solidBlue};
            server.sendContent((const char *)resp, 3);
            break;
        }
        case CMD_GET_BRIGHTNESS: {
            uint8_t resp[] = {settings.brightness};
            server.sendContent((const char *)resp, 1);
            break;
        }
        case CMD_GET_IS_ON: {
            uint8_t resp[] = {getIsOn()};
            server.sendContent((const char *)resp, 1);
            break;
        }
        case CMD_SET_COLOR: {
            REQUIRE_ARGS(args, 3);
            if (settings.solidRed != args[0] || settings.solidRed != args[1] ||
                settings.solidBlue != args[2]) {
                settings.solidRed = args[0];
                settings.solidGreen = args[1];
                settings.solidBlue = args[2];
                markSettingsDirty();
            }
            break;
        }
        case CMD_SET_BRIGHTNESS: {
            REQUIRE_ARGS(args, 1);
            if (settings.brightness != args[0]) {
                settings.brightness = args[0];
                markSettingsDirty();
            }
            break;
        }
        case CMD_SET_IS_ON:
            REQUIRE_ARGS(args, 1);
            setIsOn(args[0]);
            break;
        default:
            statusCode = 2;
            goto end;
        }
    }

end:
    uint8_t footer[] = {statusCode};
    server.sendContent((const char *)footer, 1);
}
