#include "Network.hpp"
#include "Power.hpp"
#include "UserInput.hpp"

#define PIN_PHOTORESISTOR A0

void setup() {
	Storage::setup();
	UserInput::setup();

	Serial.begin(115200);
	delay(1000); // wait for serial monitor
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

#define PHOTORESISTOR_ACCEPTABLE_ERROR 15
#define TASMOTA_STEP "2" // todo: implement bigger step with larger error
void loop() {
	Serial.println("running");
	// todo: show "thinking" led to let user know system is alive (given long wifi connection time)

	// handle toggle button
	UserInput::loop();

	// todo: what is current draw of photoresistor?

	if (Network::isLightOn()) {
		int lastLightValue = -1; // initialize to impossible value as no previous reading

		// try not to get stuck forever
		for (int i = 0; i < 55; i++) {
			lightValue = readLightValue();
			Serial.print("light value: ");
			Serial.println(lightValue);

			int error = StorageData::targetLightValue - lightValue;
			if (lastLightValue >= 0 && StorageData::targetLightValue == Storage::lastTargetLightValue && abs(lastLightValue - lightValue) < 6) {
				Storage::loop(); // push last target value to rtc if changed
				Serial.println("no change");
			} else if (error > PHOTORESISTOR_ACCEPTABLE_ERROR) {
				// increase brightness
				Network::sendCmnd("dimmer2+%2B" TASMOTA_STEP);
			} else if (error < -PHOTORESISTOR_ACCEPTABLE_ERROR) {
				// decrease brightness
				Network::sendCmnd("dimmer2+-" TASMOTA_STEP);
				// todo: can't turn off at brightness 1 since 0 becomes off & locks us out (for now)
			} else { // at target
				Serial.println("at target");
				break;
			}

			lastLightValue = lightValue;

			delay(200);
		}
	} else {
		Serial.println("light is off");
	}

	// todo: if user interacted recently, don't go to sleep (for faster alterarions)

	Storage::loop();

	Serial.println("going to sleep");
	Power::lightSleep(60e3);
	// todo: also physically wire power on/off switch to esp power
}
