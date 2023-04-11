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
#include <cstdint>
#include <mutex>
#include <string>
#include <unordered_map>

#include "rigel/metrics/counter.h"
#include "rigel/metrics/stats_reporter.h"

namespace rigel {

class CounterImpl : public Counter {
 public:
  CounterImpl() noexcept;

  // Ensure the class is non-copyable.
  CounterImpl(const CounterImpl &) = delete;

  CounterImpl &operator=(const CounterImpl &) = delete;

  // Methods to implement the Counter interface.
  void Inc() noexcept;

  void Inc(int64_t) noexcept;

  // Report reports the current value of the counter. It must only be called
  // from a single thread.
  void Report(const std::string &name,
              const std::unordered_map<std::string, std::string> &tags,
              StatsReporter *reporter);

  // Value returns the current value of the counter. It must only be called from
  // a single thread.
  int64_t Value();

 private:
  int64_t previous_;
  std::atomic<int64_t> current_;
};

}  // namespace rigel
