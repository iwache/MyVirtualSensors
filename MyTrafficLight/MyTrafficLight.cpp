// Enable debug prints to serial monitor
#define MY_DEBUG

// Enable and select radio type attached
#if defined(WIN32)
#define MY_ETHER_TCP
#else
#define MY_RADIO_NRF24
#endif

#define MY_FIRMATA_CLIENT
#define MY_FIRMATA_NETWORK_HOST "192.168.16.112" // my Arduino MKR1000 with WiFi Server Firmata

#if defined(WIN32)
#include "../../libraries/MySensors/MySensors.h"
#else
#include <MySensors.h>
#endif

#if defined(WIN32)
#define D0 16
#define D1 5
#define D2 4
#define D3 0
#define D4 2
#define D5 14
#define D6 12
#define D7 13
#define D8 15
#define D9 3
#define D10 1

#define LED_BUILTIN D0
#endif

// Traffic light LEDs
#define LED_RED D5 // red light
#define LED_YLW D6 // yellow light
#define LED_GRN D7 // green light

#define DISCO_DELAY 100 // switch delay in disco mode

typedef enum
{
	LightOff,
	LightRed,
	LightYellow,
	LightGreen
} TrafficLightMode;

TrafficLightMode lightMode = LightOff;
bool discoMode = false;
uint8_t discoState = 0;
uint32_t lastTime = 0;
static uint16_t discoDelay = DISCO_DELAY;
static bool startBlinkLed = false;

#define BLINK_LED_DELAY 500
#define LED_PIN LED_BUILTIN

//###################################################

boolean debug = true;

const int busyPin = D1;
const int resetPin = D2;
const int clockPin = D3;
const int dataPin = D4;

int busyPinState = HIGH;
int volume = 4;

void sendCommand(unsigned int command)
{
	if (debug) Serial.println("--------------");
	if (debug) Serial.print("Command (");
	if (debug) Serial.print(command, HEX);
	if (debug) Serial.println(")");

	//Start bit Low level pulse.
	digitalWrite(clockPin, LOW);
	delay(2);


	// * 0x8000 = 100000000 00000000 = 16 Bits, Send Command as Serial Transfer *
	for (unsigned int mask = 0x8000; mask > 0; mask >>= 1) {

		digitalWrite(clockPin, LOW);
		delay(1); //delayMicroseconds(50);

		// * Set Data-Bit on the Line *
		if (command & mask) {

			if (debug) Serial.print("1");
			digitalWrite(dataPin, HIGH);
		}
		else {

			if (debug) Serial.print("0");
			digitalWrite(dataPin, LOW);
		}
		if (debug) Serial.print(" ");

		// * Wait 50us *
		delay(1); //delayMicroseconds(50);

		// * Clock: High ... time to get Data-Bit *
		digitalWrite(clockPin, HIGH);

		// * Wait some time to give time to read the bit (possible values: 5-6500us) *
		delay(1); //delayMicroseconds(50);
	}

	if (debug) Serial.println(" ");


	// * Give Busy time to go "High" *
	delay(50);
}

void resetModule()
{
	digitalWrite(clockPin, HIGH);
	digitalWrite(resetPin, HIGH);
	delay(5);
	digitalWrite(resetPin, LOW);
	delay(5);
	digitalWrite(resetPin, HIGH);

	// * Wait *
	delay(700);

	// * Volume has to be "FULL VOLUME" 7  ... must be set, because after Power on or Reset Button sometimes the sound will not start! *
	sendCommand(0xfff7);
	delay(800);
}


void startSoundInBackground(int soundNumber) {

	sendCommand(soundNumber);
}

void startSoundAndWaitTilEnd(int soundNumber) {

	sendCommand(soundNumber);

	busyPinState = digitalRead(busyPin);
	while (busyPinState == HIGH) busyPinState = digitalRead(busyPin);
}

void stopSound() {
	sendCommand(0xFFFF);
}

// * Volume 0-4 (0 = Off) *
void setVolume(int targetVol) {

	// * the value can not be lower 4, volume < 4 = random volume modulation => distrotion *
	if (targetVol >= 0 && targetVol < 5) {

		if (targetVol == 0)   sendCommand(0xFFF0);
		else {

			if (targetVol > volume) {

				int steps = targetVol - volume;
				for (int i = 0; i<steps; i++) {
					volume++;
					sendCommand(0xFFF0 + 3 + volume);
					delay(800); // * without this delay... the sound will destroyed *
				}
			}
			else if (targetVol < volume) {

				int steps = volume - targetVol;
				for (int i = 0; i<steps; i++) {
					volume--;
					sendCommand(0xFFF0 + 3 + volume);
					delay(800); // * without this delay... the sound will destroyed *
				}
			}
		}
	}
}

