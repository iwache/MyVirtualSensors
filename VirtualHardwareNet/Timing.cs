using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading;
using System.Threading.Tasks;

namespace VirtualHardwareNet
{
    public class Timing
    {
        private long _lastMicrosTicks;
        private long _lastMillisTicks;
        private uint _lastMillis;
        private uint _lastMicros;


        public Timing()
        {
            _lastMillisTicks = DateTime.UtcNow.Ticks;
            _lastMicrosTicks = DateTime.UtcNow.Ticks;
        }

        public uint Millis()
        {
            long currentTicks = DateTime.UtcNow.Ticks;
            long elapsedTicks = currentTicks - _lastMillisTicks;

            _lastMillis += (uint)(elapsedTicks / 10000);

            _lastMillisTicks = DateTime.UtcNow.Ticks;

            return _lastMillis;
        }

        public uint Micros()
        {
            long currentTicks = DateTime.UtcNow.Ticks;
            long elapsedTicks = currentTicks - _lastMicrosTicks;

            _lastMicros += (uint)(elapsedTicks / 10);

            _lastMicrosTicks = DateTime.UtcNow.Ticks;

            return _lastMicros;
        }
    }
}
