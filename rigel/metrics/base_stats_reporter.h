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

#include <memory>

#include "rigel/metrics/capabilities.h"

namespace rigel {

class BaseStatsReporter {
 public:
  virtual ~BaseStatsReporter() = default;

  // Capabilities returns the Capabilities of the BaseStatsReporter.
  virtual std::unique_ptr<rigel::Capabilities> Capabilities() = 0;

  // Flush flushes any metrics the BaseStatsReporter may have buffered.
  virtual void Flush() = 0;
};

}  // namespace rigel
