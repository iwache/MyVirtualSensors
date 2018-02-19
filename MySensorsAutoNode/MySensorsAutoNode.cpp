// Enable debug prints to serial monitor
#define MY_DEBUG

// Enable and select radio type attached
//#define MY_RADIO_NRF24
#define MY_ETHER

#include <MySensors.h>

void setup()
{
	boolean test = false;
	
}

void presentation()
{
	// Present locally attached sensors here
	sendSketchInfo("MySensorsAutoNode", "1.0");
}

void loop()
{
	
}

void receive(const MyMessage &message)
{
	
}

// for Visual C++ we need following functions at least as empty stubs
#if defined(WIN32)
void preHwInit(void) {}

void before(void) {}

void receiveTime(uint32_t value) {}
#endif

