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


#include "pinned_memory_manager.h"

#include <sstream>
#include "rigel/framework/numa_utils.h"
#include "turbo/log/logging.h"

#ifdef RIGEL_ENABLE_GPU
#include <cuda_runtime_api.h>
#endif  // RIGEL_ENABLE_GPU

namespace rigel {

    namespace {

        std::string
        PointerToString(void *ptr) {
            std::stringstream ss;
            ss << ptr;
            return ss.str();
        }

        turbo::Status
        ParseIntOption(const std::string &msg, const std::string &arg, int *value) {
            try {
                *value = std::stoi(arg);
            }
            catch (const std::invalid_argument &ia) {
                return turbo::Status(
                        turbo::kInvalidArgument,
                        msg + ": Can't parse '" + arg + "' to integer");
            }
            return turbo::OkStatus();
        }

    }  // namespace

    std::unique_ptr<PinnedMemoryManager> PinnedMemoryManager::instance_;
    uint64_t PinnedMemoryManager::pinned_memory_byte_size_;

    PinnedMemoryManager::PinnedMemory::PinnedMemory(
            void *pinned_memory_buffer, uint64_t size)
            : pinned_memory_buffer_(pinned_memory_buffer) {
        if (pinned_memory_buffer_ != nullptr) {
            managed_pinned_memory_ = boost::interprocess::managed_external_buffer(
                    boost::interprocess::create_only_t{}, pinned_memory_buffer_, size);
        }
    }


    PinnedMemoryManager::PinnedMemory::~PinnedMemory() {
#ifdef RIGEL_ENABLE_GPU
        if (pinned_memory_buffer_ != nullptr) {
          cudaFreeHost(pinned_memory_buffer_);
        }
#endif  // RIGEL_ENABLE_GPU
    }

    PinnedMemoryManager::~PinnedMemoryManager() {
        // Clean up
        for (const auto &memory_info: memory_info_) {
            const auto &is_pinned = memory_info.second.first;
            if (!is_pinned) {
                free(memory_info.first);
            }
        }
    }

    void
    PinnedMemoryManager::AddPinnedMemoryBuffer(
            const std::shared_ptr<PinnedMemory> &pinned_memory_buffer,
            unsigned long node_mask) {
        pinned_memory_buffers_[node_mask] = pinned_memory_buffer;
    }

    turbo::Status
    PinnedMemoryManager::AllocInternal(
            void **ptr, uint64_t size, MemoryType *allocated_type,
            bool allow_nonpinned_fallback, PinnedMemory *pinned_memory_buffer) {
        auto status = turbo::OkStatus();
        if (pinned_memory_buffer->pinned_memory_buffer_ != nullptr) {
            std::lock_guard<std::mutex> lk(pinned_memory_buffer->buffer_mtx_);
            *ptr = pinned_memory_buffer->managed_pinned_memory_.allocate(
                    size, std::nothrow_t{});
            *allocated_type = MEMORY_CPU_PINNED;
            if (*ptr == nullptr) {
                status = turbo::Status(
                        turbo::kInternal, "failed to allocate pinned system memory");
            }
        } else {
            status = turbo::Status(
                    turbo::kInternal,
                    "failed to allocate pinned system memory: no pinned memory pool");
        }

        bool is_pinned = true;
        if ((!status.ok()) && allow_nonpinned_fallback) {
            static bool warning_logged = false;
            if (!warning_logged) {
                TURBO_LOG(WARNING) << status.ToString()
                                   << ", falling back to non-pinned system memory";
                warning_logged = true;
            }
            *ptr = malloc(size);
            *allocated_type = MEMORY_CPU;
            is_pinned = false;
            if (*ptr == nullptr) {
                status = turbo::Status(
                        turbo::kInternal,
                        "failed to allocate non-pinned system memory");
            } else {
                status = turbo::OkStatus();
            }
        }

        // keep track of allocated buffer or clean up
        {
            std::lock_guard<std::mutex> lk(info_mtx_);
            if (status.ok()) {
                auto res = memory_info_.emplace(
                        *ptr, std::make_pair(is_pinned, pinned_memory_buffer));
                if (!res.second) {
                    status = turbo::Status(
                            turbo::kInternal, "unexpected memory address collision, '" +
                                              PointerToString(*ptr) +
                                              "' has been managed");
                }
                TURBO_DLOG(INFO) << (is_pinned ? "" : "non-")
                                 << "pinned memory allocation: "
                                 << "size " << size << ", addr " << *ptr;
            }
        }

        if ((!status.ok()) && (*ptr != nullptr)) {
            if (is_pinned) {
                std::lock_guard<std::mutex> lk(pinned_memory_buffer->buffer_mtx_);
                pinned_memory_buffer->managed_pinned_memory_.deallocate(*ptr);
            } else {
                free(*ptr);
            }
        }

        return status;
    }

