# Arduino Buttons Template Library

Vital H B Software - 2017

An interrupt-driven, fully-debounced template class to manage input from physical buttons on the Arduino platform.
Based on the work of Nicholas Parks Young: https://github.com/Alarm-Siren/arduino-buttons

Although in principle this class is compatible with all members of the Arduino platform, note that it is limited to only having buttons connected to those pins that accept interrupts - on the ARM boards (Due, Zero etc) this is all pins, but on AVR boards (e.g. Uno, Leonardo, Mega 2560, etc) these pins are in limited supply.

Note that for pragmatic reasons, this is a fully static class. This means you do not need to create any instances of it, besides the instantiaton of the template itself:

...
#define NUMBEROFBUTTONS	2
#define BUTTON1_PIN		2
#define BUTTON2_PIN		3

enum ButtonIds : uint8_t {	Button1, Button2 };

const uint8_t buttonPins[] = { BUTTON1_PIN, BUTTON2_PIN };

template class Buttons&#60;NUMBEROFBUTTONS&#62;; // use like this: Buttons&#60;NUMBEROFBUTTONS&#62;::begin(buttonPins);
OR
using buttons = Buttons&#60;NUMBEROFBUTTONS&#62;; // use like this: buttons::begin(buttonPins);
...

## Library Setup
Just #include the buttonsTemplate.h file to your .ino source file and any other files that will reference the buttons template class. For more details, see the example program included with the library.

## Comments, Requests, Bugs & Contributions
All are welcome. Please file an "Issue" in the Bug Tracker.

## License
This library is licensed under the GNU LGPL v2.1, which can be found in the LICENSE file.
