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
#include <cstdint>
#include <memory>
#include <string>
#include <unordered_map>

#include "rigel/metrics/base_stats_reporter.h"
#include "rigel/metrics/buckets.h"
#include "rigel/metrics/histogram.h"

namespace rigel {

class StatsReporter : public BaseStatsReporter {
 public:
  virtual ~StatsReporter() = default;

  // Methods to report metrics.
  virtual void ReportCounter(
      const std::string &name,
      const std::unordered_map<std::string, std::string> &tags,
      int64_t value) = 0;

  virtual void ReportGauge(
      const std::string &name,
      const std::unordered_map<std::string, std::string> &tags,
      double value) = 0;

  virtual void ReportTimer(
      const std::string &name,
      const std::unordered_map<std::string, std::string> &tags,
      std::chrono::nanoseconds value) = 0;

  virtual void ReportHistogramValueSamples(
      const std::string &name,
      const std::unordered_map<std::string, std::string> &tags,
      uint64_t bucket_id, uint64_t num_buckets, double buckets_lower_bound,
      double buckets_upper_bound, uint64_t samples) = 0;

  virtual void ReportHistogramDurationSamples(
      const std::string &name,
      const std::unordered_map<std::string, std::string> &tags,
      uint64_t bucket_id, uint64_t num_buckets,
      std::chrono::nanoseconds buckets_lower_bound,
      std::chrono::nanoseconds buckets_upper_bound, uint64_t samples) = 0;
};

}  // namespace rigel
