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

#include "rigel/metrics/timer_impl.h"

namespace rigel {

TimerImpl::TimerImpl(const std::string &name,
                     const std::unordered_map<std::string, std::string> &tags,
                     std::shared_ptr<StatsReporter> reporter) noexcept
    : name_(name), tags_(tags), reporter_(reporter) {}

std::shared_ptr<TimerImpl> TimerImpl::New(
    const std::string &name,
    const std::unordered_map<std::string, std::string> &tags,
    std::shared_ptr<StatsReporter> reporter) noexcept {
  return std::shared_ptr<TimerImpl>(new TimerImpl(name, tags, reporter));
}

void TimerImpl::Record(std::chrono::nanoseconds value) {
  Record(static_cast<int64_t>(value.count()));
}

void TimerImpl::Record(int64_t value) {
  if (reporter_ != nullptr) {
    reporter_->ReportTimer(name_, tags_, std::chrono::nanoseconds(value));
  }
}

Stopwatch TimerImpl::Start() {
  return Stopwatch(std::chrono::steady_clock::now(), shared_from_this());
}

void TimerImpl::RecordStopwatch(std::chrono::steady_clock::time_point start) {
  auto const duration = std::chrono::steady_clock::now() - start;
  Record(duration);
}

}  // namespace rigel
