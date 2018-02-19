using System;
using System.Collections;
using System.Collections.Generic;
using System.Linq;
using System.Net;
using System.Net.Sockets;
using System.Text;
using System.Threading;

namespace VirtualHardwareNet
{
    public class EthernetUdpNet : IDisposable
    {
        // see:
        // https://stackoverflow.com/questions/19786668/c-sharp-udp-socket-client-and-server

        private struct ReceiveResult
        {
            public byte[] Buffer { get; }
            public IPEndPoint RemoteEndPoint { get; }
            public ReceiveResult(byte[] buffer, IPEndPoint remoteEndPoint)
            {
                Buffer = buffer;
                RemoteEndPoint = remoteEndPoint;
            }
        }

        private UdpClient _client;
        private Queue<ReceiveResult> _receivedPackets = new Queue<ReceiveResult>();
        private ByteQueue _receiveQueue = new ByteQueue();
        private ByteQueue _sendQueue = new ByteQueue();
        private IPEndPoint _packetEndpoint;

        private readonly ManualResetEvent _mreHandleClient = new ManualResetEvent(false);
                private volatile bool _treadActive;

        public string ErrorMessage { get; private set; }
        public string SocketErrorCode { get; private set; }
        public int ErrorCode { get; private set; }

        private volatile bool _allocated;
        public bool Allocated
        {
            get { return _allocated; }
            set { _allocated = value; }
        }

        /// <summary>
        /// EthernetClient constructor.
        /// </summary>
        public EthernetUdpNet()
        {
        }


        public int begin(ushort port)
        {
            close();

            try
            {
                _client = new UdpClient(port);
                _client.DontFragment = true;
            }
            catch (Exception e)
            {
                _client = null;
                setExceptionMessage(e);
                return 0;
            }

            _treadActive = true;
            var clientThread = new Thread(new ParameterizedThreadStart(handleClient));
            clientThread.Start(_client);
            return 1;

        }

        private bool tryGetHostAddress(string hostname, out IPAddress ipAddress)
        {
            IPAddress[] ipAaddresses = Dns.GetHostAddresses(hostname);

            foreach (var item in ipAaddresses)
            {
                // only use IP version 4 address
                if (item.AddressFamily == AddressFamily.InterNetwork)
                {
                    ipAddress = item;
                    return true;
                }
            }

            ipAddress = IPAddress.None;
            return false;
        }

        /// <summary>
        /// Initiate a connection with host:port.
        /// </summary>
        /// <param name="host">name to resolve or a stringified dotted IP address.</param>
        /// <param name="port">to connect to.</param>
        /// <returns>1 if SUCCESS or 0 if FAILURE</returns>
        public int beginPacket(string hostname, ushort port)
        {
            IPAddress ipAddress;
            if (!IPAddress.TryParse(hostname, out ipAddress))
            {
                if (!tryGetHostAddress(hostname, out ipAddress))
                {
                    return 0;
                }
            }
            _packetEndpoint = new IPEndPoint(ipAddress, port);
            //_client.Connect(_packetEndpoint);
            return 1;
        }

        public int endPacket()
        {
            byte[] buffer = new byte[_sendQueue.Length];
            _sendQueue.Dequeue(buffer, 0, _sendQueue.Length);
            return _client.Send(buffer, buffer.Length, _packetEndpoint);
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
                SocketErrorCode = "SocketError";
            }
        }

        private void handleClient(object obj)
        {
            try
            {
                var client = obj as UdpClient;
                int count;
                while (_treadActive)
                {
                    if (client.Available > 0)
                    {
                        // int length = buffer.Length - _receiveQueue.Length;

                        IPEndPoint remoteEP = new IPEndPoint(IPAddress.Any, 0);
                        byte[] buffer = client.Receive(ref remoteEP);
                        count = buffer.Length;
                        if (count > 0)
                        {
                            ReceiveResult result = new ReceiveResult(buffer, remoteEP);
                            lock (_receivedPackets)
                            {
                                _receivedPackets.Enqueue(result); 
                            }
                        }
                    }

                    // prevent high CPU usage
                    Thread.Sleep(10);
                }
            }
            catch (Exception e)
            {
                setExceptionMessage(e);
            }
            finally
            {
                _mreHandleClient.Set();
            }
        }

        public int parsePacket(out uint remoteIpAddress, out ushort remotePort)
        {
            Thread.Yield();

            int packetLength = 0;
            remoteIpAddress = 0;
            remotePort = 0;
            ReceiveResult result = new ReceiveResult();

            lock (_receivedPackets)
            {
                if (_receivedPackets.Count > 0)
                {
                    result = _receivedPackets.Dequeue();
                }
                else
                {
                    return packetLength;
                }
            }

            if (result.RemoteEndPoint != null)
            {
                var ep = result.RemoteEndPoint;
                var bytes = ep.Address.GetAddressBytes();
                remoteIpAddress = BitConverter.ToUInt32(bytes, 0);
                remotePort = (ushort)ep.Port;
            }
            if (result.Buffer != null)
            {
                packetLength = result.Buffer.Length;
            }

            _receiveQueue.Clear();
            _receiveQueue.Enqueue(result.Buffer, 0, packetLength);

            return packetLength;
        }

        /// <summary>
        /// Write at most 'size' bytes.
        /// </summary>
        /// <param name="buf">Buffer to read from.</param>
        /// <param name="size">of the buffer.</param>
        /// <returns>0 if FAILURE or the number of bytes sent.</returns>
        public uint write(byte[] buf, uint size)
        {
            if (size == 0)
                return 0;

            int length = 128 - _sendQueue.Length;
            if (size > length)
            {
                size = (uint)length;
            }
            _sendQueue.Enqueue(buf, 0, (int)size);

            Thread.Yield();

            return size;
        }

        /// <summary>
        /// Read a number of bytes and store in a buffer.
        /// </summary>
        /// <param name="buf">buffer to write to.</param>
        /// <param name="bytes">number of bytes to read.</param>
        /// <returns>-1 if no data or number of read bytes.</returns>
        public int read(ref byte[] buf, uint bytes)
        {
            Thread.Yield();

            int result = -1;
            int length = _receiveQueue.Length;
            length = bytes > length ? length : (int)bytes;
            if (length > 0)
            {
                result = _receiveQueue.Dequeue(buf, 0, length);
            }

            return result;
        }

        /// <summary>
        /// Returns the next byte of the read queue without removing it from the queue.
        /// </summary>
        /// <returns>-1 if no data, else the first byte of incoming data available.</returns>
        public int peek()
        {
            Thread.Yield();

            int result = -1;
            if (_receiveQueue.Length > 0)
            {
                result = _receiveQueue.Peek();
            }

            return result;
        }

        /// <summary>
        /// Close the connection.
        /// </summary>
        public void close()
        {
            if (_client != null)
            {
                // signal and wait for thread handleClient() has finished
                _treadActive = false;
                _mreHandleClient.WaitOne();

                _client.Close();
                _client = null;

                _receivedPackets.Clear();
                _receiveQueue.Clear();
                _sendQueue.Clear();

                Allocated = false;
            }
        }

        #region IDisposable Support

        private bool disposedValue = false;

        protected virtual void Dispose(bool disposing)
        {
            if (!disposedValue)
            {
                if (disposing)
                {
                    _client.Close();
                    _client = null;
                }
                disposedValue = true;
            }
        }

        public void Dispose()
        {
            Dispose(true);
        }

        #endregion IDisposable Support
    }
}
