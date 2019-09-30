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

#ifndef BUTTONS_TEMPLATE_H
#define BUTTONS_TEMPLATE_H

#include <Arduino.h>

 /**
 * This structure encompasses information relating to an individual button.
 */
struct Button
{
	/**
	* Stores pin number of the button.
	*/
	uint8_t buttonPin;

	/**
	* Stores the most recently measured state of the button.
	* true = pushed, false = not pushed.
	*/
	bool currentState;

	/**
	* This flag indicates is set to true when currentState changes,
	* and (optionally) set false when that state is read.
	*/
	bool changeFlag;
	bool longClickFlag;

	/**
	* This records the last time that an Interrupt was triggered from this pin.
	* Used as part of the debounce routine.
	*/
	unsigned long lastChangeTime;

	/**
	* Constructor for objects of Button.
	*/
	Button() :
		buttonPin(false),
		currentState(false),
		changeFlag(false),
		longClickFlag(false),
		lastChangeTime(0)
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
template <uint8_t NumberOfButtons>
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
	 * @param clearChangeFlag   If true, the Change Flag for this button will be cleared at the same time.
	 * @return                  true if the button has been clicked since the Change Flag
	 *                          was last cleared, false otherwise.
	 */
	static inline bool clicked(uint8_t buttonId, bool clearChangeFlag)
	{
		return changed(buttonId, clearChangeFlag) && down(buttonId);
	}
	// Clears the change flag only if the button is clicked
	static bool clicked(uint8_t buttonId)
	{
		bool bChanged = changed(buttonId);
		if (bChanged && down(buttonId))
		{
			clearChangedFlag(buttonId);
		}
		return bChanged;
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
	static inline bool released(uint8_t buttonId, bool clearChangeFlag)
	{
		return changed(buttonId, clearChangeFlag) && !down(buttonId);
	}
	//Clears the change flag only if the button is released
	static bool released(uint8_t buttonId)
	{
		bool bChanged = changed(buttonId);
		if (changed(buttonId) && !down(buttonId))
		{
			clearChangedFlag(buttonId);
		}
		return bChanged;
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
	static inline bool delayedDown(uint8_t buttonId, uint16_t downTime)
	{
		return down(buttonId) && ((millis() - _buttonStatus[buttonId].lastChangeTime) > downTime);
	}

	static inline bool longClicked(uint8_t buttonId, uint16_t downTime, bool clearLongClickFlag)
	{
		if (delayedDown(buttonId, downTime))
		{
			bool result = _buttonStatus[buttonId].longClickFlag;
			_buttonStatus[buttonId].longClickFlag &= !clearLongClickFlag;
			return result;
		}
		return false;
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
	static inline bool down(uint8_t buttonId)
	{
		return  _buttonStatus[buttonId].currentState;
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
	static inline bool up(uint8_t buttonId)
	{
		return !down(buttonId);
	}

	/**
	 * Returns a bool value indicating if the button's state has changed since the
	 * Change Flag was last cleared.
	 * This return value is independent of whether the button itself is up or down, it
	 * need only have changed.
	 *
	 * @param buttonId          Index of the button whose status is to be checked.
	 * @param clearChangeFlag   If true, the Change Flag for this button will be cleared at the same time.
	 * @return                  true if the button's state has changed.
	 */
	static bool changed(uint8_t buttonId, bool clearChangeFlag);
	static inline bool changed(uint8_t buttonId)
	{
		return _buttonStatus[buttonId].changeFlag;
	}

	/**
	 * This method clears all Change Flags for all buttons.
	 * Useful to call when entering or leaving a user-interaction context so that spurious
	 * button presses during the "non-interactive" phase do not trigger an unexpected action.
	 */
	static void clearAllChangeFlags();

	static inline void clearChangedFlag(uint8_t buttonId)
	{
		_buttonStatus[buttonId].changeFlag = false;
	}

	/**
	 * Returns the number of buttons currently controlled by this class.
	 *
	 * @return    The number of buttons controlled by this class
	 */
	static inline uint8_t numberOfButtons()
	{
		return (_begun) ? NumberOfButtons : 0;
	}

	static inline bool polledDown(uint8_t buttonId)
	{
		return digitalRead(_buttonStatus[buttonId].buttonPin) == LOW;
	}

	//This class has only static members, therefore constructors etc are pointless.
	Buttons() = delete;
	~Buttons() = delete;
	Buttons& operator=(const Buttons&) = delete;
	Buttons(const Buttons&) = delete;

private:

	/**
	* Debounce period in milliseconds.
	*/
	static constexpr unsigned long DEBOUNCE_DELAY = 50;

	/**
	* This function is called whenever a button interrupt is fired.
	* It reads all the button states and updates their _buttonStatus objects
	* accordingly.
	*/
	static void button_ISR();

	/**
	* This array stores Button objects for each button controlled by this class,
	* each containing relevant information for the servicing of the ISR.
	* Its volatile because its members may be modified by an ISR, so we need to
	* prevent register caching of member values.
	*/
	static volatile Button _buttonStatus[NumberOfButtons];

	/**
	* Set to true if this class has been initialised, false otherwise.
	*/
	static bool _begun;

};

template <uint8_t NumberOfButtons>
bool Buttons<NumberOfButtons>::_begun = false;

template <uint8_t NumberOfButtons>
volatile Button Buttons<NumberOfButtons>::_buttonStatus[NumberOfButtons];

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
		_buttonStatus[i].buttonPin = buttonPins[i];
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
		_buttonStatus[i].currentState = !digitalRead(_buttonStatus[i].buttonPin);
		_buttonStatus[i].changeFlag = false;
		_buttonStatus[i].longClickFlag = false;
		_buttonStatus[i].lastChangeTime = millis();
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
		detachInterrupt(digitalPinToInterrupt(_buttonStatus[i].buttonPin));
	}

	//Object has been stopped.
	_begun = false;
}

template <uint8_t NumberOfButtons>
void Buttons<NumberOfButtons>::button_ISR()
{
	for (uint8_t i = 0; i < NumberOfButtons; i++)
	{
		const bool readState = !digitalRead(_buttonStatus[i].buttonPin);
		if (readState != _buttonStatus[i].currentState)
		{
			if (millis() > _buttonStatus[i].lastChangeTime + DEBOUNCE_DELAY)
			{
				_buttonStatus[i].currentState = readState;
				_buttonStatus[i].changeFlag = true;
				_buttonStatus[i].longClickFlag = true;
			}
			_buttonStatus[i].lastChangeTime = millis();
		}
	}
}

template <uint8_t NumberOfButtons>
bool Buttons<NumberOfButtons>::changed(uint8_t buttonId, bool clearChangeFlag)
{
	//Save current changeFlag before optionally clearing it.
	bool answer = _buttonStatus[buttonId].changeFlag;

	_buttonStatus[buttonId].changeFlag &= !clearChangeFlag;

	return answer;
}

template <uint8_t NumberOfButtons>
void Buttons<NumberOfButtons>::clearAllChangeFlags()
{
	for (uint8_t i = 0; i < NumberOfButtons; i++)
	{
		_buttonStatus[i].changeFlag = false;
	}
}

#endif //BUTTONS_TEMPLATE_H
