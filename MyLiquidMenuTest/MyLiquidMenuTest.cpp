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

#include "Button.h"

#define A0 15
#define A1 16

// pin definitions
#define OLED_DC 2
#define OLED_CS 3
#define OLED_RST 5

#define LED 6 // 6 = LED_BUILTIN on MKR1000
//#define BUTTON 7 // Button connected to pin 7 and GND; we need a pullup input
Encoder myEncoder1(0, 1);
LiquidCrystal_SSD1306 oled;

int lastEncoderValue1;

const byte startingScreen = 2;

const bool pullup = true;
Button *left;
Button *right;
//Button up(8, pullup);
//Button down(9, pullup);
Button *enter;

enum FunctionTypes {
	increase = 1,
	decrease = 2,
};

// These are the pin definitions and variables for their PWM state.
const byte led = 6;
byte led_level = 0;

// Variables used for setting "preferences".
bool isFading = false;
char* isFading_text;
unsigned int fadePeriod = 100;
bool isBlinking = false;
char* isBlinking_text;
unsigned int blinkPeriod = 1000;


LiquidLine welcome_line1(1, 0, "LiquidMenu ", LIQUIDMENU_VERSION);
LiquidLine welcome_line2(1, 1, "Functions ex.");
LiquidLine welcome_line3(1, 2, "");
LiquidLine welcome_line4(1, 3, "LED Level: ", led_level);
LiquidScreen welcome_screen(welcome_line1, welcome_line2, welcome_line3, welcome_line4);

LiquidLine ledTitleLine(6, 0, "LED");
LiquidLine led_line(4, 1, "Level: ", led_level);
LiquidScreen led_screen(ledTitleLine, led_line);

LiquidLine fade_line(1, 0, "Fade - ", isFading_text);
LiquidLine fadePeriod_line(1, 1, "Period: ", fadePeriod, "ms");
LiquidScreen fade_screen(fade_line, fadePeriod_line);

LiquidLine blink_line(1, 0, "Blink - ", isBlinking_text);
LiquidLine blinkPeriod_line(1, 1, "Period: ", blinkPeriod, "ms");
LiquidLine fade_line2(1, 2, "Fade - ", isFading_text);
LiquidLine fadePeriod_line2(1, 3, "Period: ", fadePeriod, "ms");
LiquidScreen blink_screen(blink_line, blinkPeriod_line, fade_line2, fadePeriod_line2);

LiquidMenu menu(oled); // , startingScreen);

// Callback functions
void increase_led_level() {
	if (led_level <= 225) {
		led_level += 25;
	}
	else {
		led_level = 0;
	}
	analogWrite(led, led_level);
}

void decrease_led_level() {
	if (led_level >= 25) {
		led_level -= 25;
	}
	else {
		led_level = 250;
	}
	analogWrite(led, led_level);
}

void led_off() {
	led_level = 0;
	analogWrite(led, led_level);
}

void fade_switch() {
	led_off();
	if (isFading == true) {
		isFading = false;
		isFading_text = (char*)"OFF";
	}
	else {
		isFading = true;
		isFading_text = (char*)"ON";
		isBlinking = false;
		isBlinking_text = (char*)"OFF";
	}
}

void increase_fadePeriod() {
	if (fadePeriod < 3000) {
		fadePeriod += 10;
	}
}

void decrease_fadePeriod() {
	if (fadePeriod > 10) {
		fadePeriod -= 10;
	}
}

void blink_switch() {
	led_off();
	if (isBlinking == true) {
		isBlinking = false;
		isBlinking_text = (char*)"OFF";
	}
	else {
		isBlinking = true;
		isBlinking_text = (char*)"ON";
		isFading = false;
		isFading_text = (char*)"OFF";
	}
}

void increase_blinkPeriod() {
	if (blinkPeriod < 3000) {
		blinkPeriod += 50;
	}
}

void decrease_blinkPeriod() {
	if (blinkPeriod > 50) {
		blinkPeriod -= 50;
	}
}

