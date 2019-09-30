//#define LONG_RELEASE_DELAY 2000 //ms. Optional to override the default value
//#define DOUBLE_CLICK_DELAY 300 //ms. Optional to override the default value
#include <buttonsTemplate.h>

// Buttons
#define NUMBEROFBUTTONS	2
//#define BUTTON1_PIN		6 // D6 on Arduino Nano; PB3 on Blue Pill
//#define BUTTON2_PIN		0 // D0 on Arduino Nano; PB9 on Blue Pill
#define BUTTON1_PIN		13 // PF0 on APRS_Tracker_STM32F070F6
#define BUTTON2_PIN		1 // PA1 on APRS_Tracker_STM32F070F6

enum ButtonIds : uint8_t {	Button1, Button2 };

const uint8_t buttonPins[] = { BUTTON1_PIN, BUTTON2_PIN };

using buttons = Buttons<NUMBEROFBUTTONS>;

void setup()
{
	Serial.begin(115200);
	delay(500);
	
	Serial.println("Button Test");

	buttons::begin(buttonPins);
}

void loop()
{
	if (buttons::clicked(Button1))
		Serial.println("1 clicked");

	if (buttons::shortReleased(Button1))
		Serial.println("1 shortReleased");

	if (buttons::longReleased(Button1))
		Serial.println("1 longReleased");

	if (buttons::doubleClicked(Button1))
		Serial.println("1 doubleClicked");

	if (buttons::clicked(Button2))
		Serial.println("2 clicked");

	if (buttons::shortReleased(Button2))
		Serial.println("2 shortReleased");

	if (buttons::longReleased(Button2))
		Serial.println("2 longReleased");

	if (buttons::doubleClicked(Button2))
		Serial.println("2 doubleClicked");
}
