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


#ifndef RIGEL_MEMORY_MEMORY_TYPE_H_
#define RIGEL_MEMORY_MEMORY_TYPE_H_

namespace rigel {

    enum MemoryType {
        MEMORY_CPU,
        MEMORY_CPU_PINNED,
        MEMORY_GPU
    };
}  // namespace rigel

#endif // RIGEL_MEMORY_MEMORY_TYPE_H_
