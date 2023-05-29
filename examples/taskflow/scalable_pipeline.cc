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
// This program demonstrates how to create a pipeline scheduling framework
// that propagates a series of integers and adds one to the result at each
// stage, using a range of pipes provided by the application.
//
// The pipeline has the following structure:
//
// o -> o -> o
// |    |    |
// v    v    v
// o -> o -> o
// |    |    |
// v    v    v
// o -> o -> o
// |    |    |
// v    v    v
// o -> o -> o
//
// Then, the program resets the pipeline to a new range of five pipes.
//
// o -> o -> o -> o -> o
// |    |    |    |    |
// v    v    v    v    v
// o -> o -> o -> o -> o
// |    |    |    |    |
// v    v    v    v    v
// o -> o -> o -> o -> o
// |    |    |    |    |
// v    v    v    v    v
// o -> o -> o -> o -> o

#include "rigel/taskflow/taskflow.h"
#include "rigel/taskflow/algorithm/pipeline.h"

int main() {

  rigel::Taskflow taskflow("pipeline");
  rigel::Executor executor;

  const size_t num_lines = 4;

  // create data storage
  std::array<size_t, num_lines> buffer;

  // define the pipe callable
  auto pipe_callable = [&buffer] (rigel::Pipeflow& pf) mutable {
    switch(pf.pipe()) {
      // first stage generates only 5 scheduling tokens and saves the
      // token number into the buffer.
      case 0: {
        if(pf.token() == 5) {
          pf.stop();
        }
        else {
          printf("stage 1: input token = %zu\n", pf.token());
          buffer[pf.line()] = pf.token();
        }
        return;
      }
      break;

      // other stages propagate the previous result to this pipe and
      // increment it by one
      default: {
        printf(
          "stage %zu: input buffer[%zu] = %zu\n", pf.pipe(), pf.line(), buffer[pf.line()]
        );
        buffer[pf.line()] = buffer[pf.line()] + 1;
      }
      break;
    }
  };

  // create a vector of three pipes
  std::vector< rigel::Pipe<std::function<void(rigel::Pipeflow&)>> > pipes;

  for(size_t i=0; i<3; i++) {
    pipes.emplace_back(rigel::PipeType::SERIAL, pipe_callable);
  }

  // create a pipeline of four parallel lines using the given vector of pipes
  rigel::ScalablePipeline<decltype(pipes)::iterator> pl(num_lines, pipes.begin(), pipes.end());

  // build the pipeline graph using composition
  rigel::Task init = taskflow.emplace([](){ std::cout << "ready\n"; })
                          .name("starting pipeline");
  rigel::Task task = taskflow.composed_of(pl)
                          .name("pipeline");
  rigel::Task stop = taskflow.emplace([](){ std::cout << "stopped\n"; })
                          .name("pipeline stopped");

  // create task dependency
  init.precede(task);
  task.precede(stop);

  // dump the pipeline graph structure (with composition)
  taskflow.dump(std::cout);

  // run the pipeline
  executor.run(taskflow).wait();

  // reset the pipeline to a new range of five pipes and starts from
  // the initial state (i.e., token counts from zero)
  for(size_t i=0; i<2; i++) {
    pipes.emplace_back(rigel::PipeType::SERIAL, pipe_callable);
  }
  pl.reset(pipes.begin(), pipes.end());

  executor.run(taskflow).wait();

  return 0;
}



