#include <FastLED.h>
#include <SPIFFS.h>
#include <WiFi.h>
#include <WiFiSettings.h>
#include <ArduinoOTA.h>
#include <HTTPClient.h>
#include <tr064.h>

const int ledpin  = 27;
const int numleds = 25;
const int buttonpin = 39;
const int debounce_threshhold = 10;

enum button_state_t {
    NOT_PRESSED = 0,
    JUST_PRESSED,
    HELD,
    JUST_RELEASED,
};

bool button_state;
int debounce_rounds;

String to_call;
String fritzbox_username;
String fritzbox_password;
String fritzbox_ip;
int fritzbox_port;

CRGB leds[numleds];
WiFiClient wificlient;
TR064 *connection;

void setup_ota()
{
    ArduinoOTA.setHostname(WiFiSettings.hostname.c_str());
    ArduinoOTA.setPassword(WiFiSettings.password.c_str());
    ArduinoOTA.begin();
}

void setup_fritzbox()
{
    connection = new TR064(fritzbox_port, fritzbox_ip, fritzbox_username, fritzbox_password);
    connection->debug_level = -1;
    connection->init();
}

void setup()
{
    button_state = false;
    debounce_rounds = 0;

    FastLED.addLeds<WS2812B, ledpin, GRB>(leds, numleds);
    FastLED.setBrightness(20);

    SPIFFS.begin(true);
    pinMode(buttonpin, INPUT);

    to_call = WiFiSettings.string("to_call", 1, 32, "**610");
    fritzbox_username = WiFiSettings.string("fritzbox_username", 64, "admin");
    fritzbox_password = WiFiSettings.string("fritzbox_password", 64, "");
    fritzbox_ip = WiFiSettings.string("fritzbox_ip", 7, 15, "10.10.10.7");
    fritzbox_port = WiFiSettings.integer("fritzbox_port", 49000);

    WiFiSettings.onWaitLoop = []() {
        static CHSV color(0, 255, 255);
        color.hue += 10;
        FastLED.showColor(color);
        if (!digitalRead(buttonpin)) WiFiSettings.portal();
        return 50;
    };

    WiFiSettings.onSuccess = []() {
        FastLED.showColor(CRGB::Yellow);
        setup_fritzbox();

        FastLED.showColor(CRGB::Blue);
        setup_ota();

        FastLED.showColor(CRGB::Green);
        delay(200);
    };

    WiFiSettings.onPortal = []() {
        setup_ota();
    };

    WiFiSettings.onPortalWaitLoop = []() {
        static CHSV color(0, 255, 255);
        color.saturation--;
        FastLED.showColor(color);
        ArduinoOTA.handle();
    };

    WiFiSettings.connect();
}

enum button_state_t check_button()
{
    bool inst_button_state = !digitalRead(buttonpin);
    if (inst_button_state != button_state) {
        debounce_rounds++;
    } else if (debounce_rounds > 0) {
        debounce_rounds--;
    }

    if (debounce_rounds > debounce_threshhold) {
        debounce_rounds = 0;
        button_state = inst_button_state;
        return button_state ? JUST_PRESSED : JUST_RELEASED;
    }

    return button_state ? HELD : NOT_PRESSED;
}

void dial()
{
    String params[][2] = {{"NewX_AVM-DE_PhoneNumber", to_call}};
    String req[][2] = {{}};
    connection->action("X_VoIP:1", "X_AVM-DE_DialNumber", params, 1, req, 0);
}

void loop()
{
    static CHSV color(0, 255, 0);

    button_state_t btn = check_button();
    if (btn == JUST_PRESSED) {
        color.value = 255;
        color.hue += 10;
    }

    FastLED.showColor(color);

    if (btn == JUST_PRESSED) {
        dial();
    }

    if (btn == NOT_PRESSED && color.value > 0) {
        color.value -= 1;
    }

    ArduinoOTA.handle();
}
