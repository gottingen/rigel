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
#include "rigel/metrics/stopwatch.h"
#include "rigel/metrics/timer.h"

namespace rigel {

class TimerImpl : public Timer,
                  public StopwatchRecorder,
                  public std::enable_shared_from_this<StopwatchRecorder> {
 public:
  // New is used in place of the default constructor to ensure that callers are
  // returned a shared pointer to a TimerImpl object since the class
  // inherits from the std::enable_shared_from_this class.
  static std::shared_ptr<TimerImpl> New(
      const std::string &name,
      const std::unordered_map<std::string, std::string> &tags,
      std::shared_ptr<StatsReporter> reporter) noexcept;

  // Ensure the class is non-copyable.
  TimerImpl(const TimerImpl &) = delete;

  TimerImpl &operator=(const TimerImpl &) = delete;

  // Methods to implement the Timer interface.
  void Record(std::chrono::nanoseconds);

  void Record(int64_t);

  Stopwatch Start();

  // Methods to implement the StopwatchRecorder interface.
  void RecordStopwatch(std::chrono::steady_clock::time_point);

 private:
  TimerImpl(const std::string &name,
            const std::unordered_map<std::string, std::string> &tags,
            std::shared_ptr<StatsReporter> reporter) noexcept;

  const std::string name_;
  const std::unordered_map<std::string, std::string> tags_;
  std::shared_ptr<StatsReporter> reporter_;
};

}  // namespace rigel
