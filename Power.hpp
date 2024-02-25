#ifndef POWER_HPP
#define POWER_HPP

#include <ESP8266WiFi.h>
#include <user_interface.h>

#include "config.h"

extern os_timer_t* timer_list;

// https://kevinstadler.github.io/notes/esp8266-deep-sleep-light-sleep-arduino/
// https://forum.arduino.cc/t/963225
// https://github.com/alenaksu/AirQualityMonitor/blob/main/src/PowerManager.cpp
namespace Power {
	os_timer_t* lastTimerList;

	void fpm_wakup_cb_func(void) {
		Serial.println("woke up from light sleep");
		Serial.flush();

		timer_list = lastTimerList;
		lastTimerList = nullptr;

		wifi_fpm_close(); // disable force sleep

		// wifi_disable_gpio_wakeup();
		// gpio_pin_wakeup_disable();

		// wifi_set_opmode(STATION_MODE);
		// wifi_station_connect();
	}

	void lightSleep(long sleepTime_ms) {
		Serial.flush();

		// for timer-based light sleep to work, the os timers need to be disconnected
		lastTimerList = timer_list;
		timer_list = nullptr;

		wifi_set_opmode(NULL_MODE);
		wifi_fpm_set_sleep_type(LIGHT_SLEEP_T);
		wifi_fpm_set_wakeup_cb(fpm_wakup_cb_func);
		wifi_fpm_open();

		// optional: register one or more wake-up interrupts. the chip
		// will wake from whichever (timeout or interrupt) occurs earlier
		gpio_pin_wakeup_enable(PIN_SW, GPIO_PIN_INTR_LOLEVEL);
		gpio_pin_wakeup_enable(PIN_CLK, GPIO_PIN_INTR_LOLEVEL);
		gpio_pin_wakeup_enable(PIN_DT, GPIO_PIN_INTR_LOLEVEL);

		// light sleep function requires microseconds
		wifi_fpm_do_sleep(sleepTime_ms * 1000);

		// timed light sleep is only entered when the sleep command is
		// followed by a delay() that is at least 1ms longer than the sleep
		delay(sleepTime_ms + 1);
	}

	// light sleep forever until hardware interrupt
	void lightSleep() {
		lightSleep(0xFFFFFFF);
	}
}

#endif