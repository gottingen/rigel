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

#include <boost/interprocess/managed_external_buffer.hpp>
#include <map>
#include <memory>
#include <mutex>
#include <turbo/base/status.h>
#include "rigel/memory/memory_type.h"

namespace rigel {

    // This is a singleton class responsible for maintaining pinned memory pool
    // used by the inference server. Pinned memory allocations and deallocations
    // must be requested via functions provided by this class.
    class PinnedMemoryManager {
    public:
        // Options to configure pinned memeory manager.
        struct Options {
            Options(
                    uint64_t b = 0,
                    const std::map<std::string, std::map<std::string, std::string>> &host_policy_map = {})
                    : pinned_memory_pool_byte_size_(b), host_policy_map_(host_policy_map) {
            }

            uint64_t pinned_memory_pool_byte_size_;
            std::map<std::string, std::map<std::string, std::string>> host_policy_map_;
        };

        ~PinnedMemoryManager();

        // Create the pinned memory manager based on 'options' specified.
        // Return Status object indicating success or failure.
        static turbo::Status Create(const Options &options);

        // Allocate pinned memory with the requested 'size' and return the pointer
        // in 'ptr'. If 'allow_nonpinned_fallback' is true, regular system memory
        // will be allocated as fallback in the case where pinned memory fails to
        // be allocated.
        // Return Status object indicating success or failure.
        static turbo::Status Alloc(
                void **ptr, uint64_t size, MemoryType *allocated_type,
                bool allow_nonpinned_fallback);

        // Free the memory allocated by the pinned memory manager.
        // Return Status object indicating success or failure.
        static turbo::Status Free(void *ptr);

    protected:
        // Provide explicit control on the lifecycle of the CUDA memory manager,
        // for testing only.
        static void Reset();

    private:
        class PinnedMemory {
        public:
            PinnedMemory(void *pinned_memory_buffer, uint64_t size);

            ~PinnedMemory();

            void *pinned_memory_buffer_;
            std::mutex buffer_mtx_;
            boost::interprocess::managed_external_buffer managed_pinned_memory_;
        };

        PinnedMemoryManager() = default;

        turbo::Status AllocInternal(
                void **ptr, uint64_t size, MemoryType *allocated_type,
                bool allow_nonpinned_fallback, PinnedMemory *pinned_memory_buffer);

        turbo::Status FreeInternal(void *ptr);

        void AddPinnedMemoryBuffer(
                const std::shared_ptr<PinnedMemory> &pinned_memory_buffer,
                unsigned long node_mask);

        static std::unique_ptr<PinnedMemoryManager> instance_;
        static uint64_t pinned_memory_byte_size_;

        std::mutex info_mtx_;
        std::map<void *, std::pair<bool, PinnedMemory *>> memory_info_;
        std::map<unsigned long, std::shared_ptr<PinnedMemory>> pinned_memory_buffers_;
    };

}  // namespace triton::core
