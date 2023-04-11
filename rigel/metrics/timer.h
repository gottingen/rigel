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

#include <chrono>

#include "rigel/metrics/stopwatch.h"

namespace rigel {

class Timer {
 public:
  virtual ~Timer() = default;

  // Record emits the provided duration of time as a timer metric.
  virtual void Record(std::chrono::nanoseconds) = 0;

  // Record emits the provided value as a timer metric.
  virtual void Record(int64_t) = 0;

  // Start returns a new Stopwatch from the current instant.
  virtual Stopwatch Start() = 0;
};

}  // namespace rigel
