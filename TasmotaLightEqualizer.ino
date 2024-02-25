#include <ESP8266HTTPClient.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>

#include "Power.hpp"
#include "config.h"
#include "secrets.h"

void setup() {
	pinMode(PIN_SW, INPUT_PULLUP);
	pinMode(PIN_CLK, INPUT);
	pinMode(PIN_DT, INPUT);

	attachInterrupt(digitalPinToInterrupt(PIN_SW), toggleButtonInterrupt, RISING);
	attachInterrupt(digitalPinToInterrupt(PIN_CLK), updateEncoderInterrupt, CHANGE);
	attachInterrupt(digitalPinToInterrupt(PIN_DT), updateEncoderInterrupt, CHANGE);

	Serial.begin(115200);
	delay(1000); // wait for serial monitor
}

void loop() {
	Serial.println("running");

	connectWifi(); // reconnect after sleep
	sendCmnd("power2+toggle");

	Serial.println("going to sleep");
	Power::lightSleep(SLEEP_TIMEOUT);
	// even if we wake up from sleep due to interrupt, code will still be delayed
	// todo: also physically wire power on/off switch to esp power
}

void connectWifi() {
	WiFi.mode(WIFI_STA);
	WiFi.begin(WIFI_SSID, WIFI_PASSPHRASE);
	Serial.print("wifi connecting");
	while (WiFi.status() != WL_CONNECTED) {
		delay(500);
		Serial.print(".");
	}
	Serial.println();
	Serial.print("connected to \"");
	Serial.print(WIFI_SSID);
	Serial.print("\" with ip ");
	Serial.println(WiFi.localIP());
}

void sendCmnd(const char* cmnd) {
	if (WiFi.status() == WL_CONNECTED) {
		WiFiClient client;
		HTTPClient http;

		String serverPath = URL_CMND + cmnd;

		http.begin(client, serverPath.c_str());

		int httpResponseCode = http.GET();

		if (httpResponseCode > 0) {
			Serial.print("http response code: ");
			Serial.println(httpResponseCode);
			String payload = http.getString();
			Serial.println(payload);
		} else {
			Serial.print("error code: ");
			Serial.println(httpResponseCode);
		}

		http.end();
	} else {
		Serial.println("wifi disconnected");
	}
}

ICACHE_RAM_ATTR void toggleButtonInterrupt() {
	// https://forum.arduino.cc/t/45110
	static unsigned long lastTime = 0;
	unsigned long currentTime = millis();
	// if interrupts come faster than 200ms, assume it's a bounce and ignore
	if (currentTime - lastTime > 200) {
		// sendCmnd("power2+toggle");
		Serial.println("toggle");
	}
	lastTime = currentTime;
}

volatile int encoderCount = 0;

ICACHE_RAM_ATTR void updateEncoderInterrupt() {
	static int lastStateCLK = HIGH; // pullup
	static int currentStateCLK;

	currentStateCLK = digitalRead(PIN_CLK);

	if (currentStateCLK != lastStateCLK && currentStateCLK == 1) {
		// pulse occurred, react to only 1 state change to avoid double count

		if (digitalRead(PIN_DT) != currentStateCLK) {
			// encoder is rotating CCW
			encoderCount--;
		} else {
			// encoder is rotating CW
			encoderCount++;
		}

		Serial.println(encoderCount);
	}

	lastStateCLK = currentStateCLK;
}