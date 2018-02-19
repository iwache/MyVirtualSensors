// ProcessSynchronizationWrapper.h

#pragma once

//using namespace System;

class ProcessSynchronizationWrapperPrivate;

class __declspec(dllexport) ProcessSynchronizationWrapper
{
private: ProcessSynchronizationWrapperPrivate* _private;

public: ProcessSynchronizationWrapper();

public: ~ProcessSynchronizationWrapper();

public: unsigned int wait(unsigned int milliseconds);

public: unsigned int millis();
};
