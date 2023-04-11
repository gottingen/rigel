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
#include <vector>

#include "rigel/metrics/buckets_iterator.h"
#include "rigel/metrics/buckets_calculator.h"

namespace rigel {

class Buckets {
 public:
  // LinearValues constructs a linear sequence of `num` Value buckets beginning
  // at `start` and incrementing by `width` each time.
  static Buckets LinearValues(double start, double width, uint64_t num);

  // ExponentialValues constructs an expoential sequence of `num` Value buckets
  // beginning at `start` and multiplying by `factor` each time.
  static Buckets ExponentialValues(double start, double factor, uint64_t num);

  // LinearDurations constructs a linear sequence of `num` Duration buckets
  // beginning at `start` and incrementing by `width` each time.
  static Buckets LinearDurations(std::chrono::nanoseconds start,
                                 std::chrono::nanoseconds width, uint64_t num);

  // ExponentialDurations constructs an expoential sequence of `num` Duration
  // buckets beginning at `start` and multiplying by `factor` each time.
  static Buckets ExponentialDurations(std::chrono::nanoseconds start,
                                      uint64_t factor, uint64_t num);

  // Kind is an enum representing the type of a sequence of buckets.
  enum class Kind {
    Values,
    Durations,
  };

  BucketsIterator begin() const;

  BucketsIterator end() const;

  uint64_t size() const { return num_; }

  Kind kind() const { return kind_; }

 private:
  Buckets(Kind kind, BucketsCalculator calculator, uint64_t num);

  const Kind kind_;
  const BucketsCalculator calculator_;
  const double num_;
};

}  // namespace rigel
