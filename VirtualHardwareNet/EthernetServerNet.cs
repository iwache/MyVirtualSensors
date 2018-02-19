using System;
using System.Collections.Generic;
using System.Linq;
using System.Net;
using System.Net.Sockets;
using System.Text;
using System.Threading;

namespace VirtualHardwareNet
{
    public class EthernetServerNet
    {
        private TcpListener _tcpListener;
        private Thread _listenThread;
        private Queue<int> _newSocketNumbers = new Queue<int>();
        private EthernetNet _ethernet;

        public string ErrorMessage { get; private set; }
        public string SocketErrorCode { get; private set; }
        public int ErrorCode { get; private set; }

        public EthernetServerNet(EthernetNet ethernet)
        {
            _ethernet = ethernet;
        }

        public int begin(string ipAddress, ushort port)
        {
            IPAddress address;
            if (!IPAddress.TryParse(ipAddress, out address))
                return -1;

            try
            {
                _tcpListener = new TcpListener(address, port);
            }
            catch (Exception e)
            {
                setExceptionMessage(e);
                return -1;
            }
            _listenThread = new Thread(new ThreadStart(listenForClients));
            _listenThread.Start();

            return 1;
        }

        public int accept()
        {
            lock (_newSocketNumbers)
            {
                if (_newSocketNumbers.Count > 0)
                    return _newSocketNumbers.Dequeue();

            }
            return -1;
        }

        //public void close(int socketNumber)
        //{
        //    var client = TcpClientsList.Client(socketNumber);
        //    if (client != null)
        //        client.Close();
        //}

        //public string getRemoteIpAddress(int socketNumber)
        //{
        //    var client = TcpClientsList.Client(socketNumber);
        //    if (client == null)
        //        return null;

        //    // see: https://stackoverflow.com/questions/2717381/how-do-i-get-client-ip-address-using-tcpclient
        //    var endPoint = client.Client.RemoteEndPoint as IPEndPoint;
        //    if (endPoint == null)
        //        return null;

        //    return endPoint.Address.ToString();
        //}

        //public ushort getRemotePort(int socketNumber)
        //{
        //    var client = TcpClientsList.Client(socketNumber);
        //    if (client == null)
        //        return 0;

        //    var endPoint = client.Client.RemoteEndPoint as IPEndPoint;
        //    if (endPoint == null)
        //        return 0;

        //    return (ushort)endPoint.Port;
        //}

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
                SocketErrorCode = "SocketError";
            }
        }

        private void listenForClients()
        {
            try
            {
                _tcpListener.Start();

                while (true)
                {
                    TcpClient tcpClient = null;

                    try
                    {
                        //blocks until a client has connected to the server
                        tcpClient = _tcpListener.AcceptTcpClient();

                        int socketNumber = -1;
                        EthernetClientNet ethernetClient = _ethernet.NewClient(ref socketNumber);
                        if (ethernetClient != null)
                        {
                            ethernetClient.connect(tcpClient);

                            lock (_newSocketNumbers)
                                _newSocketNumbers.Enqueue(socketNumber);
                        }
                        else
                        {
                            // all EternetClientNet sockets used, close new TCP client
                            tcpClient.Close();
                        }
                    }
                    catch
                    {
                        return;
                    }
                }
            }
            finally
            {
                // nothing to do for now
            }
        }
    }
}
