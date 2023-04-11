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

#include "rigel/metrics/scope_builder.h"

#include "rigel/metrics/scope_impl.h"

namespace rigel {

namespace {
const std::string DEFAULT_PREFIX = "";
const std::string DEFAULT_SEPARATOR = ".";
const std::chrono::seconds DEFAULT_REPORTING_INTERVAL = std::chrono::seconds(0);
const std::unordered_map<std::string, std::string> DEFAULT_TAGS =
    std::unordered_map<std::string, std::string>{};
const std::shared_ptr<StatsReporter> DEFAULT_REPORTER =
    NoopStatsReporter::New();
}  // namespace

ScopeBuilder::ScopeBuilder()
    : prefix_(DEFAULT_PREFIX),
      separator_(DEFAULT_SEPARATOR),
      reporting_interval_(DEFAULT_REPORTING_INTERVAL),
      tags_(DEFAULT_TAGS),
      reporter_(DEFAULT_REPORTER) {}

ScopeBuilder &ScopeBuilder::reporter(
    std::shared_ptr<StatsReporter> reporter) noexcept {
  reporter_ = reporter;
  return *this;
}

ScopeBuilder &ScopeBuilder::prefix(const std::string &prefix) noexcept {
  prefix_ = prefix;
  return *this;
}

ScopeBuilder &ScopeBuilder::separator(const std::string &separator) noexcept {
  separator_ = separator;
  return *this;
}

ScopeBuilder &ScopeBuilder::tags(
    const std::unordered_map<std::string, std::string> &tags) noexcept {
  tags_ = tags;
  return *this;
}

ScopeBuilder &ScopeBuilder::reporting_interval(
    std::chrono::seconds interval) noexcept {
  reporting_interval_ = interval;
  return *this;
}

std::unique_ptr<Scope> ScopeBuilder::Build() noexcept {
  return std::unique_ptr<Scope>{
      new ScopeImpl(this->prefix_, this->separator_, this->tags_,
                    this->reporting_interval_, this->reporter_)};
}

}  // namespace rigel
