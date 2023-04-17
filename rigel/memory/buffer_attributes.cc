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

#include "rigel/memory/buffer_attributes.h"
#include <cstring>


#include "rigel/memory/constans.h"

namespace rigel {
    void
    BufferAttributes::SetByteSize(const size_t &byte_size) {
        byte_size_ = byte_size;
    }

    void
    BufferAttributes::SetMemoryType(const enum MemoryType &memory_type) {
        memory_type_ = memory_type;
    }

    void
    BufferAttributes::SetMemoryTypeId(const int64_t &memory_type_id) {
        memory_type_id_ = memory_type_id;
    }

    void
    BufferAttributes::SetCudaIpcHandle(void *cuda_ipc_handle) {
        char *lcuda_ipc_handle = reinterpret_cast<char *>(cuda_ipc_handle);
        cuda_ipc_handle_.clear();
        std::copy(
                lcuda_ipc_handle, lcuda_ipc_handle + CUDA_IPC_STRUCT_SIZE,
                std::back_inserter(cuda_ipc_handle_));
    }

    void *
    BufferAttributes::CudaIpcHandle() {
        if (cuda_ipc_handle_.empty()) {
            return nullptr;
        } else {
            return reinterpret_cast<void *>(cuda_ipc_handle_.data());
        }
    }

    size_t
    BufferAttributes::ByteSize() const {
        return byte_size_;
    }

    MemoryType
    BufferAttributes::MemoryType() const {
        return memory_type_;
    }

    int64_t
    BufferAttributes::MemoryTypeId() const {
        return memory_type_id_;
    }

    BufferAttributes::BufferAttributes(
            size_t byte_size, enum MemoryType memory_type,
            int64_t memory_type_id, char *cuda_ipc_handle)
            : byte_size_(byte_size), memory_type_(memory_type),
              memory_type_id_(memory_type_id) {
        // cuda ipc handle size
        cuda_ipc_handle_.reserve(CUDA_IPC_STRUCT_SIZE);

        if (cuda_ipc_handle != nullptr) {
            std::copy(
                    cuda_ipc_handle, cuda_ipc_handle + CUDA_IPC_STRUCT_SIZE,
                    std::back_inserter(cuda_ipc_handle_));
        }
    }
}  // namespace rigel
