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

#include "rigel/metrics/histogram_impl.h"

#include <limits>
#include <string>
#include <unordered_map>
#include <vector>

namespace rigel {

HistogramImpl::HistogramImpl(const Buckets &buckets) noexcept
    : buckets_(CreateBuckets(buckets)) {}

std::shared_ptr<HistogramImpl> HistogramImpl::New(
    const Buckets &buckets) noexcept {
  return std::shared_ptr<HistogramImpl>(new HistogramImpl(buckets));
}

std::vector<HistogramBucket> HistogramImpl::CreateBuckets(
    const Buckets &buckets) {
  std::vector<HistogramBucket> histogram_buckets;
  auto const size = buckets.size();
  auto const kind = buckets.kind();
  if (size == 0) {
    histogram_buckets.push_back(
        HistogramBucket(kind, 0, 1, std::numeric_limits<double>::min(),
                        std::numeric_limits<double>::max()));
  } else {
    histogram_buckets.reserve(size);

    auto lower_bound = std::numeric_limits<double>::min();
    for (auto it = buckets.begin(); it != buckets.end(); it++) {
      auto upper_bound = *it;
      auto index = static_cast<uint64_t>(std::distance(buckets.begin(), it));
      histogram_buckets.push_back(HistogramBucket(kind, index, buckets.size(),
                                                  lower_bound, upper_bound));
      lower_bound = upper_bound;
    }

    // Add a catch-all bucket for anything past the last bucket.
    histogram_buckets.push_back(
        HistogramBucket(kind, buckets.size(), buckets.size(), lower_bound,
                        std::numeric_limits<double>::max()));
  }

  return histogram_buckets;
}

void HistogramImpl::Record(double val) noexcept {
  // Find the first bucket who's upper bound is greater than val.
  auto it = std::upper_bound(buckets_.begin(), buckets_.end(), val,
                             [](double lhs, const HistogramBucket &rhs) {
                               return lhs < rhs.upper_bound();
                             });
  it->Record();
}

void HistogramImpl::Record(std::chrono::nanoseconds val) noexcept {
  Record(static_cast<double>(val.count()));
}

Stopwatch HistogramImpl::Start() noexcept {
  return Stopwatch(std::chrono::steady_clock::now(), shared_from_this());
}

void HistogramImpl::RecordStopwatch(
    std::chrono::steady_clock::time_point start) {
  auto const duration = std::chrono::steady_clock::now() - start;
  Record(duration);
}

void HistogramImpl::Report(
    const std::string &name,
    const std::unordered_map<std::string, std::string> &tags,
    StatsReporter *reporter) {
  for (auto &bucket : buckets_) {
    bucket.Report(name, tags, reporter);
  }
}

}  // namespace rigel
