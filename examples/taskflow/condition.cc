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
// The example creates the following cyclic graph of one iterative loop:
//
//       A
//       |
//       v
//       B<---|
//       |    |
//       v    |
//       C----|
//       |
//       v
//       D
//
// - A is a task that initializes a counter to zero
// - B is a task that increments the counter
// - C is a condition task that loops with B until the counter
//   reaches a breaking number
// - D is a task that finalizes the result

#include "rigel/taskflow/taskflow.h"

int main() {

  rigel::Executor executor;
  rigel::Taskflow taskflow("Conditional Tasking Demo");

  int counter;

  auto A = taskflow.emplace([&](){
    std::cout << "initializes the counter to zero\n";
    counter = 0;
  }).name("A");

  auto B = taskflow.emplace([&](){
    std::cout << "loops to increment the counter\n";
    counter++;
  }).name("B");

  auto C = taskflow.emplace([&](){
    std::cout << "counter is " << counter << " -> ";
    if(counter != 5) {
      std::cout << "loops again (goes to B)\n";
      return 0;
    }
    std::cout << "breaks the loop (goes to D)\n";
    return 1;
  }).name("C");

  auto D = taskflow.emplace([&](){
    std::cout << "done with counter equal to " << counter << '\n';
  }).name("D");

  A.precede(B);
  B.precede(C);
  C.precede(B);
  C.precede(D);

  // visualizes the taskflow
  taskflow.dump(std::cout);

  // executes the taskflow
  executor.run(taskflow).wait();

  assert(counter == 5);

  return 0;
}







