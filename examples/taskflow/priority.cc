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
// This program demonstrates how to set priority to a task.
//
// Currently, Taskflow supports only three priority levels:
//   + rigel::TaskPriority::HIGH   (numerical value = 0)
//   + rigel::TaskPriority::NORMAL (numerical value = 1)
//   + rigel::TaskPriority::LOW    (numerical value = 2)
// 
// Priority-based execution is non-preemptive. Once a task 
// has started to execute, it will execute to completion,
// even if a higher priority task has been spawned or enqueued. 

#include "rigel/taskflow/taskflow.h"

int main() {
  
  // create an executor of only one worker to enable 
  // deterministic behavior
  rigel::Executor executor(1);

  rigel::Taskflow taskflow;

  int counter {0};
  
  // Here we create five tasks and print thier execution
  // orders which should align with assigned priorities
  auto [A, B, C, D, E] = taskflow.emplace(
    [] () { },
    [&] () { 
      std::cout << "Task B: " << counter++ << '\n';  // 0
    },
    [&] () { 
      std::cout << "Task C: " << counter++ << '\n';  // 2
    },
    [&] () { 
      std::cout << "Task D: " << counter++ << '\n';  // 1
    },
    [] () { }
  );

  A.precede(B, C, D); 
  E.succeed(B, C, D);
  
  // By default, all tasks are of rigel::TaskPriority::HIGH
  B.priority(rigel::TaskPriority::HIGH);
  C.priority(rigel::TaskPriority::LOW);
  D.priority(rigel::TaskPriority::NORMAL);

  assert(B.priority() == rigel::TaskPriority::HIGH);
  assert(C.priority() == rigel::TaskPriority::LOW);
  assert(D.priority() == rigel::TaskPriority::NORMAL);
  
  // we should see B, D, and C in their priority order
  executor.run(taskflow).wait();
}

