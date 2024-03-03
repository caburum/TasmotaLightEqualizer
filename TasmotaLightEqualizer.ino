#include "Network.hpp"
#include "Power.hpp"
#include "UserInput.hpp"

#define PIN_PHOTORESISTOR A0

void setup() {
	Serial.begin(115200);
	delay(1000); // wait for serial monitor

	Storage::setup();
	UserInput::setup();
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
#define TASMOTA_STEP "2" // todo: implement bigger step with larger error
void loop() {
	Serial.println("running");
	// todo: show "thinking" led (blue?) to let user know system is alive (given long wifi connection time)

	// handle toggle button
	UserInput::loop(); // todo: flash red/green on press to indicate new state (on/off)

	// todo: what is current draw of photoresistor? might want to have a pin for turning it on

	if (Network::isLightOn()) {
		int lastLightValue = -1; // initialize to impossible value as no previous reading

		// try not to get stuck forever
		for (int i = 0; i < 55; i++) {
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
				// will generally end up looping here if user interacted recently, so don't go to sleep (for faster alterations)
			} else if (error > PHOTORESISTOR_ACCEPTABLE_ERROR) {
				// increase brightness
				Network::sendCmnd(("dimmer2+%2B" + String(step)).c_str());
			} else if (error < -PHOTORESISTOR_ACCEPTABLE_ERROR) {
				// decrease brightness
				Network::sendCmnd(("dimmer2+-" + String(step)).c_str());
				// todo: can't turn off at brightness 1 since 0 becomes off & locks us out (for now), track set state & autodisable if tasmota power changed to off while we think we *should* be on
			} else { // at target
				Serial.println("at target");
				// break;
			}

			lastLightValue = lightValue;

			// crashes... why?
			// unsigned long startMillis = millis();
			// while (millis() - startMillis < 500) yield();
		}
	} else {
		Serial.println("light is off");
	}

	Storage::loop();

	Serial.println("going to sleep");
	Power::lightSleep(60e3);
	// todo: also physically wire power on/off switch to esp power
}
