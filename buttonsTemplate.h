/*
 *  Arduino Buttons Template Library
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

/**
* Default periods in milliseconds.
* Can be overridden in user files by #defining them before including this file.
*/
#ifndef BUTTON_DEBOUNCE_DELAY
#define BUTTON_DEBOUNCE_DELAY 30
#endif
#ifndef DOUBLE_CLICK_DELAY
#define DOUBLE_CLICK_DELAY 500
#endif
#ifndef LONG_RELEASE_DELAY
#define LONG_RELEASE_DELAY 1000
#endif

/**
* This structure encompasses information relating to an individual button.
*/
struct Button
{
	/**
	* Stores pin number of the button.
	*/
	uint8_t pin;

	/**
	* Stores the most recently measured state of the button.
	*/
	uint8_t state;

	/**
	* This records the last time that an Interrupt was triggered from this pin.
	* Used as part of the debounce routine.
	*/
	unsigned long lastChangeTime, lastClickTime;

	/**
	* Constructor for objects of Button.
	*/
	Button() :
		pin(0),
		state(0),
		//		currentState(false),
		//		changeFlag(false),
		//		longClickFlag(false),
		lastChangeTime(0),
		lastClickTime(0)
	{
	}
};

/**
 * This static-only template class implements a system for getting user input from buttons.
 * It internally applies debounce periods and tracks whether a button press or release
 * has been "processed" by use of a Change Flag for each button.
 *
 * This is all interrupt driven, so there is no penalty to running code except when the
 * user actually presses a button. This also means, implicitly, you must ensure that
 * any pins used for button inputs are capable of having interrupts attached to them.
 * On the Arduino Due (for example) all digital pins can be used in this way, but on
 * the Arduino Uno, only pins 2 and 3 can have interrupts attached.
 */
template <const uint8_t NumberOfButtons>
class Buttons final
{
public:

	/**
	 * Initialize the buttons as attached to the specified pins and attach appropriate interrupts.
	 * The index of each button in the buttonPins parameter array is preserved for the buttonId parameter
	 * on accessor methods such as clicked, down etc. Hence, if you want to read the status of
	 * the button attached to the pin specified in buttonPins[3], you could call clicked(3, true).
	 *
	 * @param buttonPins        pointer to an array of uint8_t, each being the number of a
	 *                          pin with a button attached that is to be managed by this object. The number of items in
	 *									 the array must be the same as the "NumberOfButtons" used on the template instantiation.
	 * @return                  true on success, false on failure.
	 */
	static bool begin(const uint8_t buttonPins[]);

	/**
	 * Detach interrupts from the pins controlled by this object.
	 * If the object has not been started with begin(), or begin() failed, calling this will do nothing.
	 */
	static void stop();

	/**
	 * Returns a bool value indicating if the user has "clicked" the button,
	 * defined as the button being down and the Change Flag set.
	 *
	 * @param buttonId          Index of the button whose status is to be checked.
	 * @return                  true if the button has been clicked, false otherwise.
	 */
	static bool clicked(uint8_t buttonId) __attribute__((always_inline))
	{
		uint8_t bClicked = _buttons[buttonId].state & CLICKED_FLAG;
		_buttons[buttonId].state &= ~CLICKED_FLAG;
		return bClicked != 0;
	}

	/**
	* Returns a bool value indicating if the user has "released" the button,
	* defined as the button being up and the Change Flag set.
	*
	* @param buttonId          Index of the button whose status is to be checked.
	* @param clearChangeFlag   If true, the Change Flag for this button will be cleared at the same time.
	* @return                  true if the button has been clicked since the Change Flag
	*                          was last cleared, false otherwise.
	*/
	static bool shortReleased(uint8_t buttonId) __attribute__((always_inline))
	{
		uint8_t bShortReleased = _buttons[buttonId].state & SHORT_RELEASED_FLAG;
		_buttons[buttonId].state &= ~SHORT_RELEASED_FLAG;
		return bShortReleased != 0;
	}

	/**
	* Returns a bool value indicating if the user has "released" the button,
	* after being pressed for at least the period Timeout of milliseconds.
	* Defined as the button being up and the Change Flag set.
	*
	* @param buttonId          Index of the button whose status is to be checked.
	* @param clearChangeFlag   If true, the Change Flag for this button will be cleared at the same time.
	* @return                  true if the button has been clicked since the Change Flag
	*                          was last cleared, false otherwise.
	*/
	static bool longReleased(uint8_t buttonId) __attribute__((always_inline))
	{
		uint8_t blongReleased = _buttons[buttonId].state & LONG_RELEASED_FLAG;
		_buttons[buttonId].state &= ~LONG_RELEASED_FLAG;
		return blongReleased != 0;
	}

	static bool doubleClicked(uint8_t buttonId) __attribute__((always_inline))
	{
		uint8_t bDoubleClicked = _buttons[buttonId].state & DOUBLE_CLICKED_FLAG;
		_buttons[buttonId].state &= ~DOUBLE_CLICKED_FLAG;
		return bDoubleClicked != 0;
	}

	/**
	 * Returns a bool value indicating if the button is currently "down"/"pressed".
	 * This return value is independent of the state of the Change Flag, however you can
	 * clear the Change Flag if desired.
	 * It is the opposite of the up() method.
	 *
	 * @param buttonId          Index of the button whose status is to be checked.
	 * @param clearChangeFlag   If true, the Change Flag for this button will be cleared at the same time.
	 * @return                  true if the button is down.
	 */
	static bool down(uint8_t buttonId) __attribute__((always_inline))
	{
		return (_buttons[buttonId].state & PRESSED_FLAG) != 0;
	}

