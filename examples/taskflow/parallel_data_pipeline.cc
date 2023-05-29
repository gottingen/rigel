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
// This program demonstrates how to use rigel::DataPipeline to create
// a pipeline with in-pipe data automatically managed by the Taskflow
// library.

#include "rigel/taskflow/taskflow.h"
#include "rigel/taskflow/algorithm/data_pipeline.h"

int main() {

  // dataflow => void -> int -> std::string -> float -> void 
  rigel::Taskflow taskflow("pipeline");
  rigel::Executor executor;

  const size_t num_lines = 3;
  
  // create a pipeline graph
  rigel::DataPipeline pl(num_lines,
    rigel::make_data_pipe<void, int>(rigel::PipeType::SERIAL, [&](rigel::Pipeflow& pf) {
      if(pf.token() == 5) {
        pf.stop();
        return 0;
      }
      else {
        printf("first pipe returns %zu\n", pf.token());
        return static_cast<int>(pf.token());
      }
    }),

    rigel::make_data_pipe<int, std::string>(rigel::PipeType::SERIAL, [](int& input) {
      printf("second pipe returns a strong of %d\n", input + 100);
      return std::to_string(input + 100);
    }),

    rigel::make_data_pipe<std::string, void>(rigel::PipeType::SERIAL, [](std::string& input) {
      printf("third pipe receives the input string %s\n", input.c_str());
    })
  );

  // build the pipeline graph using composition
  taskflow.composed_of(pl).name("pipeline");

  // dump the pipeline graph structure (with composition)
  taskflow.dump(std::cout);

  // run the pipeline
  executor.run(taskflow).wait();

  return 0;
}

