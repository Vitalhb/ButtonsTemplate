#include <buttonsTemplate.h>

// Buttons
#define NUMBEROFBUTTONS	2
#define BUTTON1_PIN		2
#define BUTTON2_PIN		3

enum ButtonIds : uint8_t {	Button1, Button2 };

const uint8_t buttonPins[] = { BUTTON1_PIN, BUTTON2_PIN };

using buttons = Buttons<NUMBEROFBUTTONS>;

void setup()
{
	Serial.begin(115200);

	buttons::begin(buttonPins);

}

void loop()
{
	if (buttons::released(Button1, true))
		Serial.println("Button1 released");

	if (buttons::clicked(Button2, true))
		Serial.println("Button2 clicked");

}
