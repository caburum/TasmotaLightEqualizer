#ifndef NETWORK_H
#define NETWORK_H

#include <ESP8266HTTPClient.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>

#include "secrets.h"

namespace Network {
	/** connect to wifi if not already connected */
	void connectWifi() {
		if (WiFi.status() != WL_CONNECTED) {
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
	}

	void sendCmnd(const char* cmnd) {
		connectWifi();
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
}

#endif