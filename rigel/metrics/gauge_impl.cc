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

#include <string>

#include "rigel/metrics/gauge_impl.h"

namespace rigel {

    GaugeImpl::GaugeImpl() noexcept: current_(0), updated_(false) {}

    void GaugeImpl::Update(double value) noexcept {
        current_ = value;
        updated_ = true;
    }

    void GaugeImpl::Report(const std::string &name,
                           const std::unordered_map<std::string, std::string> &tags,
                           StatsReporter *reporter) {
        bool expected = true;
        if (updated_.compare_exchange_strong(expected, false) &&
            reporter != nullptr) {
            reporter->ReportGauge(name, tags, current_);
        }
    }

}  // namespace rigel
