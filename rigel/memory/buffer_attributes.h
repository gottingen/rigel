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

#include <iterator>
#include <vector>
#include "rigel/memory/memory_type.h"


namespace rigel {
    //
    // A class to hold information about the buffer allocation.
    //
    class BufferAttributes {
    public:
        BufferAttributes(
                size_t byte_size, enum MemoryType memory_type,
                int64_t memory_type_id, char cuda_ipc_handle[64]);

        BufferAttributes() {
            memory_type_ = MEMORY_CPU;
            memory_type_id_ = 0;
            cuda_ipc_handle_.reserve(64);
        }

        // Set the buffer byte size
        void SetByteSize(const size_t &byte_size);

        // Set the buffer memory_type
        void SetMemoryType(const MemoryType &memory_type);

        // Set the buffer memory type id
        void SetMemoryTypeId(const int64_t &memory_type_id);

        // Set the cuda ipc handle
        void SetCudaIpcHandle(void *cuda_ipc_handle);

        // Get the cuda ipc handle
        void *CudaIpcHandle();

        // Get the byte size
        size_t ByteSize() const;

        // Get the memory type
        MemoryType MemoryType() const;

        // Get the memory type id
        int64_t MemoryTypeId() const;

    private:
        size_t byte_size_;
        enum MemoryType memory_type_;
        int64_t memory_type_id_;
        std::vector<char> cuda_ipc_handle_;
    };
}  // namespace rigel

