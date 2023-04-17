//
// Copyright 2023 The Turbo Authors.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      https://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//

#pragma once

#include <map>
#include <thread>
#include <vector>
#include <map>
#include <string>
#include <turbo/base/status.h>

namespace rigel {

        // Helper function to set memory policy and thread affinity on current thread
        turbo::Status SetNumaConfigOnThread(
                const std::map<std::string, std::string> &host_policy);

        // Restrict the memory allocation to specific NUMA node.
        turbo::Status SetNumaMemoryPolicy(
                const std::map<std::string, std::string> &host_policy);

        // Retrieve the node mask used to set memory policy for the current thread
        turbo::Status GetNumaMemoryPolicyNodeMask(unsigned long *node_mask);

        // Reset the memory allocation setting.
        turbo::Status ResetNumaMemoryPolicy();

        // Set a thread affinity to be on specific cpus.
        turbo::Status SetNumaThreadAffinity(
                std::thread::native_handle_type thread,
                const std::map<std::string, std::string> &host_policy);

}  // namespace rigel
