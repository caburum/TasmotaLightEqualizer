#ifndef STORAGE_H
#define STORAGE_H

#define OFFSET_TARGET_LIGHT_VALUE 0

namespace StorageData {
	volatile int32_t targetLightValue; // even though this should be uint32, int32 makes dealing with clamping easier as numbers don't wrap as badly
}

namespace Storage {
	using namespace StorageData;

	int32_t lastTargetLightValue;

	// initialize values from rtc
	inline void setup() {
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