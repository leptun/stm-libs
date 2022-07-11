#pragma once
#include <inttypes.h>
#include <Arduino.h>

template <class T>
class Timer {
public:
  Timer(bool autoStart = false) {
	  if (autoStart)
	    start();
	  else
	    stop();
  }
  void start() {
	  _start_time = millis();
	  _isRunning = true;
  }
  void stop() { _isRunning = false; }
  bool running() const { return _isRunning; }
  bool expired(T msPeriod) {
	  if (!_isRunning)
	    return false;
	  bool expired = false;
	  const T now = millis();
	  if (_start_time <=  _start_time + msPeriod) {
	    if ((now >= _start_time + msPeriod) || (now < _start_time)) {
	      expired = true;
	    }
	  }
	  else {
	    if ((now >= _start_time + msPeriod) && (now < _start_time)) {
	      expired = true;
	    }
	  }
	  if (expired) _isRunning = false;
	  return expired;
  }
  T elapsed() {
	  return _isRunning ? (millis() - _start_time) : 0;
  }
private:
  bool _isRunning;
  T _start_time;
};


template class Timer<uint32_t>;
using LongTimer = Timer<uint32_t>; //max 4294967295ms

template class Timer<uint16_t>;
using ShortTimer = Timer<uint16_t>; //max 65535ms