void loopSound()
{

	// * A short testcase: *

	for (int8_t i = 0; i < 10; i++)
	{
//		resetModule();
		startSoundInBackground(1);
	}

	startSoundInBackground(1);
	wait(4000);
	stopSound();

	startSoundInBackground(1);
	delay(2000);
	stopSound();

	setVolume(volume);
	delay(20000);

	/*
	setVolume(1);
	delay(3000);
	setVolume(3);
	delay(3000);
	setVolume(1);
	delay(3000);
	setVolume(0);
	delay(3000);
	setVolume(4);
	delay(3000);
	setVolume(2);
	delay(3000);
	setVolume(3);
	delay(3000);
	setVolume(4);
	delay(3000);
	*/

	delay(2000);
}


//###################################################


void loopBlinkLED()
{
	static bool active = false;
	static uint32_t lastTime = 0;

	if (startBlinkLed) {
		startBlinkLed = false;
		active = true;
		lastTime = millis();
	}

	if (active) {
		if (millis() - lastTime > BLINK_LED_DELAY) {
			digitalWrite(LED_PIN, HIGH);
			active = false;
		}
		else {
			digitalWrite(LED_PIN, LOW);
		}
	}
}


void setTrafficLightMode(TrafficLightMode mode)
{
	lightMode = mode;
	switch (mode)
	{
	case LightOff:
		digitalWrite(LED_RED, LOW);
		digitalWrite(LED_YLW, LOW);
		digitalWrite(LED_GRN, LOW);
		break;
	case LightRed:
		digitalWrite(LED_RED, HIGH);
		digitalWrite(LED_YLW, LOW);
		digitalWrite(LED_GRN, LOW);
		break;
	case LightYellow:
		digitalWrite(LED_RED, LOW);
		digitalWrite(LED_YLW, HIGH);
		digitalWrite(LED_GRN, LOW);
		break;
	case LightGreen:
		digitalWrite(LED_RED, LOW);
		digitalWrite(LED_YLW, LOW);
		digitalWrite(LED_GRN, HIGH);
		break;
	}
}

void readCommands()
{
	if (Serial.available()) {
		byte inChar = Serial.read();

		if (inChar == 'R' || inChar == 'r') {
			discoMode = false;
			setTrafficLightMode(LightRed);
		}
		else if (inChar == 'Y' || inChar == 'y') {
			discoMode = false;
			setTrafficLightMode(LightYellow);
		}
		else if (inChar == 'G' || inChar == 'g') {
			discoMode = false;
			setTrafficLightMode(LightGreen);
		}
		else if (inChar == 'O' || inChar == 'o') {
			discoMode = false;
			setTrafficLightMode(LightOff);
		}
		else if (inChar == 'D' || inChar == 'd') {
			discoMode = true;
			discoState = 3;
			lastTime = millis() - discoDelay;
		}
		else if (inChar == 'B' || inChar == 'b') {
			startBlinkLed = true;
		}

		else {
			Serial.println("Invalid input!");
		}
	}
}

void updateDiscoMode()
{
	if (!discoMode) {
		return;
	}

	uint32_t thisTime = millis();
	if (thisTime - lastTime >= discoDelay) {

		discoState++;
		if (discoState > 3) {
			discoState = 0;
		}

		switch (discoState)
		{
		case 0:
			setTrafficLightMode(LightRed);
			break;
		case 1:
			setTrafficLightMode(LightYellow);
			break;
		case 2:
			setTrafficLightMode(LightGreen);
			break;
		case 3:
			setTrafficLightMode(LightYellow);
			break;
		default:
			break;
		}
		lastTime = thisTime;
	}
}

void setup()
{
	pinMode(LED_RED, OUTPUT);          // sets the digital pins for traffic light LEDs as output
	pinMode(LED_YLW, OUTPUT);
	pinMode(LED_GRN, OUTPUT);
	pinMode(LED_PIN, OUTPUT);

	pinMode(resetPin, OUTPUT);
	pinMode(clockPin, OUTPUT);
	pinMode(dataPin, OUTPUT);
	pinMode(busyPin, INPUT);

	delay(1000); // * Wait for the Pins *
	resetModule();
}

void loop()
{
	loopSound();

	readCommands();
	updateDiscoMode();

	loopBlinkLED();
	return;


	digitalWrite(LED_RED, HIGH);
	delay(1000);
	digitalWrite(LED_RED, LOW);
	digitalWrite(LED_YLW, HIGH);
	delay(1000);
	digitalWrite(LED_YLW, LOW);
	digitalWrite(LED_GRN, HIGH);
	delay(1000);
	digitalWrite(LED_GRN, LOW);
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

void presentation() {}

void receive(const MyMessage &message) {}

void receiveTime(uint32_t value) {}
#endif

