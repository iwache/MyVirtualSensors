
// Enable debug prints to serial monitor
#define MY_DEBUG

//#define MY_NODE_ID 123

//#define MY_CORE_ONLY

// Enable and select radio type attached
//#define MY_RADIO_NRF24
//#define MY_RADIO_NRF5_ESB
//#define MY_RADIO_RFM69
//#define MY_RADIO_RFM95
#define MY_ETHER

// Enable gateway ethernet module type
#define MY_GATEWAY_WIN32

// W5100 Ethernet module SPI enable (optional if using a shield/module that manages SPI_EN signal)
//#define MY_W5100_SPI_EN 4

// Enable UDP communication
//#define MY_USE_UDP  // If using UDP you need to set MY_CONTROLLER_IP_ADDRESS below

// Enable MY_IP_ADDRESS here if you want a static ip address (no DHCP)
//#define MY_IP_ADDRESS 192,168,178,66

// If using static ip you can define Gateway and Subnet address as well
//#define MY_IP_GATEWAY_ADDRESS 192,168,178,1
//#define MY_IP_SUBNET_ADDRESS 255,255,255,0

// The port to keep open on node server mode / or port to contact in client mode
//#define MY_PORT 5003

// Controller ip address. Enables client mode (default is "server" mode).
// Also enable this if MY_USE_UDP is used and you want sensor data sent somewhere.
//#define MY_CONTROLLER_IP_ADDRESS 192, 168, 178, 254

// Enable inclusion mode
//#define MY_INCLUSION_MODE_FEATURE
// Enable Inclusion mode button on gateway
//#define MY_INCLUSION_BUTTON_FEATURE
// Set inclusion mode duration (in seconds)
//#define MY_INCLUSION_MODE_DURATION 60
// Digital pin used for inclusion mode button
//#define MY_INCLUSION_MODE_BUTTON_PIN  3

// Set blinking period
//#define MY_DEFAULT_LED_BLINK_PERIOD 300

// Flash leds on rx/tx/err
// Uncomment to override default HW configurations
//#define MY_DEFAULT_ERR_LED_PIN 7  // Error led pin
//#define MY_DEFAULT_RX_LED_PIN  8  // Receive led pin
//#define MY_DEFAULT_TX_LED_PIN  9  // Transmit led pin

#if defined(MY_USE_UDP)
#include <EthernetUdp.h>
#endif
#include <Ethernet.h>
#include <EthernetUdp.h>
#include <EthernetClient.h>
#include <EthernetServer.h>

#include <MySensors.h>

// if you don't want to use DNS (and reduce your sketch size)
// use the numeric IP instead of the name for the server:
//IPAddress server(74,125,232,128);  // numeric IP for Google (no DNS)
char* serverAddress = "www.google.de";    // name address for Google (using DNS)

									 // Initialize the Ethernet client library
									 // with the IP address and port of the server
									 // that you want to connect to (port 80 is default for HTTP):
EthernetClient client;

// Initialize the Ethernet server library
// with the IP address and port you want to use
// (port 80 is default for HTTP):
EthernetServer server(80);

// Enter a MAC address for your controller below.
// Newer Ethernet shields have a MAC address printed on a sticker on the shield
byte mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};
unsigned int localPort = 8888;       // local port to listen for UDP packets
char timeServer[] = "time.nist.gov"; // time.nist.gov NTP server
const int NTP_PACKET_SIZE = 48; // NTP time stamp is in the first 48 bytes of the message
byte packetBuffer[NTP_PACKET_SIZE]; //buffer to hold incoming and outgoing packets
									// A UDP instance to let us send and receive packets over UDP
EthernetUDP Udp;

void before(void) {}

void setup() {}
void UDP_setup() 
{
	// start Ethernet and UDP
	if (Ethernet.begin(mac) == 0) {
		Serial.println("Failed to configure Ethernet using DHCP");
		// no point in carrying on, so do nothing forevermore:
		for (;;)
			;
	}
	Udp.begin(localPort);
}
void ES_setup()
{
	// start the Ethernet connection and the server:
	server.begin();
	Serial.print("server is at ");
//	Serial.println(Ethernet.localIP());
	Serial.println("unknown local IP");
}
void EC_setup()
{
	// Setup locally attached sensors
	if (client.connect(serverAddress, 80)) {
		Serial.println("connected to server");
		// Make a HTTP request:
		client.println("GET / HTTP/1.1");
		client.println("Host: www.google.de");
		client.println("Connection: close");
		client.println();
	}
}


// send an NTP request to the time server at the given address
void sendNTPpacket(char* address) 
{
	// set all bytes in the buffer to 0
	memset(packetBuffer, 0, NTP_PACKET_SIZE);
	// Initialize values needed to form NTP request
	// (see URL above for details on the packets)
	packetBuffer[0] = 0b11100011;   // LI, Version, Mode
	packetBuffer[1] = 0;     // Stratum, or type of clock
	packetBuffer[2] = 6;     // Polling Interval
	packetBuffer[3] = 0xEC;  // Peer Clock Precision
							 // 8 bytes of zero for Root Delay & Root Dispersion
	packetBuffer[12] = 49;
	packetBuffer[13] = 0x4E;
	packetBuffer[14] = 49;
	packetBuffer[15] = 52;

	// all NTP fields have been given values, now
	// you can send a packet requesting a timestamp:
	Udp.beginPacket(address, 123); //NTP requests are to port 123
	Udp.write(packetBuffer, NTP_PACKET_SIZE);
	Udp.endPacket();
}


