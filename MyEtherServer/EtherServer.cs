using System;
using System.Collections.Generic;
using System.Linq;
using System.Net;
using System.Net.Sockets;
using System.Text;
using System.Threading;

namespace MyEtherServer
{
    public class EtherServer
    {
        private const byte BROADCAST_ADDRESS = 255;

        private TcpListener _tcpListener;
        private Thread _listenThread;
        private Dictionary<int, EtherClient> _etherClients = new Dictionary<int, EtherClient>();
        private Dictionary<int, EtherClient> _etherNodes = new Dictionary<int, EtherClient>();

//        private bool _serverFinished;


        public EtherServer(ushort port)
        {
            StartListener(port);
        }

        private void StartListener(ushort port)
        {
            _tcpListener = new TcpListener(IPAddress.Any, port);
            _listenThread = new Thread(new ThreadStart(listenForClients));
            _listenThread.Start();
        }

        public void ProcessClients()
        {
            List<int> disconnectedClients = new List<int>();

            lock (_etherClients)
            {
                foreach (var item in _etherClients)
                {
                    EtherClient client = item.Value;
                    if (client != null)
                    {
                        if (client.connected() == 1)
                        {
                            client.Process();
                        }
                        else
                        {
                            // add disconnect client to list of disconnected client
                            int clientKey = item.Key;
                            disconnectedClients.Add(clientKey);
                        }
                    }
                }

                foreach (var key in disconnectedClients)
                {
                    EtherClient client = _etherClients[key];
                    _etherClients.Remove(key);

                    int address = -1;
                    if (client != null)
                        address = client.Address;
                    Console.WriteLine("Client disconnected: {0}; node address: {1}", key, address);

                    if (_etherNodes.ContainsValue(client))
                    {
                        // Get Dictionary key from value
                        // see: https://stackoverflow.com/questions/2444033/get-dictionary-key-by-value
                        var nodeItem = _etherNodes.FirstOrDefault(item => item.Value == client);
                        // Check if nodeIdem is Default and not First
                        if (nodeItem.Key != 0 || nodeItem.Value != null)
                            _etherNodes.Remove(nodeItem.Key);
                    }
                    client.close();
                }
            }
        }

        public void AddEtherNode(EtherClient client)
        {
            int address = client.Address;
            if (address == BROADCAST_ADDRESS)
                return;

            if (!_etherClients.ContainsValue(client))
            {
                Console.WriteLine("ERROR Unable to assign node address {0} to unknown client.", address);
                return;
            }

            var clientKey = _etherClients.FirstOrDefault(item => item.Value == client).Key;

            if (_etherNodes.ContainsKey(address))
            {
                var otherClient = _etherNodes[address];
                var otherKey = _etherClients.FirstOrDefault(item => item.Value == otherClient).Key;
                Console.WriteLine("ERROR with node address assigning to Client {0}: other Client {1} is already assigned with address {2}.", clientKey, otherKey, address);
                return;
            }

            _etherNodes.Add(client.Address, client);
            Console.WriteLine("Client {0} node address assigned: {1}", clientKey, address);
        }

        private void listenForClients()
        {
            try
            {
                _tcpListener.Start();

                while (true)
                {
                    TcpClient client = null;
                    
                    try
                    {
                        //blocks until a client has connected to the server
                        client = _tcpListener.AcceptTcpClient();

                        var etherClient = new EtherClient(this);
                        etherClient.connect(client);

                        var key = _etherClients.Count;

                        // Console.WriteLine() is this thread safe
                        // see: https://stackoverflow.com/questions/1079980/calling-console-writeline-from-multiple-threads
                        Console.WriteLine("Client connected: {0}", key);

                        lock (_etherClients)
                            _etherClients.Add(key, etherClient);
                    }
                    catch
                    {
                        return;
                    }
                }
            }
            finally
            {
//                _serverFinished = true;
            }
        }

        public void TransportSend(EtherClient etherClient, byte source, byte destination, byte[] data, byte len, bool noACK)
        {
            Console.Write("Packet: [{0}-{1}-{2}-{3}-{4}-{5}-{6}] len:{7} noACK:{8} ", data[0], data[1], data[2], data[3], data[4], data[5], data[6], len, noACK);

            bool sent = false;

            if (destination == BROADCAST_ADDRESS)
            {
                lock (_etherClients)
                {
                    foreach (var item in _etherClients)
                    {
                        if (item.Value == null)
                            continue;

                        // Do not send back broadcast to sender
                        if (item.Value == etherClient)
                            continue;

                        sent = true;
                        EtherClient client = item.Value;
                        client.TransportSend(source, destination, data, len, noACK);
                    }
                }
            }
            else
            {
                if (_etherNodes.ContainsKey(destination))
                {
                    sent = true;
                    EtherClient client = _etherNodes[destination];
                    client.TransportSend(source, destination, data, len, noACK);
                }
            }

            Console.WriteLine(sent ? "OK" : "NOACK");

        }
    }
}
