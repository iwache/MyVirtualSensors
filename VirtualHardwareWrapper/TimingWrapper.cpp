// TimingWrapper.cpp

#include <msclr\auto_gcroot.h>
#include "TimingWrapper.h"

using namespace System::Runtime::InteropServices; // Marshal
using namespace VirtualHardwareNet;


class TimingWrapperPrivate
{
	public: msclr::auto_gcroot<Timing^> timing;
};

TimingWrapper::TimingWrapper()
{
	_private = new TimingWrapperPrivate();
	_private->timing = gcnew Timing();
}

TimingWrapper::~TimingWrapper()
{
	delete _private;
}

unsigned int TimingWrapper::millis()
{
	return _private->timing->Millis();
}

unsigned int TimingWrapper::micros()
{
	return _private->timing->Micros();
}
