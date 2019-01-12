// Copyright 2019 Vilya Harvey
#ifndef VH_FPSCOUNTER_H
#define VH_FPSCOUNTER_H

#include <cstdint>

namespace vh {

  class FPSCounter
  {
  public:
    FPSCounter(double initialTimeMS=0.0);

    void newFrame(double currentTimeMS);

    double msPerFrame() const;
    double framesPerSec() const;

  private:
    static constexpr uint8_t kFrames    = 64;
    static constexpr uint8_t kFrameMask = kFrames - 1;

    inline uint8_t nextFrame() const { return (_i + 1) & kFrameMask; }

    double _frames[kFrames];
    uint8_t _i = 0;
  };

} // namespace vh

#endif // VH_FPSCOUNTER_H
