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
#include "rigel/metrics/capabilities.h"
#include "rigel/metrics/counter.h"
#include "rigel/metrics/gauge.h"
#include "rigel/metrics/histogram.h"
#include "rigel/metrics/timer.h"

namespace rigel {

class Scope {
 public:
  virtual ~Scope() = default;

  // Counter returns a new Counter with the provided name.
  virtual std::shared_ptr<rigel::Counter> Counter(
      const std::string &name) noexcept = 0;

  // Gauge returns a new Gauge with the provided name.
  virtual std::shared_ptr<rigel::Gauge> Gauge(
      const std::string &name) noexcept = 0;

  // Timer returns a new Timer with the provided name.
  virtual std::shared_ptr<rigel::Timer> Timer(
      const std::string &name) noexcept = 0;

  // Histogram returns a new Histogram with the provided name.
  virtual std::shared_ptr<rigel::Histogram> Histogram(
      const std::string &name, const Buckets &buckets) noexcept = 0;

  // SubScope creates a new child scope with the same tags as the parent but
  // with the additional name.
  virtual std::shared_ptr<Scope> SubScope(const std::string &name) noexcept = 0;

  // Tagged creates a new child scope with the same name as the parent and with
  // the tags of the parent and those provided. The provided tags take
  // precedence over the parent's tags.
  virtual std::shared_ptr<Scope> Tagged(
      const std::unordered_map<std::string, std::string> &tags) noexcept = 0;

  // Capabilities returns the Capabilities of the Scope.
  virtual std::unique_ptr<rigel::Capabilities> Capabilities() noexcept = 0;
};

}  // namespace rigel
