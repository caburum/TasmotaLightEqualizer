#ifndef STORAGE_H
#define STORAGE_H

#define OFFSET_BOOTCOUNT 0
#define OFFSET_TARGET_LIGHT_VALUE 1

namespace StorageData {
	volatile int32_t targetLightValue; // even though this should be uint32, int32 makes dealing with clamping easier as numbers don't wrap as badly
	uint32_t bootCount;
}

namespace Storage {
	using namespace StorageData;

	int32_t lastTargetLightValue;

	// initialize values from rtc
	inline void setup() {
		if (!ESP.rtcUserMemoryRead(OFFSET_BOOTCOUNT, &bootCount, sizeof(bootCount))) {
			Serial.println(F("rtc read failed"));
			bootCount = 0;
		}
		bootCount++;
		ESP.rtcUserMemoryWrite(OFFSET_BOOTCOUNT, &bootCount, sizeof(bootCount));
		Serial.print(F("boot count: "));
		Serial.println(bootCount);

		ESP.rtcUserMemoryRead(OFFSET_TARGET_LIGHT_VALUE, (uint32_t*)(&lastTargetLightValue), sizeof(targetLightValue));
		targetLightValue = lastTargetLightValue; // since value is volatile, use last as proxy
	}

	// push values to rtc
	inline void loop() {
		if (lastTargetLightValue != targetLightValue) {
			lastTargetLightValue = targetLightValue; // since value is volatile, use last as proxy
			// just dump the raw bytes of the value
			ESP.rtcUserMemoryWrite(OFFSET_TARGET_LIGHT_VALUE, (uint32_t*)(&lastTargetLightValue), sizeof(targetLightValue));
		}
	}
}

#endif