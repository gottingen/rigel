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

#include <string>
#include <unordered_map>

#include "rigel/metrics/stats_reporter.h"

namespace rigel {

class NoopStatsReporter : public StatsReporter {
 public:
  static std::shared_ptr<StatsReporter> New();

  // Methods to implement the StatsReporter interface.
  std::unique_ptr<rigel::Capabilities> Capabilities();

  void Flush();

  void ReportCounter(const std::string &name,
                     const std::unordered_map<std::string, std::string> &tags,
                     int64_t value);

  void ReportGauge(const std::string &name,
                   const std::unordered_map<std::string, std::string> &tags,
                   double value);

  void ReportTimer(const std::string &name,
                   const std::unordered_map<std::string, std::string> &tags,
                   std::chrono::nanoseconds value);

  void ReportHistogramValueSamples(
      const std::string &name,
      const std::unordered_map<std::string, std::string> &tags,
      uint64_t bucket_id, uint64_t num_buckets, double buckets_lower_bound,
      double buckets_upper_bound, uint64_t samples);

  void ReportHistogramDurationSamples(
      const std::string &name,
      const std::unordered_map<std::string, std::string> &tags,
      uint64_t bucket_id, uint64_t num_buckets,
      std::chrono::nanoseconds buckets_lower_bound,
      std::chrono::nanoseconds buckets_upper_bound, uint64_t samples);
};

}  // namespace rigel
