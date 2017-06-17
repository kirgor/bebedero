#include <avr/io.h>
#include <util/delay.h>
#include <light-avr/input.h>
#include <light-avr/output.h>
#include <light-avr/delay.h>

// Delay between main loop iterations
#define MAIN_LOOP_DELAY 50
// Delay for button push debounce (to filter messy signal after physical push)
#define BUTTON_PUSH_DEBOUNCE_DELAY 50
// Delay for button relay debounce (to filter messy signal after relay is activated)
#define BUTTON_RELAY_DEBOUNCE_DELAY 500
// After device is turned on, it will turn off automatically after specified time
#define AUTO_TURN_OFF_TIMEOUT 180000
// After device is turned off, sensor will not activate for specified time
#define SENSOR_TURN_ON_TIMEOUT 60000
// Delay and count passed time
#define delayMsCounted(ms) delayMs(ms); time += ms;

// Elapsed time
long time = 0;
// Last turn on timestamp
long turnOnTime = 0;
// Last turn off timestamp
long turnOffTime = 0;
// Current turned on status
char on = 0;
// Previous button up status
char prevButtonUp = 1;

/**
 * Turns the device on
 */
void turnOn() {
	turnOnTime = time;
	writeOutputB(0, 1);
	on = 1;
}

/**
 * Turns the device off
 */
void turnOff() {
	turnOffTime = time;
	writeOutputB(0, 0);
	on = 0;
}

char processButton() {
	char buttonActive = 0;
	char buttonUp = readInputB(1);
	if (prevButtonUp && !buttonUp) {
		delayMsCounted(BUTTON_PUSH_DEBOUNCE_DELAY);
		buttonUp = readInputB(1);
		if (!buttonUp) {
			buttonActive = 1;
			if (!on)
				turnOn();
			else
				turnOff();
			delayMsCounted(BUTTON_RELAY_DEBOUNCE_DELAY);
		}
	}
	prevButtonUp = buttonUp;

	return buttonActive;
}

char processSensor() {
	char sensorActive = readInputB(2);
	if (sensorActive && !on	&& (turnOffTime == 0 || time - turnOffTime > SENSOR_TURN_ON_TIMEOUT)) {
		turnOn();
		return 1;
	}
	return 0;
}

void processTimeout() {
	if (on && time - turnOnTime > AUTO_TURN_OFF_TIMEOUT) {
		turnOff();
	}
}

int main() {
	initOutputB(0);

	// Main loop
	while (1) {
		if (!processButton()) {
			if (!processSensor()) {
				processTimeout();
			}
		}

		delayMsCounted(MAIN_LOOP_DELAY);
	}
}
