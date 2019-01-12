// Copyright 2019 Vilya Harvey
#include "FPSCounter.h"

namespace vh {

  FPSCounter::FPSCounter(double initialTimeMS)
  {
    for (int i = 0; i < kFrames; i++) {
      _frames[i] = initialTimeMS;
    }
  }


  void FPSCounter::newFrame(double currentTimeMS)
  {
    _i = nextFrame();
    _frames[_i] = currentTimeMS;
  }


  double FPSCounter::msPerFrame() const
  {
    double totalTime = _frames[_i] - _frames[nextFrame()];
    return totalTime / double(kFrames);
  }


  double FPSCounter::framesPerSec() const
  {
    double totalTime = _frames[_i] - _frames[nextFrame()];
    return double(kFrames) * 1000.0 / totalTime;
  }

} // namespace vh
