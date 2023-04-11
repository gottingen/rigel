// Copyright 2023 The Rigel Authors
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     https://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//

#pragma once

#include <memory>

#include "rigel/metrics/stopwatch_recorder.h"

namespace rigel {

class Stopwatch {
 public:
  Stopwatch(std::chrono::steady_clock::time_point,
            std::shared_ptr<StopwatchRecorder> recorder);

  // Stop stops the Stopwatch and records the duration of time it observed.
  void Stop();

 private:
  const std::chrono::steady_clock::time_point start_;
  std::shared_ptr<StopwatchRecorder> recorder_;
};

}  // namespace rigel
