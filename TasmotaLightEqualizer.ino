#include "Network.hpp"
#include "Power.hpp"
#include "StatusLight.hpp"
#include "UserInput.hpp"

#define PIN_PHOTORESISTOR A0

void setup() {
	Serial.begin(115200);
	delay(1000); // wait for serial monitor

	Storage::setup();
	UserInput::setup();
	StatusLight::setup();
}

int lightValue = 0;

#define PHOTORESISTOR_READS 3
inline int readLightValue() {
	int sum = 0;
	for (int i = 0; i < PHOTORESISTOR_READS; i++) {
		sum += analogRead(PIN_PHOTORESISTOR);
	}
	return sum / PHOTORESISTOR_READS;
}

#define PHOTORESISTOR_ACCEPTABLE_ERROR 10
void loop() {
	Serial.println("running");

	// handle toggle button
	UserInput::loop();

	// todo: what is current draw of photoresistor? might want to have a pin for turning it on

	networkBooleanResult_t networkStatus = Network::power2();
	if (networkStatus == NETWORK_ON) {
		StatusLight::setColor(false, false, true); // blue

		int lastLightValue = -1; // initialize to impossible value as no previous reading

		bool reachedTargetOnce = false;
		// try not to get stuck forever
		for (int i = 0; i < 55; i++) {
			ESP.wdtFeed();

			lightValue = readLightValue();
			Serial.print("light value: ");
			Serial.print(lightValue);

			int error = StorageData::targetLightValue - lightValue;

			int step = constrain(.13 * abs(error), 1, 30); // for every 1 light value, dimmer increases by approx .147; underestimate to be safe & prevent major oscillations

			Serial.print(" step: ");
			Serial.println(step);

			if (lastLightValue >= 0 && StorageData::targetLightValue == Storage::lastTargetLightValue && abs(lastLightValue - lightValue) < 6) {
				Storage::loop(); // push last target value to rtc if changed
				Serial.println("no change");
				StatusLight::setColor(true, true, false); // yellow
				// will generally end up looping here if user interacted recently, so don't go to sleep (for faster alterations)
			} else if (error > PHOTORESISTOR_ACCEPTABLE_ERROR) {
				if (reachedTargetOnce) StatusLight::setColor(false, false, false); // off
				// increase brightness
				Network::sendCmnd(("dimmer2+%2B" + String(step)).c_str());
			} else if (error < -PHOTORESISTOR_ACCEPTABLE_ERROR) {
				if (reachedTargetOnce) StatusLight::setColor(false, false, false); // off
				// decrease brightness
				Network::sendCmnd(("dimmer2+-" + String(step)).c_str());
				// todo: can't turn off at brightness 1 since 0 becomes off & locks us out (for now), track set state & autodisable if tasmota power changed to off while we think we *should* be on
			} else { // at target
				reachedTargetOnce = true;
				Serial.println("at target");
				StatusLight::setColor(false, true, false); // green
				// break;
			}

			lastLightValue = lightValue;
		}
	} else if (networkStatus == NETWORK_OFF) {
		Serial.println("light is off");
	} else {
		Serial.println("network error");
		StatusLight::setColor(true, false, false); // red
	}

	Storage::loop();

	// todo: test for crash?
	unsigned long startMillis = millis();
	while (millis() - startMillis < 100) {
		ESP.wdtFeed();
		optimistic_yield(10e3);
	}

	StatusLight::setColor(false, false, false); // off

	Serial.println("going to sleep");
	Power::lightSleep(60e3);
}
