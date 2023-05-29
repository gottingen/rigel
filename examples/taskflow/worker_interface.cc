// Copyright 2023 The titan-search Authors.
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
// This program demonstrates how to change the worker behavior
// upon the creation of an executor.

#include "rigel/taskflow/taskflow.h"

class CustomWorkerBehavior : public rigel::WorkerInterface {

  public:
  
  // to call before the worker enters the scheduling loop
  void scheduler_prologue(rigel::Worker& w) override {
    std::cout << rigel::stringify(
      "worker ", w.id(), " (native=", w.thread()->native_handle(), ") enters scheduler\n"
    ); 
  }

  // to call after the worker leaves the scheduling loop
  void scheduler_epilogue(rigel::Worker& w, std::exception_ptr) override {
    std::cout << rigel::stringify(
      "worker ", w.id(), " (native=", w.thread()->native_handle(), ") leaves scheduler\n"
    ); 
  }
};

int main() {

  rigel::Executor executor(4, std::make_shared<CustomWorkerBehavior>());
  
  return 0;
}
