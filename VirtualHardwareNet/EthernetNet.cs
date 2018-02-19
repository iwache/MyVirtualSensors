using System;
using System.Collections.Generic;
using System.Linq;
using System.Net.Sockets;
using System.Text;

namespace VirtualHardwareNet
{
    public class EthernetNet
    {
        public static readonly int MAX_UDP_CLIENTS = 8;
        public static readonly int MAX_CLIENTS = 8;
        public static readonly int MAX_SERVERS = 2;

        public string ErrorMessage { get; private set; }
        public string SocketErrorCode { get; private set; }
        public int ErrorCode { get; private set; }

        private EthernetUdpNet[] _udpClients = new EthernetUdpNet[MAX_UDP_CLIENTS];
        private EthernetClientNet[] _clients = new EthernetClientNet[MAX_CLIENTS];
        private EthernetServerNet[] _servers = new EthernetServerNet[MAX_SERVERS];

        public EthernetUdpNet NewUdpClient(ref int socketNumber)
        {
            lock (_udpClients)
            {
                for (int i = 0; i < _udpClients.Length; i++)
                {
                    if (_udpClients[i] == null || !_udpClients[i].Allocated)
                    {
                        if (_udpClients[i] == null)
                            _udpClients[i] = new EthernetUdpNet();

                        _udpClients[i].Allocated = true;
                        socketNumber = i;
                        return _udpClients[i];
                    }
                }
            }

            socketNumber = -1;
            return null;
        }

        private bool tryGetUdpClient(int socketNumber, out EthernetUdpNet updClient)
        {
            updClient = null;
            if (socketNumber < 0 || socketNumber >= MAX_UDP_CLIENTS)
                return false;

            updClient = _udpClients[socketNumber];
            if (updClient == null)
                return false;

            return true;
        }

        public EthernetClientNet NewClient(ref int socketNumber)
        {
            lock (_clients)
            {
                for (int i = 0; i < _clients.Length; i++)
                {
                    if (_clients[i] == null || !_clients[i].Allocated)
                    {
                        if (_clients[i] == null)
                            _clients[i] = new EthernetClientNet();

                        _clients[i].Allocated = true;
                        socketNumber = i;
                        return _clients[i];
                    }
                }
            }

            socketNumber = -1;
            return null;
        }

        private bool tryGetClient(int socketNumber, out EthernetClientNet client)
        {
            client = null;
            if (socketNumber < 0 || socketNumber >= MAX_CLIENTS)
                return false;

            client = _clients[socketNumber];
            if (client == null)
                return false;

            return true;
        }

        private EthernetServerNet newServer(ref int socketNumber)
        {
            for (int i = 0; i < _servers.Length; i++)
            {
                if (_servers[i] == null)
                {
                    _servers[i] = new EthernetServerNet(this);
                    socketNumber = i;
                    return _servers[i];
                }
            }
            socketNumber = -1;
            return null;
        }

        private bool tryGetServer(int socketNumber, out EthernetServerNet server)
        {
            server = null;
            if (socketNumber < 0 || socketNumber >= MAX_CLIENTS)
                return false;

            server = _servers[socketNumber];
            if (server == null)
                return false;

            return true;
        }
        public EthernetNet()
        {
        }

        public string clientErrorMessage(int socketNumber)
        {
            if (socketNumber == -1)
                return ErrorMessage;

            EthernetClientNet client;
            if (tryGetClient(socketNumber, out client))
                return client.ErrorMessage;

            return null;
        }

        public string clientSocketErrorCode(int socketNumber)
        {
            if (socketNumber == -1)
                return SocketErrorCode;

            EthernetClientNet client;
            if (tryGetClient(socketNumber, out client))
                return client.SocketErrorCode;

            return null;
        }

        public int clientErrorCode(int socketNumber)
        {
            if (socketNumber == -1)
                return ErrorCode;

            EthernetClientNet client;
            if (tryGetClient(socketNumber, out client))
                return client.ErrorCode;

            return -1;
        }

        public int clientConnect(string hostname, ushort port, ref int socketNumber)
        {
            EthernetClientNet client = NewClient(ref socketNumber);
            if (client != null)
            {
                return client.connect(hostname, port);
            }

            // all TcpClient sockets used
            setExceptionMessage(new SocketException(10024));
            return -1;
        }

        public int clientAvailable(int socketNumber)
        {
            EthernetClientNet client;
            if (tryGetClient(socketNumber, out client))
                return client.available();

            return 0;
        }

        public byte clientConnected(int socketNumber)
        {
            EthernetClientNet client;
            if (tryGetClient(socketNumber, out client))
                return client.connected();

            return 0;
        }

        public int clientPeek(int socketNumber)
        {
            EthernetClientNet client;
            if (tryGetClient(socketNumber, out client))
                return client.peek();

            return -1;
        }

        public void clientFlush(int socketNumber)
        {
            EthernetClientNet client;
            if (tryGetClient(socketNumber, out client))
                client.flush();
        }