void loop() 
{
}
void UDP_loop()
{
	sendNTPpacket(timeServer); // send an NTP packet to a time server

							   // wait to see if a reply is available
	delay(1000);
	if (Udp.parsePacket()) {
		// We've received a packet, read the data from it
		Udp.read(packetBuffer, NTP_PACKET_SIZE); // read the packet into the buffer

												 // the timestamp starts at byte 40 of the received packet and is four bytes,
												 // or two words, long. First, extract the two words:

		unsigned long highWord = word(packetBuffer[40], packetBuffer[41]);
		unsigned long lowWord = word(packetBuffer[42], packetBuffer[43]);
		// combine the four bytes (two words) into a long integer
		// this is NTP time (seconds since Jan 1 1900):
		unsigned long secsSince1900 = highWord << 16 | lowWord;
		Serial.print("Seconds since Jan 1 1900 = ");
		Serial.println(secsSince1900);

		// now convert NTP time into everyday time:
		Serial.print("Unix time = ");
		// Unix time starts on Jan 1 1970. In seconds, that's 2208988800:
		const unsigned long seventyYears = 2208988800UL;
		// subtract seventy years:
		unsigned long epoch = secsSince1900 - seventyYears;
		// print Unix time:
		Serial.println(epoch);


		// print the hour, minute and second:
		Serial.print("The UTC time is ");       // UTC is the time at Greenwich Meridian (GMT)
		Serial.print((epoch % 86400L) / 3600); // print the hour (86400 equals secs per day)
		Serial.print(':');
		if (((epoch % 3600) / 60) < 10) {
			// In the first 10 minutes of each hour, we'll want a leading '0'
			Serial.print('0');
		}
		Serial.print((epoch % 3600) / 60); // print the minute (3600 equals secs per minute)
		Serial.print(':');
		if ((epoch % 60) < 10) {
			// In the first 10 seconds of each minute, we'll want a leading '0'
			Serial.print('0');
		}
		Serial.println(epoch % 60); // print the second
	}
	// wait ten seconds before asking for the time again
	delay(9000);
	Ethernet.maintain();
}
void ES_loop()
{
	// listen for incoming clients
	EthernetClient client = server.available();
	if (client) {
		Serial.println("new client");
		// an http request ends with a blank line
		bool currentLineIsBlank = true;
		while (client.connected()) {
			if (client.available()) {
				char c = client.read();
				Serial.write(c);
				// if you've gotten to the end of the line (received a newline
				// character) and the line is blank, the http request has ended,
				// so you can send a reply
				if (c == '\n' && currentLineIsBlank) {
					// send a standard http response header
					client.println("HTTP/1.1 200 OK");
					client.println("Content-Type: text/html");
					client.println("Connection: close");  // the connection will be closed after completion of the response
					client.println("Refresh: 1");  // refresh the page automatically every 5 sec
					client.println();
					client.println("<!DOCTYPE HTML>");
					client.println("<html>");
					// output the value of each analog input pin
					for (int analogChannel = 0; analogChannel < 6; analogChannel++) {
						int sensorReading = random(1024); //analogRead(analogChannel);
						client.print("analog input ");
						client.print(analogChannel);
						client.print(" is ");
						client.print(sensorReading);
						client.println("<br />");
					}
					client.println("</html>");
					break;
				}
				if (c == '\n') {
					// you're starting a new line
					currentLineIsBlank = true;
				}
				else if (c != '\r') {
					// you've gotten a character on the current line
					currentLineIsBlank = false;
				}
			}
		}
		// give the web browser time to receive the data
		delay(1);
		// close the connection:
//		client.stop();
		client.flush();
		client.close();

		Serial.println("client disconnected");
	}
}
void EC_loop()
{
	// Send locally attached sensors data here
	while (client.available()) {
		char c = client.read();
		Serial.write(c);
	}

	// if the server's disconnected, stop the client:
	if (!client.connected()) {
		Serial.println();
		Serial.println("disconnecting from server.");
		client.stop();

		// do nothing forevermore:
		while (true);
	}
}

void presentation() {
	// Present locally attached sensors here
}

void receiveTime(uint32_t value) {
}

void receive(const MyMessage &message) {
}

void EC_preHwInit(void) {
	EC_setup();
	while (true)
	{
		EC_loop();
		delay(10);
	}
}
void ES_preHwInit(void) {
	ES_setup();
	while (true)
	{
		ES_loop();
		delay(10);
	}
}
void UDP_preHwInit(void) {
	UDP_setup();
	while (true)
	{
		UDP_loop();
		delay(10);
	}
}
void preHwInit(void)
{
	//EC_preHwInit();
	//ES_preHwInit();
	UDP_preHwInit();
}