    turbo::Status
    PinnedMemoryManager::FreeInternal(void *ptr) {
        bool is_pinned = true;
        PinnedMemory *pinned_memory_buffer = nullptr;
        {
            std::lock_guard<std::mutex> lk(info_mtx_);
            auto it = memory_info_.find(ptr);
            if (it != memory_info_.end()) {
                is_pinned = it->second.first;
                pinned_memory_buffer = it->second.second;
                TURBO_DLOG(INFO) << (is_pinned ? "" : "non-")
                                 << "pinned memory deallocation: "
                                 << "addr " << ptr;
                memory_info_.erase(it);
            } else {
                return turbo::Status(
                        turbo::kInternal, "unexpected memory address '" +
                                          PointerToString(ptr) +
                                          "' is not being managed");
            }
        }

        if (is_pinned) {
            std::lock_guard<std::mutex> lk(pinned_memory_buffer->buffer_mtx_);
            pinned_memory_buffer->managed_pinned_memory_.deallocate(ptr);
        } else {
            free(ptr);
        }
        return turbo::OkStatus();
    }

    void
    PinnedMemoryManager::Reset() {
        instance_.reset();
    }

    turbo::Status
    PinnedMemoryManager::Create(const Options &options) {
        if (instance_ != nullptr) {
            TURBO_LOG(WARNING) << "New pinned memory pool of size "
                               << options.pinned_memory_pool_byte_size_
                               << " could not be created since one already exists"
                               << " of size " << pinned_memory_byte_size_;
            return turbo::OkStatus();
        }

        instance_.reset(new PinnedMemoryManager());
        if (options.host_policy_map_.empty()) {
            void *buffer = nullptr;
#ifdef RIGEL_ENABLE_GPU
            auto err = cudaHostAlloc(
                &buffer, options.pinned_memory_pool_byte_size_, cudaHostAllocPortable);
            if (err != cudaSuccess) {
              buffer = nullptr;
              LOG_WARNING << "Unable to allocate pinned system memory, pinned memory "
                             "pool will not be available: "
                          << std::string(cudaGetErrorString(err));
            } else if (options.pinned_memory_pool_byte_size_ != 0) {
              LOG_INFO << "Pinned memory pool is created at '"
                       << PointerToString(buffer) << "' with size "
                       << options.pinned_memory_pool_byte_size_;
            } else {
              LOG_INFO << "Pinned memory pool disabled";
            }
#endif  // RIGEL_ENABLE_GPU
            try {
                instance_->AddPinnedMemoryBuffer(
                        std::shared_ptr<PinnedMemory>(
                                new PinnedMemory(buffer, options.pinned_memory_pool_byte_size_)),
                        0);
            }
            catch (const std::exception &ex) {
                return turbo::Status(
                        turbo::kInternal,
                        "Failed to add Pinned Memory buffer: " + std::string(ex.what()));
            }
        } else {
            // Create only one buffer / manager should be created for one node,
            // and all associated devices should request memory from the shared manager
            std::map < int32_t, std::string > numa_map;
            for (const auto host_policy: options.host_policy_map_) {
                const auto numa_it = host_policy.second.find("numa-node");
                if (numa_it != host_policy.second.end()) {
                    int32_t numa_id;
                    if (ParseIntOption("Parsing NUMA node", numa_it->second, &numa_id)
                            .ok()) {
                        numa_map.emplace(numa_id, host_policy.first);
                    }
                }
            }
            for (const auto node_policy: numa_map) {
                auto status =
                        SetNumaMemoryPolicy(options.host_policy_map_.at(node_policy.second));
                if (!status.ok()) {
                    TURBO_LOG(WARNING) << "Unable to allocate pinned system memory for NUMA node "
                                       << node_policy.first << ": " << status.ToString();
                    continue;
                }
                unsigned long node_mask;
                status = GetNumaMemoryPolicyNodeMask(&node_mask);
                if (!status.ok()) {
                    TURBO_LOG(WARNING) << "Unable to get NUMA node set for current thread: "
                                       << status.ToString();
                    continue;
                }
                void *buffer = nullptr;
#ifdef RIGEL_ENABLE_GPU
                auto err = cudaHostAlloc(
                    &buffer, options.pinned_memory_pool_byte_size_,
                    cudaHostAllocPortable);
                if (err != cudaSuccess) {
                  buffer = nullptr;
                  LOG_WARNING << "Unable to allocate pinned system memory, pinned memory "
                                 "pool will not be available: "
                              << std::string(cudaGetErrorString(err));
                } else if (options.pinned_memory_pool_byte_size_ != 0) {
                  LOG_INFO << "Pinned memory pool is created at '"
                           << PointerToString(buffer) << "' with size "
                           << options.pinned_memory_pool_byte_size_;
                } else {
                  LOG_INFO << "Pinned memory pool disabled";
                }
#endif  // RIGEL_ENABLE_GPU
                ResetNumaMemoryPolicy();
                try {
                    instance_->AddPinnedMemoryBuffer(
                            std::shared_ptr<PinnedMemory>(new PinnedMemory(
                                    buffer, options.pinned_memory_pool_byte_size_)),
                            node_mask);
                }
                catch (const std::exception &ex) {
                    return turbo::Status(
                            turbo::kInternal,
                            "Failed to add Pinned Memory buffer with host policy: " +
                            std::string(ex.what()));
                }
            }
            // If no pinned memory is allocated, add an empty entry where all allocation
            // will be on normal system memory
            if (instance_->pinned_memory_buffers_.empty()) {
                try {
                    instance_->AddPinnedMemoryBuffer(
                            std::shared_ptr<PinnedMemory>(new PinnedMemory(
                                    nullptr, options.pinned_memory_pool_byte_size_)),
                            0);
                }
                catch (const std::exception &ex) {
                    return turbo::Status(
                            turbo::kInternal,
                            "Failed to add empty Pinned Memory entry: " +
                            std::string(ex.what()));
                }
            }
        }
        pinned_memory_byte_size_ = options.pinned_memory_pool_byte_size_;
        return turbo::OkStatus();
    }

    turbo::Status
    PinnedMemoryManager::Alloc(
            void **ptr, uint64_t size, MemoryType *allocated_type,
            bool allow_nonpinned_fallback) {
        if (instance_ == nullptr) {
            return turbo::Status(turbo::kUnavailable, "PinnedMemoryManager has not been created");
        }

        auto pinned_memory_buffer =
                instance_->pinned_memory_buffers_.begin()->second.get();
        if (instance_->pinned_memory_buffers_.size() > 1) {
            unsigned long node_mask;
            if (GetNumaMemoryPolicyNodeMask(&node_mask).ok()) {
                auto it = instance_->pinned_memory_buffers_.find(node_mask);
                if (it != instance_->pinned_memory_buffers_.end()) {
                    pinned_memory_buffer = it->second.get();
                }
            }
        }

        return instance_->AllocInternal(
                ptr, size, allocated_type, allow_nonpinned_fallback,
                pinned_memory_buffer);
    }

    turbo::Status
    PinnedMemoryManager::Free(void *ptr) {
        if (instance_ == nullptr) {
            return turbo::Status(
                    turbo::kUnavailable, "PinnedMemoryManager has not been created");
        }

        return instance_->FreeInternal(ptr);
    }

}  // namespace rigel
