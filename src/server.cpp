#include "config.h"
#include "secrets.h"
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

    CMD_SET_COLOR = 2,
    CMD_SET_BRIGHTNESS = 3
};

#define REQUIRE_ARGS(name, count)                                              \
    if (i + count >= len)                                                      \
        return;                                                                \
    uint8_t name[count];                                                       \
    for (int k = 0; k < count; k++)                                            \
        name[k] = data[++i];

void handleApiRequest() {
    WiFiClient client = server.client();

    if (!server.hasArg("plain")) {
        server.send(400, "text/plain", "no data provided");
        return;
    }

    server.sendHeader("Access-Control-Allow-Origin", "*", true);
    server.setContentLength(CONTENT_LENGTH_UNKNOWN);
    server.send(200, "application/octet-stream", "");

    String data = server.arg("plain");
    int len = data.length();

    for (int i = 0; i < len; i++) {
        uint8_t cmd = data[i];

        switch (cmd) {
        case CMD_GET_COLOR: {
            uint8_t resp[] = {255, 0, 123};
            server.sendContent((const char *)resp, 3);
            break;
        }
        case CMD_GET_BRIGHTNESS: {
            uint8_t resp[] = {99};
            server.sendContent((const char *)resp, 1);
            break;
        }
        case CMD_SET_COLOR: {
            REQUIRE_ARGS(rgb, 3);
            Serial.print("rgb: ");
            Serial.print(rgb[0]);
            Serial.print(", ");
            Serial.print(rgb[1]);
            Serial.print(", ");
            Serial.println(rgb[2]);
            break;
        }
        case CMD_SET_BRIGHTNESS: {
            REQUIRE_ARGS(brightness, 1);
            Serial.print("brightness: ");
            Serial.println(brightness[0]);
            break;
        }
        default:
            return;
        }
    }
}
