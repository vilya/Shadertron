// Copyright 2019 Vilya Harvey
#ifndef VH_TIMER_H
#define VH_TIMER_H

#include <chrono>

namespace vh {

  class Timer {
  public:
    Timer() = default;
    explicit Timer(bool autostart);

    void start();
    void stop();
    void resume();

    void adjustTimeSecs(double secs);
    void adjustTimeMS(double ms);
    void adjustTimeUS(double us);

    bool running() const;

    double elapsedSecs() const;
    double elapsedMS() const;
    double elapsedUS() const;

  private:
    double now() const;

  private:
    /// Whenever we deal with a point in time in this class, the value will be
    /// relative to the point in time at which this timer was created. This
    /// helps avoid rounding errors as we get further away from the standard
    /// epoch.
    std::chrono::high_resolution_clock::time_point _ref;

    /// Start and stop times are the number of milliseconds since the timer was
    /// created.
    double _start, _stop;

    bool _running = false;
  };

} // namespace vh

#endif // VH_TIMER_H
