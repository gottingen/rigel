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

#include "rigel/metrics/histogram_bucket.h"

#include <string>
#include <unordered_map>
#include <vector>

namespace rigel {

HistogramBucket::HistogramBucket(Buckets::Kind kind, uint64_t bucket_id,
                                 uint64_t num_buckets, double lower_bound,
                                 double upper_bound)
    : kind_(kind),
      bucket_id_(bucket_id),
      num_buckets_(num_buckets),
      lower_bound_(lower_bound),
      upper_bound_(upper_bound),
      samples_(new CounterImpl()) {}

void HistogramBucket::Record() { samples_->Inc(1); }

void HistogramBucket::Report(
    const std::string &name,
    const std::unordered_map<std::string, std::string> &tags,
    StatsReporter *reporter) {
  auto samples = samples_->Value();
  if (samples != 0 && reporter != nullptr) {
    if (kind_ == Buckets::Kind::Values) {
      reporter->ReportHistogramValueSamples(name, tags, bucket_id_,
                                            num_buckets_, lower_bound_,
                                            upper_bound_, samples);
    } else {
      reporter->ReportHistogramDurationSamples(
          name, tags, bucket_id_, num_buckets_,
          std::chrono::nanoseconds(static_cast<int64_t>(lower_bound_)),
          std::chrono::nanoseconds(static_cast<int64_t>(upper_bound_)),
          samples);
    }
  }
}

double HistogramBucket::lower_bound() const { return lower_bound_; }

double HistogramBucket::upper_bound() const { return upper_bound_; }

}  // namespace rigel
