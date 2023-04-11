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

#include <atomic>
#include <string>
#include <unordered_map>

#include "rigel/metrics/gauge.h"
#include "rigel/metrics/stats_reporter.h"

namespace rigel {

class GaugeImpl : public Gauge {
 public:
  GaugeImpl() noexcept;

  // Ensure the class is non-copyable.
  GaugeImpl(const GaugeImpl &) = delete;

  GaugeImpl &operator=(const GaugeImpl &) = delete;

  // Methods to implement the Gauge interface.
  void Update(double) noexcept;

  // Report reports the current value of the Gauge.
  void Report(const std::string &name,
              const std::unordered_map<std::string, std::string> &tags,
              StatsReporter *reporter);

 private:
  std::atomic<double> current_;
  std::atomic_bool updated_;
};

}  // namespace rigel
