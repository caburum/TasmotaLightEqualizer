#ifndef STORAGE_H
#define STORAGE_H

#define OFFSET_TARGET_LIGHT_VALUE 0

namespace StorageData {
	volatile uint32_t targetLightValue;
}

namespace Storage {
	using namespace StorageData;

	uint32_t lastTargetLightValue;

	// initialize values from rtc
	inline void setup() {
		ESP.rtcUserMemoryRead(OFFSET_TARGET_LIGHT_VALUE, &lastTargetLightValue, sizeof(targetLightValue));
		targetLightValue = lastTargetLightValue; // since value is volatile, use last as proxy
	}

	// push values to rtc
	inline void loop() {
		if (lastTargetLightValue != targetLightValue) {
			lastTargetLightValue = targetLightValue; // since value is volatile, use last as proxy
			ESP.rtcUserMemoryWrite(OFFSET_TARGET_LIGHT_VALUE, &lastTargetLightValue, sizeof(targetLightValue));
		}
	}
}

#endif