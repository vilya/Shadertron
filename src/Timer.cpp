// Copyright 2019 Vilya Harvey
#include "Timer.h"

namespace vh {

  Timer::Timer(bool autostart) :
    Timer()
  {
    _ref = std::chrono::high_resolution_clock::now();
    if (autostart) {
      start();
    }
  }


  void Timer::start()
  {
    _start = now();
    _stop = _start;
    _running = true;
  }


  void Timer::stop()
  {
    _stop = now();
    _running = false;
  }


  void Timer::resume()
  {
    if (!_running) {
      double elapsed = _stop - _start;
      _start = now() - elapsed;
      _stop = _start;
      _running = true;
    }
  }


  void Timer::adjustTimeSecs(double secs)
  {
    adjustTimeMS(secs * 1000.0);
  }


  void Timer::adjustTimeMS(double ms)
  {
    _start -= ms;

    double cutoff = (_running ? now() : _stop);
    if (_start > cutoff) {
      _start = cutoff;
    }

    if (_stop < _start) {
      // This happens when the timer is running and we're rewinding.
      _stop = _start;
    }
  }


  void Timer::adjustTimeUS(double us)
  {
    adjustTimeMS(us / 1000.0);
  }


  bool Timer::running() const
  {
    return _running;
  }


  double Timer::elapsedSecs() const
  {
    return elapsedMS() / 1000.0;
  }


  double Timer::elapsedMS() const
  {
    return (_running ? now() : _stop) - _start;
  }


  double Timer::elapsedUS() const
  {
    return elapsedMS() * 1000.0;
  }


  //
  // Timer private methods
  //

  double Timer::now() const
  {
    std::chrono::duration<double, std::milli> ms = std::chrono::high_resolution_clock::now() - _ref;
    return ms.count();
  }

} // namespace vh
