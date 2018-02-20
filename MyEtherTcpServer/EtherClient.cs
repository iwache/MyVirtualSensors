using VirtualHardwareNet;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace MyEtherTcpServer
{
    public class EtherClient : EthernetClientNet
    {
        // We only use SYS_PACK in this application
        private const byte ETHER_PACKET = 0x55;
        private const byte ETHER_ADDR = 0xAA;

        private const byte BROADCAST_ADDRESS = 255;

        // Receiving header information
        private byte[] _header = new byte[7];

        // Reception state machine control and storage variables
        private byte _recPhase;
        private byte _recPos;
        private byte _recCommand;
        private byte _recNoAck;
        private byte _recLen;
        private byte _recStation;
        private byte _recSender;
        private byte _recCS;
        private byte _recCalcCS;

        //private byte _nodeId;
        private byte[] _data = new byte[32];
//        private byte _packetLen;
//        private byte _packetFrom;

        // Packet wrapping characters, defined in standard ASCII table
        private const byte SOH = 1;
        private const byte STX = 2;
        private const byte ETX = 3;
        private const byte EOT = 4;

        public byte Address { get; private set; }
        public EtherServer Server { get; private set; }

        public EtherClient(EtherServer server) : base()
        {
            Address = 255;
            Server = server;
        }

        public void Process()
        {
            decodeTelegram();
        }

        public bool TransportSend(byte source, byte to, byte[] data, byte len, bool noACK)
        {
            byte cs = 0;
            byte noAck = noAckToByte(noACK);

            //hwDebugPrint("TS to: %u len: %u\n", to, len);

            write(SOH); // Start of header by writing SOH
            write(ETHER_PACKET); // ETHER Data packet telegram
            cs += ETHER_PACKET;
            write(to); // Destination address
            cs += to;
            write(source); // Source address
            cs += source;
            write(noAck); // Packet acknowledge flag
            cs += noAck;
            write(len); // Length of text
            cs += len;
            write(STX); // Start of text
            for (int i = 0; i < len; i++)
            {
                write(data[i]); // Text bytes
                cs += data[i];
            }
            write(ETX); // End of text
            write(cs);
            write(EOT); // End of transmission

            flush();
            return true;
        }


        private bool decodeTelegram()
        {
            int i;
            if (available() == 0)
            {
                return false;
            }

            while (available() > 0)
            {
                byte inch = (byte)read();

                switch (_recPhase)
                {

                    // Case 0 looks for the header. Bytes arrive in the TCP client interface and get
                    // shifted through a header buffer. When the start and end characters in
                    // the buffer match the SOH/STX pair and valid Ether command, 
                    // save the header information and progress to the next state.
                    case 0:
                        Buffer.BlockCopy(_header, 1, _header, 0, 6);

                        _header[6] = inch;
                        if ((_header[3] == SOH) && (_header[4] == ETHER_ADDR) && (_header[5] != BROADCAST_ADDRESS) && (_header[6] != _header[EOT]))
                        {
                            // Set client MySensors node address (NODE_ID)
                            Address = _header[5];
                            // Register as valid Ether node
                            Server.AddEtherNode(this);
                        }
                        else if ((_header[0] == SOH) && (_header[1] == ETHER_PACKET) && (_header[6] == STX)) // && (_header[2] != _header[3]))
                        {
                            // Packet header received
                            _recCalcCS = 0;
                            _recCommand = _header[1];
                            _recStation = _header[2];
                            _recSender = _header[3];
                            _recNoAck = _header[4];
                            _recLen = _header[5];

                            for (i = 1; i < 6; i++)
                            {
                                _recCalcCS += _header[i];
                            }
                            _recPhase = 1;
                            _recPos = 0;

                            if (_recLen == 0)
                            {
                                _recPhase = 2;
                            }

                        }
                        break;

                    // Case 1 receives the data portion of the packet.  Read in "_recLen" number
                    // of bytes and store them in the _data array.
                    case 1:
                        _data[_recPos++] = inch;
                        _recCalcCS += inch;
                        if (_recPos == _recLen)
                        {
                            _recPhase = 2;
                        }
                        break;

                    // After the data comes a single ETX character.  Do we have it?  If not,
                    // reset the state machine to default and start looking for a new header.
                    case 2:
                        // Packet properly terminated?
                        if (inch == ETX)
                        {
                            _recPhase = 3;
                        }
                        else
                        {
                            serialReset();
                        }
                        break;

                    // Next comes the checksum.  We have already calculated it from the incoming
                    // data, so just store the incoming checksum byte for later.
                    case 3:
                        _recCS = inch;
                        _recPhase = 4;
                        break;

                    // The final state - check the last character is EOT and that the checksum matches.
                    // If that test passes, then look for a valid command callback to execute.
                    // Execute it if found.
                    case 4:
                        if (inch == EOT)
                        {
                            if (_recCS == _recCalcCS)
                            {
                                bool noAck = noAckToBool(_recNoAck);
                                Server.TransportSend(this, _recSender, _recStation, _data, _recLen, noAck);
                            }
                        }
                        //Clear the data
                        serialReset();
                        //Return true, we have processed one command
                        return true;
                }
            }
            return true;
        }

        private bool noAckToBool(byte noAck)
        {
            return noAck == 0 ? false : true;
        }

        private byte noAckToByte(bool noACK)
        {
            return (byte)(noACK ? 1 : 0);
        }

        //Reset the state machine and release the data pointer
        private void serialReset()
        {
            _recPhase = 0;

            _recPos = 0;
            _recLen = 0;
            _recCommand = 0;
            _recNoAck = 0;

            _recCS = 0;
            _recCalcCS = 0;
        }
    }
}
