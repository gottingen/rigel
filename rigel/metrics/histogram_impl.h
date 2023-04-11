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

#include <algorithm>
#include <string>
#include <unordered_map>
#include <vector>

#include "rigel/metrics/buckets.h"
#include "rigel/metrics/counter.h"
#include "rigel/metrics/histogram.h"
#include "rigel/metrics/counter_impl.h"
#include "rigel/metrics/histogram_bucket.h"
#include "rigel/metrics/stats_reporter.h"

namespace rigel {
class HistogramImpl : public Histogram,
                      public StopwatchRecorder,
                      public std::enable_shared_from_this<StopwatchRecorder> {
 public:
  // New is used in place of the default constructor to ensure that callers are
  // returned a shared pointer to a HistogramImpl object since the class
  // inherits from the std::enable_shared_from_this class.
  static std::shared_ptr<HistogramImpl> New(const Buckets &buckets) noexcept;

  // Ensure the class is non-copyable.
  HistogramImpl(const HistogramImpl &) = delete;

  HistogramImpl &operator=(const HistogramImpl &) = delete;

  // Methods to implement the Histogram interface.
  void Record(double) noexcept;

  void Record(std::chrono::nanoseconds) noexcept;

  Stopwatch Start() noexcept;

  // Methods to implement the StopwatchRecorder interface.
  void RecordStopwatch(std::chrono::steady_clock::time_point);

  // Report reports the current values of the Histogram's buckets.
  void Report(const std::string &name,
              const std::unordered_map<std::string, std::string> &tags,
              StatsReporter *reporter);

 private:
  explicit HistogramImpl(const Buckets &buckets) noexcept;

  static std::vector<HistogramBucket> CreateBuckets(const Buckets &buckets);

  std::vector<HistogramBucket> buckets_;
};

}  // namespace rigel
