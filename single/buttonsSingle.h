/*
 *  Arduino ButtonSingle Library
 *  An interrupt-driven, fully-debounced class to manage input from physical buttons on the Arduino platform.
 *
 *  Copyright (C) 2017 Vital Holmo Batista
 *  Based on the work of Nicholas Parks Young: https://github.com/Alarm-Siren/arduino-buttons
 *  I have modified it just to save memory and avoid the use of dynamic allocation of RAM.
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301
 *  USA
 */

#pragma once

#include <Arduino.h>

class ButtonSingle
{
public:

	void begin(const uint8_t buttonPin);

	void stop();

	bool clicked() __attribute__((always_inline))
	{
		bool bClicked = state && CLICKED_FLAG;
		state &= ~CLICKED_FLAG;
		return bClicked;
	}

	bool released() __attribute__((always_inline))
	{
		bool bReleased = state && RELEASED_FLAG;
		state &= ~RELEASED_FLAG;
		return bReleased;
	}

	bool longClicked() __attribute__((always_inline))
	{
		bool blongClicked = state && LONG_CLICKED_FLAG;
		state &= ~LONG_CLICKED_FLAG;
		return blongClicked;
	}

	bool doubleClicked() __attribute__((always_inline))
	{
		bool bDoubleClicked = state && DOUBLE_CLICKED_FLAG;
		state &= ~DOUBLE_CLICKED_FLAG;
		return bDoubleClicked;
	}

	bool down() __attribute__((always_inline))
	{
		return state && PRESSED_FLAG;
	}

	bool up(uint8_t buttonId) __attribute__((always_inline))
	{
		return !down();
	}

	bool polledDown(uint8_t buttonId) __attribute__((always_inline))
	{
		return digitalRead(pin) == LOW;
	}

private:

	/**
	* Debounce period in milliseconds.
	*/
	static constexpr unsigned long DEBOUNCE_DELAY = 50;
	static constexpr unsigned long DOUBLE_CLICK_DELAY = 500;
	static constexpr unsigned long LONG_CLICK_DELAY = 2000;

	static constexpr uint8_t CLEAR_FLAGS = 0;
	static constexpr uint8_t PRESSED_FLAG = _BV(0);
	static constexpr uint8_t CLICKED_FLAG = _BV(1);
	static constexpr uint8_t RELEASED_FLAG = _BV(2);
	static constexpr uint8_t LONG_CLICKED_FLAG = _BV(3);
	static constexpr uint8_t DOUBLE_CLICKED_FLAG = _BV(4);

	uint8_t pin;
	uint8_t state;
	unsigned long lastChangeTime, lastClickTime;

	/**
	* This function is called whenever a button interrupt is fired.
	* It reads all the button states and updates their _buttons objects
	* accordingly.
	*/
	void button_Handler();

	static void buttonISR();
};
