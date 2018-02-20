using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading;

namespace MyEtherTcpServer
{
    class Program
    {
        static void Main(string[] args)
        {
            Console.WriteLine("My ETHER TCP Server - ver. 1.0");

            EtherServer server = new EtherServer(5445);

            Console.WriteLine("Server listening...");

            while (true)
            {
                server.ProcessClients();

                // prevent high CPU usage
                Thread.Sleep(10);
            }
        }
    }
}
