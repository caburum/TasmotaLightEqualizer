#include "Network.hpp"
#include "Power.hpp"
#include "UserInput.hpp"

#define PIN_PHOTORESISTOR A0
#define PHOTORESISTOR_READS 3
#define SLEEP_TIMEOUT .5 * 60 * 1000

void setup() {
	UserInput::setup();

	Serial.begin(115200);
	delay(1000); // wait for serial monitor
}

int lightValue = 0;
int lastLightValue = 0;

inline int readLightValue() {
	int sum = 0;
	for (int i = 0; i < PHOTORESISTOR_READS; i++) {
		sum += analogRead(PIN_PHOTORESISTOR);
	}
	return sum / PHOTORESISTOR_READS;
}

void loop() {
	Serial.println("running");
	// todo: show "thinking" led to let user know system is alive (given long wifi connection time)

	// handle toggle button
	UserInput::loop();

	// todo: fetch on state from tasmota, and only change lights if on (so we don't turn on lights if they were off)

	// todo: what is current draw of photoresistor?

	// try not to get stuck forever
	for (int i = 0; i < 15; i++) {
		lightValue = readLightValue();
		Serial.print("light value: ");
		Serial.println(lightValue);

		int diff = UserInput::targetLightValue - lightValue;
		if (diff > PHOTORESISTOR_ACCEPTABLE_ERROR) {
			// increase brightness
			Network::sendCmnd("dimmer2+%2B10");
		} else if (diff < -PHOTORESISTOR_ACCEPTABLE_ERROR) {
			// if (abs(lightValue - lastLightValue) < 50) {
			// 	// no change, just turn off
			// 	sendCmnd("power2+off");
			// } else {
			// decrease brightness
			Network::sendCmnd("dimmer2+-10");
			// }
		} else { // at target
			Serial.println("at target");
			break;
		}
	}

	lastLightValue = lightValue;

	Serial.println("going to sleep");
	Power::lightSleep(SLEEP_TIMEOUT);
	// even if we wake up from sleep due to interrupt, code will still be delayed
	// todo: also physically wire power on/off switch to esp power
}
