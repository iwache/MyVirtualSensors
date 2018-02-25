// TimingWrapper.h

#pragma once

//using namespace System;

class TimingWrapperPrivate;

class __declspec(dllexport) TimingWrapper
{
private: TimingWrapperPrivate* _private;

public: TimingWrapper();

public: ~TimingWrapper();

public: unsigned int millis();

public: unsigned int micros();
};
