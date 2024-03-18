#ifndef STATUSLIGHT_H
#define STATUSLIGHT_H

#define PIN_LED_R D1
#define PIN_LED_G D2
#define PIN_LED_B D8

namespace StatusLight {
	inline void setup() {
		pinMode(PIN_LED_R, OUTPUT);
		pinMode(PIN_LED_G, OUTPUT);
		pinMode(PIN_LED_B, OUTPUT);
	};

	void setColor(bool r, bool g, bool b) {
		digitalWrite(PIN_LED_R, r);
		digitalWrite(PIN_LED_G, g);
		digitalWrite(PIN_LED_B, b);
	}
}

#endif