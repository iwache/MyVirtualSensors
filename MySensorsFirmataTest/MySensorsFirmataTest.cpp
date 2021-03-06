// Enable debug prints to serial monitor
#define MY_DEBUG

// Enable and select radio type attached
//#define MY_RADIO_NRF24
#define MY_ETHER_TCP

#define MY_FIRMATA_CLIENT
#define MY_FIRMATA_NETWORK_HOST "192.168.16.107" // my Arduino MKR1000 with WiFi Server Firmata

#if defined(WIN32)
#include "ProjectIncludes.h"
#else
#include <MySensors.h>
#include <Encoder.h>
#include <SSD1306Ascii.h>
#include <SSD1306AsciiSpi.h>
#include <LiquidMenu.h>
#endif

#if defined(WIN32)
#define A0 15
#define A1 16
#define A6 21
#endif

// pin definitions
#define OLED_DC 2
#define OLED_CS A6 // 3
#define OLED_RST 5

const char* fontName[] = {
	"Arial14",
	"Arial_bold_14",
	"Callibri11",
	"Callibri11_bold",
	"Callibri11_italic",
	"Callibri15",
	"Corsiva_12",
	"fixed_bold10x15",
	"font5x7",
	"font8x8",
	"Iain5x7",
	"lcd5x7",
	"Stang5x7",
	"System5x7",
	"TimesNewRoman16",
	"TimesNewRoman16_bold",
	"TimesNewRoman16_italic",
	"utf8font10x16",
	"Verdana12",
	"Verdana12_bold",
	"Verdana12_italic",
	"X11fixed7x14",
	"X11fixed7x14B",
	"ZevvPeep8x16"
};
const uint8_t* fontList[] = {
	Arial14,
	Arial_bold_14,
	Callibri11,
	Callibri11_bold,
	Callibri11_italic,
	Callibri15,
	Corsiva_12,
	fixed_bold10x15,
	font5x7,
	font8x8,
	Iain5x7,
	lcd5x7,
	Stang5x7,
	System5x7,
	TimesNewRoman16,
	TimesNewRoman16_bold,
	TimesNewRoman16_italic,
	utf8font10x16,
	Verdana12,
	Verdana12_bold,
	Verdana12_italic,
	X11fixed7x14,
	X11fixed7x14B,
	ZevvPeep8x16
};
uint8_t nFont = sizeof(fontList) / sizeof(uint8_t*);


//SSD1306AsciiSpi oled;

#define LED 6 // 6 = LED_BUILTIN on MKR1000
#define BUTTON 7 // Button connected to pin 7 and GND; we need a pullup input


//Encoder myEncoder2(7, 8);
Encoder myEncoder1(0, 1);

LiquidCrystal_SSD1306 oled; // (OLED_CS, OLED_DC, OLED_RST);
/*
* Variable 'analogReading' is later configured to
* be printed on the display. 'lastAnalogReading'
* is used to check if the variable has changed.
*/
short analogReading = 0;

// Here the line is set to column 1, row 0 and will print the passed
// string and the passed variable.
LiquidLine welcome_line1(1, 0, "LiquidMenu ", LIQUIDMENU_VERSION);
// Here the column is 3, the row is 1 and the string is "Hello Menu".
LiquidLine welcome_line2(3, 1, "Hello Menu");

/*
* LiquidScreen objects represent a single screen. A screen is made of
* one or more LiquidLine objects. Up to four LiquidLine objects can
* be inserted from here, but more can be added later in setup() using
* welcome_screen.add_line(someLine_object);.
*/
// Here the LiquidLine objects are the two objects from above.
LiquidScreen welcome_screen(welcome_line1, welcome_line2);

// Here there is not only a text string but also a changing integer variable.
LiquidLine analogReading_line(0, 0, "Analog: ", analogReading, " ms   ");
LiquidScreen secondary_screen(analogReading_line);

/*
* The LiquidMenu object combines the LiquidScreen objects to form the
* menu. Here it is only instantiated and the screens are added later
* using menu.add_screen(someScreen_object);. This object is used to
* control the menu, for example: menu.next_screen(), menu.switch_focus()...
*/
LiquidMenu menu(oled);




void testOled()
{
	oled.begin(&Adafruit128x64, OLED_CS, OLED_DC, OLED_RST);

	//oled.setScroll(true);
	//oled.setFont(System5x7);
	////oled.clear();
	//oled.println("This is a very long line.");


	for (uint8_t i = nFont - 1; i < nFont; i++) {
		oled.setFont(System5x7);
		oled.clear();
		oled.println(fontName[i]);
		oled.println();
		oled.setFont(fontList[i]);
		oled.println("*+,-./0123456789:");
		oled.println("abcdefghijklmno");
		oled.println("ABCDEFGHIJKLMNO");
		delay(1000);
	}
//	oled.setFont(System5x7);
//	oled.setScroll(true);
//	oled.clear();
	oled.println("Fonts done!");

}

void setup()
{
	testOled();

//	oled.println("Setup begin.");

	pinMode(LED, OUTPUT);          // sets the digital pin 13 as output
	pinMode(BUTTON, INPUT_PULLUP);
//	oled.println("Setup done.");

	menu.add_screen(welcome_screen);
	menu.add_screen(secondary_screen);
}

void presentation()
{
	// Present locally attached sensors here
	sendSketchInfo("MySensorsAutoNode", "1.0");
}

void loop()
{
	static int lastEncoderValue1 = -1;
	static int lastEncoderValue2 = -1;

	static int line = 0;

//	menu.update();

//	oled.print(' ');
	//oled.print(line++);
	//oled.println(" - Hello world!");

	static bool lastValue = false;

	bool value = !digitalRead(BUTTON);
	digitalWrite(LED, value);

	if (value && !lastValue)
	{
		menu.next_screen();
//		menu.update();
	}
	lastValue = value;

	int encoderValue = myEncoder1.read() / 4;
	if (encoderValue != lastEncoderValue1) {
		Serial.print("Encoder 1: ");
		Serial.println(encoderValue);
		lastEncoderValue1 = encoderValue;

		analogReading = encoderValue;
		menu.softUpdate(); // update();

	//	if (abs(encoderValue) > 10)
	//		myEncoder1.write(0);
	}
	wait(100);
	return;

	//encoderValue = myEncoder2.read() / 3;
	//if (encoderValue != lastEncoderValue2) {
	//	Serial.print("Encoder 2: ");
	//	Serial.println(encoderValue);
	//	lastEncoderValue2 = encoderValue;

	//	if (abs(encoderValue) > 10)
	//		myEncoder2.write(0);
	//}
	wait(50);
//	return;

	digitalWrite(LED, HIGH);       // sets the digital pin 13 on
	wait(500);                     // waits for a second
	digitalWrite(LED, LOW);        // sets the digital pin 13 off
	wait(1000);                     // waits for a second

//	static int line = 0;

//	oled.print(' ');
//	oled.print(line++);
	oled.println(" - Hello world!");
}

void receive(const MyMessage &message)
{
	
}

// for Visual C++ we need following functions at least as empty stubs
#if defined(WIN32)
void preHwInit(void) 
{
	wait(100);

	setup();
	while (true)
	{
		loop();
		wait(0);
	}

}

void before(void) {}

void receiveTime(uint32_t value) {}
#endif