	/**
	 * Returns a bool value indicating if the button is currently "up"/"not pressed".
	 * This return value is independent of the state of the Change Flag, however you can
	 * clear the Change Flag if desired.
	 * It is the opposite of the down() method.
	 *
	 * @param buttonId          Index of the button whose status is to be checked.
	 * @param clearChangeFlag   If true, the Change Flag for this button will be cleared at the same time.
	 * @return                  true if the button is up.
	 */
	static bool up(uint8_t buttonId) __attribute__((always_inline))
	{
		return !down(buttonId);
	}

		/**
		 * Returns the number of buttons currently controlled by this class.
		 *
		 * @return    The number of buttons controlled by this class
		 */
	static uint8_t numberOfButtons() __attribute__((always_inline))
	{
		return NumberOfButtons;
	}

	static bool polledDown(uint8_t buttonId) __attribute__((always_inline))
	{
		return digitalRead(_buttons[buttonId].pin) == LOW;
	}

	//This class has only static members, therefore constructors etc are pointless.
	Buttons() = delete;
	~Buttons() = delete;
	Buttons& operator=(const Buttons&) = delete;
	Buttons(const Buttons&) = delete;

private:
	static constexpr uint8_t CLEAR_FLAGS = 0;
	static constexpr uint8_t PRESSED_FLAG = _BV(0);
	static constexpr uint8_t CLICKED_FLAG = _BV(1);
	static constexpr uint8_t SHORT_RELEASED_FLAG = _BV(2);
	static constexpr uint8_t LONG_RELEASED_FLAG = _BV(3);
	static constexpr uint8_t DOUBLE_CLICKED_FLAG = _BV(4);

	/**
	* This function is called whenever a button interrupt is fired.
	* It reads all the button states and updates their _buttons objects
	* accordingly.
	*/
	static void button_ISR();

	/**
	* This array stores Button objects for each button controlled by this class,
	* each containing relevant information for the servicing of the ISR.
	* Its volatile because its members may be modified by an ISR, so we need to
	* prevent register caching of member values.
	*/
	//	static volatile Button _buttons[NumberOfButtons];
	static volatile Button _buttons[NumberOfButtons];

	/**
	* Set to true if this class has been initialised, false otherwise.
	*/
	static bool _begun;
};

template <uint8_t NumberOfButtons>
bool Buttons<NumberOfButtons>::_begun = false;

template <uint8_t NumberOfButtons>
volatile Button Buttons<NumberOfButtons>::_buttons[NumberOfButtons];

template <uint8_t NumberOfButtons>
bool Buttons<NumberOfButtons>::begin(const uint8_t buttonPins[])
{
	// Abort if the buttonPins array is null
	if (nullptr == buttonPins) return false;

	// If Buttons has already been started, kill it before restarting it.
	if (_begun) stop();

	// Set up the input pins themselves.
	for (uint8_t i = 0; i < NumberOfButtons; i++)
	{
		_buttons[i].pin = buttonPins[i];
		pinMode(buttonPins[i], INPUT_PULLUP);
	}

	// Need to wait some time before setting up the ISRs, otherwise you can get spurious
	// changes as the pullup hasn't quite done its magic yet.
	delay(10);

	//Set up the interrupts on the pins.
	for (uint8_t i = 0; i < NumberOfButtons; i++)
	{
		attachInterrupt(digitalPinToInterrupt(buttonPins[i]), &Buttons<NumberOfButtons>::button_ISR, CHANGE);
	}

	// initialize buttons state
	for (uint8_t i = 0; i < NumberOfButtons; i++)
	{
		_buttons[i].state = digitalRead(_buttons[i].pin) ? CLEAR_FLAGS : PRESSED_FLAG;
		_buttons[i].lastClickTime = _buttons[i].lastChangeTime = millis();
	}

	// All done.
	_begun = true;
	return true;
}

template <uint8_t NumberOfButtons>
void Buttons<NumberOfButtons>::stop()
{
	// If the object is already stopped, we don't need to do anything.
	if (!_begun)
		return;

	//Disable the interrupts
	for (uint8_t i = 0; i < NumberOfButtons; i++)
	{
		detachInterrupt(digitalPinToInterrupt(_buttons[i].pin));
	}

	//Object has been stopped.
	_begun = false;
}

template <uint8_t NumberOfButtons>
void Buttons<NumberOfButtons>::button_ISR()
{
	uint32_t now = millis();
	for (uint8_t i = 0; i < NumberOfButtons; i++)
	{
		bool readState = polledDown(i);
		bool buttonState = down(i);
		if (readState != buttonState)
		{
			if (now - _buttons[i].lastChangeTime > BUTTON_DEBOUNCE_DELAY)
			{
				if (readState) // button has been clicked
				{
					if (now - _buttons[i].lastClickTime > DOUBLE_CLICK_DELAY)
					{
						_buttons[i].state = PRESSED_FLAG | CLICKED_FLAG;
					}
					else
					{
						_buttons[i].state = PRESSED_FLAG | DOUBLE_CLICKED_FLAG;
					}
					_buttons[i].lastClickTime = now;
				}
				else
				{ // button has been released
					_buttons[i].state &= ~PRESSED_FLAG;
					if (now - _buttons[i].lastClickTime > LONG_RELEASE_DELAY)
						_buttons[i].state |= LONG_RELEASED_FLAG;
					else
						_buttons[i].state |= SHORT_RELEASED_FLAG;

				}
			}
			_buttons[i].lastChangeTime = now;
		}
	}
}
