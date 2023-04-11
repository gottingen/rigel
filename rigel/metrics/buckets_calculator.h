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

#include <cstdint>

namespace rigel {

class BucketsCalculator {
 public:
  enum class Growth {
    Linear,
    Exponential,
  };

  BucketsCalculator(Growth growth, double start, double update);

  double Calculate(uint64_t index) const;

  bool operator==(BucketsCalculator other) const;

  bool operator!=(BucketsCalculator other) const;

 private:
  const Growth growth_;
  const double start_;
  const double update_;
};

}  // namespace rigel
