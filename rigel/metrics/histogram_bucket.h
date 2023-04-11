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
#include <vector>

#include "rigel/metrics/buckets.h"
#include "rigel/metrics/counter_impl.h"

namespace rigel {

class HistogramBucket {
 public:
  HistogramBucket(Buckets::Kind kind, uint64_t bucket_id, uint64_t num_buckets,
                  double lower_bound, double upper_bound);

  void Record();
  void Report(const std::string &name,
              const std::unordered_map<std::string, std::string> &tags,
              StatsReporter *reporter);

  double lower_bound() const;
  double upper_bound() const;

 private:
  const Buckets::Kind kind_;
  const uint64_t bucket_id_;
  const uint64_t num_buckets_;
  const double lower_bound_;
  const double upper_bound_;
  std::shared_ptr<CounterImpl> samples_;
};

}  // namespace rigel
