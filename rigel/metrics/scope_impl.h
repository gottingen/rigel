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

#include <condition_variable>
#include <mutex>
#include <string>
#include <thread>
#include <unordered_map>

#include "rigel/metrics/scope.h"
#include "rigel/metrics/counter_impl.h"
#include "rigel/metrics/gauge_impl.h"
#include "rigel/metrics/histogram_impl.h"
#include "rigel/metrics/timer_impl.h"
#include "rigel/metrics/stats_reporter.h"

namespace rigel {

class ScopeImpl : public Scope {
 public:
  ScopeImpl(const std::string &prefix, const std::string &separator,
            const std::unordered_map<std::string, std::string> &tags,
            std::chrono::seconds interval,
            std::shared_ptr<StatsReporter> reporter) noexcept;

  ~ScopeImpl();

  // Ensure the class is non-copyable.
  ScopeImpl(const ScopeImpl &) = delete;

  ScopeImpl &operator=(const ScopeImpl &) = delete;

  // Methods to implement the Scope interface.
  std::shared_ptr<rigel::Counter> Counter(const std::string &name) noexcept;

  std::shared_ptr<rigel::Gauge> Gauge(const std::string &name) noexcept;

  std::shared_ptr<rigel::Timer> Timer(const std::string &name) noexcept;

  std::shared_ptr<rigel::Histogram> Histogram(const std::string &name,
                                              const Buckets &buckets) noexcept;

  std::shared_ptr<rigel::Scope> SubScope(const std::string &name) noexcept;

  std::shared_ptr<rigel::Scope> Tagged(
      const std::unordered_map<std::string, std::string> &tags) noexcept;

  std::unique_ptr<rigel::Capabilities> Capabilities() noexcept;

 private:
  // SubScope constructs a subscope with the provided prefix and tags.
  std::shared_ptr<rigel::Scope> SubScope(
      const std::string &prefix,
      const std::unordered_map<std::string, std::string> &tags);

  // FullyQualifiedName returns the fully qualified name of the provided name.
  std::string FullyQualifiedName(const std::string &name);

  // ScopeID constructs a unique ID for a scope.
  static std::string ScopeID(
      const std::string &prefix,
      const std::unordered_map<std::string, std::string> &tags);

  // Run is the function used to report metrics from the Scope.
  void Run();

  // Report reports the Scope's metrics to its Reporter.
  void Report();

  const std::string prefix_;
  const std::string separator_;
  const std::unordered_map<std::string, std::string> tags_;
  const std::chrono::nanoseconds interval_;
  std::shared_ptr<StatsReporter> reporter_;

  std::thread thread_;
  std::condition_variable cv_;
  std::mutex running_mutex_;
  bool running_;

  std::mutex registry_mutex_;
  std::unordered_map<std::string, std::shared_ptr<ScopeImpl>> registry_;

  std::mutex counters_mutex_;
  std::unordered_map<std::string, std::shared_ptr<CounterImpl>> counters_;

  std::mutex gauges_mutex_;
  std::unordered_map<std::string, std::shared_ptr<GaugeImpl>> gauges_;

  std::mutex timers_mutex_;
  std::unordered_map<std::string, std::shared_ptr<TimerImpl>> timers_;

  std::mutex histograms_mutex_;
  std::unordered_map<std::string, std::shared_ptr<HistogramImpl>> histograms_;
};

}  // namespace rigel
