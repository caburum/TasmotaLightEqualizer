#ifndef NETWORK_H
#define NETWORK_H

#include <Arduino_JSON.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>

#include "secrets.h"

namespace Network {
	WiFiClient client;

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

	String sendCmnd(const char* cmnd) {
		connectWifi();
		String response;
		if (WiFi.status() == WL_CONNECTED) {
			String serverPath = URL_CMND + cmnd;

			HTTPClient http; // ?attempting to reuse causes a crash
			http.begin(client, serverPath.c_str());

			int httpResponseCode = http.GET();

			if (httpResponseCode > 0) {
				Serial.print("http response code: ");
				Serial.println(httpResponseCode);
				response = http.getString();
				Serial.println(response);
			} else {
				Serial.print("error code: ");
				Serial.println(httpResponseCode);
			}

			http.end();
		} else {
			Serial.println("wifi disconnected");
		}
		return response;
	}

	bool isLightOn() {
		String response = sendCmnd("POWER2");
		JSONVar json = JSON.parse(response);
		if (JSON.typeof(json) == "undefined") {
			Serial.println("json parse failed");
			return false;
		}
		if (json.hasOwnProperty("POWER2")) {
			return json.hasPropertyEqual("POWER2", "ON");
		}
		return false;
	}
}

#endif