// Checks all the buttons.
void buttonsCheck() {
	if (right->check() == LOW) {
		menu.next_screen();
	}
	if (left->check() == LOW) {
		menu.previous_screen();
	}

	int encoderValue = myEncoder1.read() / 4;
	if (encoderValue != lastEncoderValue1) {
		Serial.print("Encoder 1: ");
		Serial.println(encoderValue);

		if (encoderValue > lastEncoderValue1)
			menu.call_function(increase, true);
		else
			menu.call_function(decrease, true);

		lastEncoderValue1 = encoderValue;
	}
/*
	if (up.check() == LOW) {
		// Calls the function identified with
		// increase or 1 for the focused line.
		menu.call_function(increase);
	}
	if (down.check() == LOW) {
		menu.call_function(decrease);
	}
*/
	if (enter->check() == LOW) {
		// Switches focus to the next line.
		menu.switch_focus();
	}
}

// The fading function.
void fade() {
	static bool goingUp = true;
	if (goingUp) {
		led_level += 25;
	}
	else {
		led_level -= 25;
	}
	if (led_level > 225) {
		goingUp = false;
		led_level = 250;
	}
	if (led_level < 25 && goingUp == false) {
		goingUp = true;
		led_level = 0;
	}
	analogWrite(led, led_level);
}

// The blinking function.
void blink() {
	static bool blinkState = LOW;
	if (blinkState == LOW) {
		blinkState = HIGH;
		led_level = 255;
	}
	else {
		blinkState = LOW;
		led_level = 0;
	}
	analogWrite(led, led_level);
}

//--------------------------------------------------------------------------------------

void setup()
{
	pinMode(led, OUTPUT);

	left = new Button(A0, pullup);
	right = new Button(A1, pullup);
	enter = new Button(7, pullup);

	lastEncoderValue1 = myEncoder1.read() / 4;

	oled.begin(&Adafruit128x64, OLED_CS, OLED_DC, OLED_RST);
	oled.setFont(LiquidMenu8x16); // ZevvPeep8x16);
//	oled.println("OLED Font set!");

	// The increasing functions are attached with identification of 1.
	/*
	* This function can later be called by pressing the 'UP' button
	* when 'led_line' is focused. If some other line is focused it's
	* corresponding function will be called.
	*/
	led_line.attach_function(increase, increase_led_level);
	// The decreasing functions are attached with identification of 2.
	led_line.attach_function(decrease, decrease_led_level);

	// Here the same function is attached with two different identifications.
	// It will be called on 'UP' or 'DOWN' button press.
	fade_line.attach_function(1, fade_switch);
	fade_line.attach_function(2, fade_switch);
	fadePeriod_line.attach_function(increase, increase_fadePeriod);
	fadePeriod_line.attach_function(decrease, decrease_fadePeriod);

	blink_line.attach_function(1, blink_switch);
	blink_line.attach_function(2, blink_switch);
	blinkPeriod_line.attach_function(increase, increase_blinkPeriod);
	blinkPeriod_line.attach_function(decrease, decrease_blinkPeriod);

	fade_line2.attach_function(1, fade_switch);
	fade_line2.attach_function(2, fade_switch);
	fadePeriod_line2.attach_function(increase, increase_fadePeriod);
	fadePeriod_line2.attach_function(decrease, decrease_fadePeriod);

	menu.add_screen(welcome_screen);
	menu.add_screen(led_screen);
	menu.add_screen(fade_screen);
	menu.add_screen(blink_screen);

	isFading_text = (char*)"OFF";
	isBlinking_text = (char*)"OFF";

//	menu.update();
//	menu.set_focusPosition(Position::LEFT);
	menu.switch_focus();
}

void menuSoftUpdate()
{
	if (menu.currentScreen() == 0 || menu.currentScreen() == 1)
		menu.softUpdate();
}

void loop() 
{
	buttonsCheck();

	static unsigned long lastMillis_blink = 0;
	if ((isFading == true) && ((millis() - lastMillis_blink) > fadePeriod)) {
		lastMillis_blink = millis();
		fade();
		menuSoftUpdate();
	}

	static unsigned long lastMillis_fade = 0;
	if ((isBlinking == true) && ((millis() - lastMillis_fade) > blinkPeriod)) {
		lastMillis_fade = millis();
		blink();
		menuSoftUpdate();
	}

	wait(100);
}

// for Visual C++ we need following functions at least as empty stubs
#if defined(WIN32)
void preHwInit(void)
{
	setup();
	while (true)
	{
		loop();
		wait(5);
	}
}

void before(void) {}

void presentation() {}

void receive(const MyMessage &message) {}

void receiveTime(uint32_t value) {}
#endif

