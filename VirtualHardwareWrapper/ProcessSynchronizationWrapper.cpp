// ProcessSynchronizationWrapper.cpp

#include <msclr\auto_gcroot.h>
#include "ProcessSynchronizationWrapper.h"

using namespace System::Runtime::InteropServices; // Marshal
using namespace VirtualHardwareNet;


class ProcessSynchronizationWrapperPrivate
{
	public: msclr::auto_gcroot<ProcessSynchronization^> procSync;
};

ProcessSynchronizationWrapper::ProcessSynchronizationWrapper()
{
	_private = new ProcessSynchronizationWrapperPrivate();
	_private->procSync = gcnew ProcessSynchronization();
}

ProcessSynchronizationWrapper::~ProcessSynchronizationWrapper()
{
	delete _private;
}

unsigned int ProcessSynchronizationWrapper::wait(unsigned int milliseconds)
{
	return _private->procSync->Wait(milliseconds);
}

unsigned int ProcessSynchronizationWrapper::millis()
{
	return _private->procSync->Millis();
}
