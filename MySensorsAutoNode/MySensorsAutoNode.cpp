// Enable debug prints to serial monitor
#define MY_DEBUG

// Enable and select radio type attached
//#define MY_RADIO_NRF24
#define MY_ETHER_TCP

#if defined(WIN32)
#include "../../libraries/MySensors/MySensors.h"
#else
#include <MySensors.h>
#endif

void setup()
{

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

