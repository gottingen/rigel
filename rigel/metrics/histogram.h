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

class Histogram {
 public:
  virtual ~Histogram() = default;

  // Record the given value.
  virtual void Record(double) noexcept = 0;

  // Record the given duration.
  virtual void Record(std::chrono::nanoseconds) noexcept = 0;

  // Return a stopwatch which can be used to time an event and record its
  // duration.
  virtual Stopwatch Start() noexcept = 0;
};

}  // namespace rigel
