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

#include "rigel/metrics/buckets.h"
#include "rigel/metrics/scope.h"
#include "rigel/metrics/noop_stats_reporter.h"
#include "rigel/metrics/stats_reporter.h"

namespace rigel {

class ScopeBuilder {
 public:
  ScopeBuilder();

  // Methods to set various options for a Scope.
  ScopeBuilder &reporter(std::shared_ptr<StatsReporter> reporter) noexcept;

  ScopeBuilder &prefix(const std::string &prefix) noexcept;

  ScopeBuilder &separator(const std::string &separator) noexcept;

  ScopeBuilder &tags(
      const std::unordered_map<std::string, std::string> &tags) noexcept;

  ScopeBuilder &reporting_interval(std::chrono::seconds interval) noexcept;

  // Build constructs a Scope and begins reporting metrics if the scope's
  // reporting interval is non-zero.
  std::unique_ptr<Scope> Build() noexcept;

 private:
  std::string prefix_;
  std::string separator_;
  std::chrono::seconds reporting_interval_;
  std::unordered_map<std::string, std::string> tags_;
  std::shared_ptr<StatsReporter> reporter_;
};

}  // namespace rigel
