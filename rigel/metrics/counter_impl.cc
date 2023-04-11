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

#include <cstdint>
#include <string>

#include "rigel/metrics/counter_impl.h"

namespace rigel {

// cppcheck reports a false positive error that previous is not initialized.
//
// cppcheck-suppress uninitMemberVar
CounterImpl::CounterImpl() noexcept : previous_(0), current_(0) {}

void CounterImpl::Inc() noexcept { Inc(1); }

void CounterImpl::Inc(int64_t delta) noexcept { current_ += delta; }

void CounterImpl::Report(
    const std::string &name,
    const std::unordered_map<std::string, std::string> &tags,
    StatsReporter *reporter) {
  auto const val = Value();
  if (val != 0 && reporter != nullptr) {
    reporter->ReportCounter(name, tags, val);
  }
}

int64_t CounterImpl::Value() {
  const auto current = current_.load();
  const auto previous = previous_;
  previous_ = current;
  return current - previous;
}

}  // namespace rigel
