#include <buttonsSingle.h>

void ButtonSingle::begin(uint8_t buttonPin)
{
	pin = buttonPin;
	pinMode(buttonPin, INPUT_PULLUP);

	// Need to wait some time before setting up the ISRs, otherwise you can get spurious
	// changes as the pullup hasn't quite done its magic yet.
	delay(10);

	//Set up the interrupts on the pin.
	attachInterrupt(digitalPinToInterrupt(buttonPin), ButtonSingle::buttonISR, CHANGE);

	// initialize button state
	state = digitalRead(pin) ? CLEAR_FLAGS : PRESSED_FLAG;
	lastClickTime = lastChangeTime = millis();
}

void ButtonSingle::stop()
{
	//Disable the interrupts
	detachInterrupt(digitalPinToInterrupt(pin));
}

void ButtonSingle::button_Handler()
{
	uint32_t now = millis();
	const bool readState = !digitalRead(pin);
	if (readState != (state && PRESSED_FLAG))
	{
		if (now - lastChangeTime > DEBOUNCE_DELAY)
		{
			if (readState) // button has been clicked
			{
				state = CLEAR_FLAGS;
				state = PRESSED_FLAG | CLICKED_FLAG;

				if (now - lastClickTime > DOUBLE_CLICK_DELAY)
				{
					state |= DOUBLE_CLICKED_FLAG;
				}
				lastClickTime = now;
			}
			else
			{ // button has been released
				state &= ~PRESSED_FLAG;
				state |= RELEASED_FLAG;
				if (now - lastClickTime > LONG_CLICK_DELAY)
				{
					state |= LONG_CLICKED_FLAG;
				}
			}
		}
		lastChangeTime = now;
	}
}

void ButtonSingle::buttonISR()
{

}