        public void clientStop(int socketNumber)
        {
            EthernetClientNet client;
            if (tryGetClient(socketNumber, out client))
                client.stop();
        }

        public byte clientStatus(int socketNumber)
        {
            EthernetClientNet client;
            if (tryGetClient(socketNumber, out client))
                return client.status();

            return 0x00; // ETHERNETCLIENT_W5100_CLOSED
        }

        public void clientClose(int socketNumber)
        {
            EthernetClientNet client;
            if (tryGetClient(socketNumber, out client))
                client.close();
        }

        public ushort clientWrite(int socketNumber, byte[] buf, ushort size)
        {
            EthernetClientNet client;
            if (tryGetClient(socketNumber, out client))
                return (ushort)client.write(buf, size);

            return 0;
        }

        public int clientRead(int socketNumber, byte[] buf, ushort bytes)
        {
            EthernetClientNet client;
            if (tryGetClient(socketNumber, out client))
                return client.read(ref buf, bytes);

            return -1;
        }

        public string clientRemoteIpAddress(int socketNumber)
        {
            EthernetClientNet client;
            if (tryGetClient(socketNumber, out client))
                return client.remoteIpAddress();

            return null;
        }

        public ushort clientRemotePort(int socketNumber)
        {
            EthernetClientNet client;
            if (tryGetClient(socketNumber, out client))
                return client.remotePort();

            return 0;
        }

        public string serverErrorMessage(int socketNumber)
        {
            EthernetServerNet server;
            if (tryGetServer(socketNumber, out server))
                return server.ErrorMessage;

            return null;
        }

        public string serverSocketErrorCode(int socketNumber)
        {
            EthernetServerNet server;
            if (tryGetServer(socketNumber, out server))
                return server.SocketErrorCode;

            return null;
        }

        public int serverErrorCode(int socketNumber)
        {
            EthernetServerNet server;
            if (tryGetServer(socketNumber, out server))
                return server.ErrorCode;

            return -1;
        }

        public int serverBegin(string ipAddress, ushort port, ref int socketNumber)
        {
            EthernetServerNet server = newServer(ref socketNumber);
            if (server != null)
            {
                return server.begin(ipAddress, port);
            }

            // all TcpClient sockets used
            setExceptionMessage(new SocketException(10024));
            return -1;
        }

        public int serverAccept(int socketNumber)
        {
            EthernetServerNet server;
            if (tryGetServer(socketNumber, out server))
                return server.accept();

            return -1;
        }

        private void setExceptionMessage(Exception e)
        {
            ErrorMessage = e.Message;
            if (e is SocketException)
            {
                var socketException = e as SocketException;
                ErrorCode = socketException.ErrorCode;
                SocketErrorCode = socketException.SocketErrorCode.ToString();
            }
            else
            {
                ErrorCode = -1;
                SocketErrorCode = "Unknown Ethernet .Net Error";
            }
        }
        //----------------------

        public int udpBegin(ushort port, ref int socketNumber)
        {
            EthernetUdpNet client = NewUdpClient(ref socketNumber);
            if (client != null)
            {
                return client.begin(port);
            }

            // all UdpClient sockets used
            setExceptionMessage(new SocketException(10024));
            return -1;
        }

        public int udpBeginPacket(int socketNumber, string ipStr, ushort port)
        {
            EthernetUdpNet client;
            if (tryGetUdpClient(socketNumber, out client))
                return client.beginPacket(ipStr, port);
            return -1;
        }

        public int udpEndPacket(int socketNumber)
        {
            EthernetUdpNet client;
            if (tryGetUdpClient(socketNumber, out client))
                return client.endPacket();
            return -1;
        }

        public int udpParsePacket(int socketNumber, out uint remoteIpAddress, out ushort remotePort)
        {
            remoteIpAddress = 0;
            remotePort = 0;

            EthernetUdpNet client;
            if (tryGetUdpClient(socketNumber, out client))
                return client.parsePacket(out remoteIpAddress, out remotePort);
            return -1;
        }

        public uint udpWrite(int socketNumber, byte[] buf, uint size)
        {
            EthernetUdpNet client;
            if (tryGetUdpClient(socketNumber, out client))
                return client.write(buf, size);
            return 0;
        }

        public int udpRead(int socketNumber, ref byte[] buf, uint bytes)
        {
            EthernetUdpNet client;
            if (tryGetUdpClient(socketNumber, out client))
                return client.read(ref buf, bytes);
            return -1;
        }

        public int udpPeek(int socketNumber)
        {
            EthernetUdpNet client;
            if (tryGetUdpClient(socketNumber, out client))
                return client.peek();
            return -1;
        }

        public void udpClose(int socketNumber)
        {
            EthernetUdpNet client;
            if (tryGetUdpClient(socketNumber, out client))
                client.close();
        }

        //----------------------

    }
}
