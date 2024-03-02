#ifndef USERINPUT_HPP
#define USERINPUT_HPP

#include "Network.hpp"
#include "Storage.hpp"
#include "config.h"

#define PIN_SW D4
#define PIN_CLK D5
#define PIN_DT D6

#define ENCODER_TARGET_STEP_SLOW 10
#define ENCODER_TARGET_STEP_MED 20
#define ENCODER_TARGET_STEP_FAST 80

namespace UserInput {
	volatile bool toggleScheduledFlag = false;

	IRAM_ATTR void toggleButtonInterrupt() {
		// https://forum.arduino.cc/t/45110
		static unsigned long lastTime = 0;
		unsigned long currentTime = millis();
		// if interrupts come faster than 200ms, assume it's a bounce and ignore
		if (currentTime - lastTime > 200) {
			toggleScheduledFlag = true; // will be unset by main loop
			Serial.println("toggleScheduledFlag: true");
		}
		lastTime = currentTime;
	}

	inline void loop() {
		if (toggleScheduledFlag) {
			toggleScheduledFlag = false;
			Network::sendCmnd("power2+toggle");
		}
	}

	// https://garrysblog.com/2021/03/20/reliably-debouncing-rotary-encoders-with-arduino-and-esp32/
	IRAM_ATTR void updateEncoderInterrupt() {
		using namespace StorageData;

		static uint8_t oldAB = 3; // lookup table index
		static int8_t encval = 0; // encoder value
		static const int8_t encStates[] = {0, -1, 1, 0, 1, 0, 0, -1, -1, 0, 0, 1, 0, 1, -1, 0}; // lookup table
		static unsigned long lastTime = 0;
		unsigned long currentTime = millis();

		oldAB <<= 2; // Remember previous state

		if (digitalRead(PIN_DT)) oldAB |= 0x02; // add current state of pin A
		if (digitalRead(PIN_CLK)) oldAB |= 0x01; // add current state of pin B

		encval += encStates[(oldAB & 0x0f)];

		// update counter if encoder has rotated a full indent, that is at least 4 steps
		if (encval > 3) { // four steps forward
			if (currentTime - lastTime > 40) {
				targetLightValue += ENCODER_TARGET_STEP_SLOW;
			} else if (currentTime - lastTime > 20) {
				targetLightValue += ENCODER_TARGET_STEP_MED;
			} else {
				targetLightValue += ENCODER_TARGET_STEP_FAST;
			}
			encval = 0;
			lastTime = currentTime;
		} else if (encval < -3) { // four steps backwards
			if (currentTime - lastTime > 40) {
				targetLightValue -= ENCODER_TARGET_STEP_SLOW;
			} else if (currentTime - lastTime > 20) {
				targetLightValue -= ENCODER_TARGET_STEP_MED;
			} else {
				targetLightValue -= ENCODER_TARGET_STEP_FAST;
			}
			encval = 0;
			lastTime = currentTime;
		} else {
			return; // skip dealing with value
		}

		targetLightValue = constrain(targetLightValue, 0, 1023);

		Serial.print("targetLightValue: ");
		Serial.println(targetLightValue);
	}

	inline void setup() {
		pinMode(PIN_SW, INPUT_PULLUP);
		pinMode(PIN_CLK, INPUT);
		pinMode(PIN_DT, INPUT);

		attachInterrupt(digitalPinToInterrupt(PIN_SW), toggleButtonInterrupt, RISING);
		attachInterrupt(digitalPinToInterrupt(PIN_CLK), updateEncoderInterrupt, CHANGE);
		attachInterrupt(digitalPinToInterrupt(PIN_DT), updateEncoderInterrupt, CHANGE);
	}
}

#endif