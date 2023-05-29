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
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN

#include "tests/doctest.h"
#include "rigel/taskflow/taskflow.h"
#include "rigel/taskflow/algorithm/pipeline.h"


// --------------------------------------------------------
// Testcase: RuntimeTasking
// --------------------------------------------------------

TEST_CASE("Runtime.Basics" * doctest::timeout(300)) {

  rigel::Taskflow taskflow;
  rigel::Executor executor;

  int a = 0;
  int b = 0;

  taskflow.emplace([&](rigel::Runtime& rt){
    rt.corun([&](rigel::Subflow& sf){
      REQUIRE(&rt.executor() == &executor);
      auto task1 = sf.emplace([&](){a++;});
      auto task2 = sf.emplace([&](){a++;});
      auto task3 = sf.emplace([&](){a++;});
      auto task4 = sf.emplace([&](){a++;});
      auto task5 = sf.emplace([&](){a++;});
      task1.precede(task2);
      task2.precede(task3);
      task3.precede(task4);
      task4.precede(task5);
    });
  });

  taskflow.emplace([&](rigel::Subflow& sf){
    sf.emplace([&](rigel::Runtime& rt){
      REQUIRE(&rt.executor() == &executor);
      rt.corun([&](rigel::Subflow& sf){
        auto task1 = sf.emplace([&](){b++;});
        auto task2 = sf.emplace([&](){b++;});
        auto task3 = sf.emplace([&](){b++;});
        auto task4 = sf.emplace([&](){b++;});
        auto task5 = sf.emplace([&](){b++;});
        task1.precede(task2);
        task2.precede(task3);
        task3.precede(task4);
        task4.precede(task5);
        sf.detach();
      });
    });
  });

  executor.run(taskflow).wait();

  REQUIRE(a == 5);
  REQUIRE(b == 5);
}

// --------------------------------------------------------
// Testcase: Runtime.Schedule.ModuleTask
// --------------------------------------------------------

TEST_CASE("Runtime.Schedule.ModuleTask" * doctest::timeout(300)) {

  rigel::Taskflow tf;
  int value {0};

  auto a = tf.emplace([&]() { value = -100; }).name("A");
  auto module_task = tf.placeholder().name("module");
  auto b = tf.emplace([&]() { value++; }).name("B");
  auto c = tf.emplace([&]() { value++; }).name("C");

  a.precede(module_task);
  module_task.precede(b);
  b.precede(c);

  rigel::Taskflow module_flow;
  auto m1 = module_flow.emplace([&]() { value++; }).name("m1");
  auto m2 = module_flow.emplace([&]() { value++; }).name("m2");
  m1.precede(m2);

  module_task.composed_of(module_flow);

  auto entrypoint = tf.emplace([]() { return 0; }).name("entrypoint");
  auto schedule = tf.emplace([&](rigel::Runtime& runtime) {
    value++;
    runtime.schedule(module_task);
  });
  entrypoint.precede(schedule, a);

  rigel::Executor executor;
  executor.run(tf).wait();

  REQUIRE(value == 5);
}

// --------------------------------------------------------
// Testcase: Runtime.ExternalGraph.Simple
// --------------------------------------------------------

TEST_CASE("Runtime.ExternalGraph.Simple" * doctest::timeout(300)) {

  const size_t N = 100;

  rigel::Executor executor;
  rigel::Taskflow taskflow;
  
  std::vector<int> results(N, 0);
  std::vector<rigel::Taskflow> graphs(N);

  for(size_t i=0; i<N; i++) {

    auto& fb = graphs[i];

    auto A = fb.emplace([&res=results[i]]()mutable{ ++res; });
    auto B = fb.emplace([&res=results[i]]()mutable{ ++res; });
    auto C = fb.emplace([&res=results[i]]()mutable{ ++res; });
    auto D = fb.emplace([&res=results[i]]()mutable{ ++res; });

    A.precede(B);
    B.precede(C);
    C.precede(D);

    taskflow.emplace([&res=results[i], &graph=graphs[i]](rigel::Runtime& rt)mutable{
      rt.corun(graph);
    });
  }
  
  executor.run_n(taskflow, 100).wait();

  for(size_t i=0; i<N; i++) {
    REQUIRE(results[i] == 400);
  }

}

// --------------------------------------------------------
// Testcase: Runtime.Subflow
// --------------------------------------------------------

void runtime_subflow(size_t w) {
  
  const size_t runtime_tasks_per_line = 20;
  const size_t lines = 4;
  const size_t subtasks = 4096;

  rigel::Executor executor(w);
  rigel::Taskflow parent;
  rigel::Taskflow taskflow;

  for (size_t subtask = 0; subtask <= subtasks; subtask = subtask == 0 ? subtask + 1 : subtask*2) {
    
    parent.clear();
    taskflow.clear();

    auto init = taskflow.emplace([](){}).name("init");
    auto end  = taskflow.emplace([](){}).name("end");

    std::vector<rigel::Task> rts;
    std::atomic<size_t> sums = 0;
    
    for (size_t i = 0; i < runtime_tasks_per_line * lines; ++i) {
      std::string rt_name = "rt-" + std::to_string(i);
      
      rts.emplace_back(taskflow.emplace([&sums, &subtask](rigel::Runtime& rt) {
        rt.corun([&sums, &subtask](rigel::Subflow& sf) {
          for (size_t j = 0; j < subtask; ++j) {
            sf.emplace([&sums]() {
              sums.fetch_add(1, std::memory_order_relaxed);
              //std::this_thread::sleep_for(std::chrono::nanoseconds(1));
            });
          }  
        });
      }).name(rt_name));
    }

    for (size_t l = 0; l < lines; ++l) {
      init.precede(rts[l*runtime_tasks_per_line]);
    }

    for (size_t l = 0; l < lines; ++l) {
      for (size_t i = 0; i < runtime_tasks_per_line-1; ++i) {
        rts[i+l*runtime_tasks_per_line].precede(rts[i+l*runtime_tasks_per_line+1]);
      }
    }

    for (size_t l = 1; l < lines+1; ++l) {
      end.succeed(rts[runtime_tasks_per_line*l-1]);
    }

    parent.composed_of(taskflow);

    executor.run(parent).wait();
    //taskflow.dump(std::cout);
    REQUIRE(sums == runtime_tasks_per_line*lines*subtask);
  }
}

TEST_CASE("Runtime.Subflow.1thread" * doctest::timeout(300)){
  runtime_subflow(1);
}

TEST_CASE("Runtime.Subflow.2threads" * doctest::timeout(300)){
  runtime_subflow(2);
}

TEST_CASE("Runtime.Subflow.3threads" * doctest::timeout(300)){
  runtime_subflow(3);
}

TEST_CASE("Runtime.Subflow.4threads" * doctest::timeout(300)){
  runtime_subflow(4);
}

TEST_CASE("Runtime.Subflow.5threads" * doctest::timeout(300)){
  runtime_subflow(5);
}

TEST_CASE("Runtime.Subflow.6threads" * doctest::timeout(300)){
  runtime_subflow(6);
}

TEST_CASE("Runtime.Subflow.7threads" * doctest::timeout(300)){
  runtime_subflow(7);
}

TEST_CASE("Runtime.Subflow.8threads" * doctest::timeout(300)){
  runtime_subflow(8);
}


// --------------------------------------------------------
// Testcase: Pipeline(SP).Runtime.Subflow
// --------------------------------------------------------

void pipeline_sp_runtime_subflow(size_t w) {
  
  size_t num_lines = 2;
  size_t subtask = 2;
  size_t max_tokens = 100000;

  rigel::Executor executor(w);
  rigel::Taskflow taskflow;
 
  //for (subtask = 0; subtask <= subtasks; subtask = subtask == 0 ? subtask + 1 : subtask*2) {
   
    std::atomic<size_t> sums = 0;
    rigel::Pipeline pl(
      num_lines, 
      rigel::Pipe{
        rigel::PipeType::SERIAL, [max_tokens](rigel::Pipeflow& pf){
          //std::cout << rigel::stringify(pf.token(), '\n');
          if (pf.token() == max_tokens) {
            pf.stop();
          }
        }
      },

      rigel::Pipe{
        rigel::PipeType::PARALLEL, [subtask, &sums](rigel::Pipeflow&, rigel::Runtime& rt) {
          rt.corun([subtask, &sums](rigel::Subflow& sf) {
            for (size_t i = 0; i < subtask; ++i) {
              sf.emplace([&sums](){
                sums.fetch_add(1, std::memory_order_relaxed);  
              });
            }
          });
        }
      }
    );

    taskflow.composed_of(pl).name("pipeline");
    executor.run(taskflow).wait();
    REQUIRE(sums == subtask*max_tokens);
  //}
}


TEST_CASE("Pipeline(SP).Runtime.Subflow.1thread" * doctest::timeout(300)){
  pipeline_sp_runtime_subflow(1);
}

TEST_CASE("Pipeline(SP).Runtime.Subflow.2threads" * doctest::timeout(300)){
  pipeline_sp_runtime_subflow(2);
}

TEST_CASE("Pipeline(SP).Runtime.Subflow.3threads" * doctest::timeout(300)){
  pipeline_sp_runtime_subflow(3);
}

TEST_CASE("Pipeline(SP).Runtime.Subflow.4threads" * doctest::timeout(300)){
  pipeline_sp_runtime_subflow(4);
}

TEST_CASE("Pipeline(SP).Runtime.Subflow.5threads" * doctest::timeout(300)){
  pipeline_sp_runtime_subflow(5);
}

TEST_CASE("Pipeline(SP).Runtime.Subflow.6threads" * doctest::timeout(300)){
  pipeline_sp_runtime_subflow(6);
}

TEST_CASE("Pipeline(SP).Runtime.Subflow.7threads" * doctest::timeout(300)){
  pipeline_sp_runtime_subflow(7);
}

TEST_CASE("Pipeline(SP).Runtime.Subflow.8threads" * doctest::timeout(300)){
  pipeline_sp_runtime_subflow(8);
}


// --------------------------------------------------------
// Testcase: Pipeline(SPSPSPSP).Runtime.Subflow
// --------------------------------------------------------

void pipeline_spspspsp_runtime_subflow(size_t w) {
  
  size_t num_lines = 4;
  size_t subtasks = 8;
  size_t max_tokens = 4096;

  rigel::Executor executor(w);
  rigel::Taskflow taskflow;
 
  for (size_t subtask = 0; subtask <= subtasks; subtask = subtask == 0 ? subtask + 1 : subtask*2) {
   
    taskflow.clear();
    
    std::atomic<size_t> sums = 0;
    rigel::Pipeline pl(
      num_lines, 
      rigel::Pipe{
        rigel::PipeType::SERIAL, [max_tokens](rigel::Pipeflow& pf){
          if (pf.token() == max_tokens) {
            pf.stop();
          }
        }
      },

      rigel::Pipe{
        rigel::PipeType::PARALLEL, [subtask, &sums](rigel::Pipeflow&, rigel::Runtime& rt) {
          rt.corun([subtask, &sums](rigel::Subflow& sf) {
            for (size_t i = 0; i < subtask; ++i) {
              sf.emplace([&sums](){
                sums.fetch_add(1, std::memory_order_relaxed);  
              });
            }
          });
        }
      },

      rigel::Pipe{
        rigel::PipeType::SERIAL, [subtask, &sums](rigel::Pipeflow&, rigel::Runtime& rt) {
          rt.corun([subtask, &sums](rigel::Subflow& sf) {
            for (size_t i = 0; i < subtask; ++i) {
              sf.emplace([&sums](){
                sums.fetch_add(1, std::memory_order_relaxed);  
              });
            }
          });
        }
      },

      rigel::Pipe{
        rigel::PipeType::PARALLEL, [subtask, &sums](rigel::Pipeflow&, rigel::Runtime& rt) {
          rt.corun([subtask, &sums](rigel::Subflow& sf) {
            for (size_t i = 0; i < subtask; ++i) {
              sf.emplace([&sums](){
                sums.fetch_add(1, std::memory_order_relaxed);  
              });
            }
          });
        }
      },

      rigel::Pipe{
        rigel::PipeType::SERIAL, [subtask, &sums](rigel::Pipeflow&, rigel::Runtime& rt) {
          rt.corun([subtask, &sums](rigel::Subflow& sf) {
            for (size_t i = 0; i < subtask; ++i) {
              sf.emplace([&sums](){
                sums.fetch_add(1, std::memory_order_relaxed);  
              });
            }
          });
        }
      },

      rigel::Pipe{
        rigel::PipeType::PARALLEL, [subtask, &sums](rigel::Pipeflow&, rigel::Runtime& rt) {
          rt.corun([subtask, &sums](rigel::Subflow& sf) {
            for (size_t i = 0; i < subtask; ++i) {
              sf.emplace([&sums](){
                sums.fetch_add(1, std::memory_order_relaxed);  
              });
            }
          });
        }
      },

      rigel::Pipe{
        rigel::PipeType::SERIAL, [subtask, &sums](rigel::Pipeflow&, rigel::Runtime& rt) {
          rt.corun([subtask, &sums](rigel::Subflow& sf) {
            for (size_t i = 0; i < subtask; ++i) {
              sf.emplace([&sums](){
                sums.fetch_add(1, std::memory_order_relaxed);  
              });
            }
          });
        }
      },

      rigel::Pipe{
        rigel::PipeType::PARALLEL, [subtask, &sums](rigel::Pipeflow&, rigel::Runtime& rt) {
          rt.corun([subtask, &sums](rigel::Subflow& sf) {
            for (size_t i = 0; i < subtask; ++i) {
              sf.emplace([&sums](){
                sums.fetch_add(1, std::memory_order_relaxed);  
              });
            }
          });
        }
      }
    );

    taskflow.composed_of(pl).name("pipeline");
    executor.run(taskflow).wait();
    REQUIRE(sums == subtask*max_tokens*7);
  }
}


TEST_CASE("Pipeline(SPSPSPSP).Runtime.Subflow.1thread" * doctest::timeout(300)){
  pipeline_spspspsp_runtime_subflow(1);
}

TEST_CASE("Pipeline(SPSPSPSP).Runtime.Subflow.2threads" * doctest::timeout(300)){
  pipeline_spspspsp_runtime_subflow(2);
}

TEST_CASE("Pipeline(SPSPSPSP).Runtime.Subflow.3threads" * doctest::timeout(300)){
  pipeline_spspspsp_runtime_subflow(3);
}

TEST_CASE("Pipeline(SPSPSPSP).Runtime.Subflow.4threads" * doctest::timeout(300)){
  pipeline_spspspsp_runtime_subflow(4);
}

TEST_CASE("Pipeline(SPSPSPSP).Runtime.Subflow.5threads" * doctest::timeout(300)){
  pipeline_spspspsp_runtime_subflow(5);
}

TEST_CASE("Pipeline(SPSPSPSP).Runtime.Subflow.6threads" * doctest::timeout(300)){
  pipeline_spspspsp_runtime_subflow(6);
}

TEST_CASE("Pipeline(SPSPSPSP).Runtime.Subflow.7threads" * doctest::timeout(300)){
  pipeline_spspspsp_runtime_subflow(7);
}

TEST_CASE("Pipeline(SPSPSPSP).Runtime.Subflow.8threads" * doctest::timeout(300)){
  pipeline_spspspsp_runtime_subflow(8);
}


// --------------------------------------------------------
// Testcase: Pipeline(SPSPSPSP).Runtime.IrregularSubflow
// --------------------------------------------------------

void pipeline_spspspsp_runtime_irregular_subflow(size_t w) {
  
  size_t num_lines = 4;
  size_t max_tokens = 32767;

  rigel::Executor executor(w);
  rigel::Taskflow taskflow;
 
  std::atomic<size_t> sums = 0;
  
  rigel::Pipeline pl(
    num_lines, 
    rigel::Pipe{
      rigel::PipeType::SERIAL, [max_tokens](rigel::Pipeflow& pf){
        if (pf.token() == max_tokens) {
          pf.stop();
        }
      }
    },

    /* subflow has the following dependency
     *    
     *     |--> B
     *  A--|
     *     |--> C
     */
    rigel::Pipe{
      rigel::PipeType::PARALLEL, [&sums](rigel::Pipeflow&, rigel::Runtime& rt) {
        rt.corun([&sums](rigel::Subflow& sf) {
          auto A = sf.emplace([&sums]() { sums.fetch_add(1, std::memory_order_relaxed); });
          auto B = sf.emplace([&sums]() { sums.fetch_add(1, std::memory_order_relaxed); });
          auto C = sf.emplace([&sums]() { sums.fetch_add(1, std::memory_order_relaxed); });
          A.precede(B, C);
        });
      }
    },

    /* subflow has the following dependency
     *
     *     |--> B--| 
     *     |       v
     *  A--|       D
     *     |       ^
     *     |--> C--|
     *
     */
    rigel::Pipe{
      rigel::PipeType::SERIAL, [&sums](rigel::Pipeflow&, rigel::Runtime& rt) {
        rt.corun([&sums](rigel::Subflow& sf) {
          auto A = sf.emplace([&sums]() { sums.fetch_add(1, std::memory_order_relaxed); });
          auto B = sf.emplace([&sums]() { sums.fetch_add(1, std::memory_order_relaxed); });
          auto C = sf.emplace([&sums]() { sums.fetch_add(1, std::memory_order_relaxed); });
          auto D = sf.emplace([&sums]() { sums.fetch_add(1, std::memory_order_relaxed); });
          A.precede(B, C);
          D.succeed(B, C);
        });
      }
    },

    /* subflow has the following dependency
     *
     *       |--> C 
     *       |       
     *  A--> B       
     *       |       
     *       |--> D 
     *
     */
    rigel::Pipe{
      rigel::PipeType::PARALLEL, [&sums](rigel::Pipeflow&, rigel::Runtime& rt) {
        rt.corun([&sums](rigel::Subflow& sf) {
          auto A = sf.emplace([&sums]() { sums.fetch_add(1, std::memory_order_relaxed); });
          auto B = sf.emplace([&sums]() { sums.fetch_add(1, std::memory_order_relaxed); });
          auto C = sf.emplace([&sums]() { sums.fetch_add(1, std::memory_order_relaxed); });
          auto D = sf.emplace([&sums]() { sums.fetch_add(1, std::memory_order_relaxed); });
          A.precede(B);
          B.precede(C, D);
        });
      }
    },

    /* subflow has the following dependency
     *
     *     |--> B--|   |--> E
     *     |       v   |
     *  A--|       D --| 
     *     |       ^   |
     *     |--> C--|   |--> F
     *
     */
    rigel::Pipe{
      rigel::PipeType::SERIAL, [&sums](rigel::Pipeflow&, rigel::Runtime& rt) {
        rt.corun([&sums](rigel::Subflow& sf) {
          auto A = sf.emplace([&sums]() { sums.fetch_add(1, std::memory_order_relaxed); });
          auto B = sf.emplace([&sums]() { sums.fetch_add(1, std::memory_order_relaxed); });
          auto C = sf.emplace([&sums]() { sums.fetch_add(1, std::memory_order_relaxed); });
          auto D = sf.emplace([&sums]() { sums.fetch_add(1, std::memory_order_relaxed); });
          auto E = sf.emplace([&sums]() { sums.fetch_add(1, std::memory_order_relaxed); });
          auto F = sf.emplace([&sums]() { sums.fetch_add(1, std::memory_order_relaxed); });
          A.precede(B, C);
          D.succeed(B, C);
          D.precede(E, F);
        });
      }
    },

    /* subflow has the following dependency
     *
     *  A --> B --> C --> D -->  E
     *
     */
    rigel::Pipe{
      rigel::PipeType::PARALLEL, [&sums](rigel::Pipeflow&, rigel::Runtime& rt) {
        rt.corun([&sums](rigel::Subflow& sf) {
          auto A = sf.emplace([&sums]() { sums.fetch_add(1, std::memory_order_relaxed); });
          auto B = sf.emplace([&sums]() { sums.fetch_add(1, std::memory_order_relaxed); });
          auto C = sf.emplace([&sums]() { sums.fetch_add(1, std::memory_order_relaxed); });
          auto D = sf.emplace([&sums]() { sums.fetch_add(1, std::memory_order_relaxed); });
          auto E = sf.emplace([&sums]() { sums.fetch_add(1, std::memory_order_relaxed); });
          A.precede(B);
          B.precede(C);
          C.precede(D);
          D.precede(E);
        });
      }
    },

    /* subflow has the following dependency
     *    
     *        |-----------|
     *        |           v
     *  A --> B --> C --> D -->  E
     *              |            ^
     *              |------------|
     *
     */
    rigel::Pipe{
      rigel::PipeType::SERIAL, [&sums](rigel::Pipeflow&, rigel::Runtime& rt) {
        rt.corun([&sums](rigel::Subflow& sf) {
          auto A = sf.emplace([&sums]() { sums.fetch_add(1, std::memory_order_relaxed); });
          auto B = sf.emplace([&sums]() { sums.fetch_add(1, std::memory_order_relaxed); });
          auto C = sf.emplace([&sums]() { sums.fetch_add(1, std::memory_order_relaxed); });
          auto D = sf.emplace([&sums]() { sums.fetch_add(1, std::memory_order_relaxed); });
          auto E = sf.emplace([&sums]() { sums.fetch_add(1, std::memory_order_relaxed); });
          A.precede(B);
          B.precede(C, D);
          C.precede(D, E);
          D.precede(E);
        });
      }
    },

    /* subflow has the following dependency
     *    
     *  |-----------|
     *  |           v
     *  A --> B --> C --> D 
     *  |                 ^
     *  |-----------------|
     *
     */
    rigel::Pipe{
      rigel::PipeType::PARALLEL, [&sums](rigel::Pipeflow&, rigel::Runtime& rt) {
        rt.corun([&sums](rigel::Subflow& sf) {
          auto A = sf.emplace([&sums]() { sums.fetch_add(1, std::memory_order_relaxed); });
          auto B = sf.emplace([&sums]() { sums.fetch_add(1, std::memory_order_relaxed); });
          auto C = sf.emplace([&sums]() { sums.fetch_add(1, std::memory_order_relaxed); });
          auto D = sf.emplace([&sums]() { sums.fetch_add(1, std::memory_order_relaxed); });
          A.precede(B, C, D);
          B.precede(C);
          C.precede(D);
        });
      }
    }
  );

  taskflow.composed_of(pl).name("pipeline");
  executor.run(taskflow).wait();

  //taskflow.dump(std::cout);
  // there are 31 spawned subtasks in total
  REQUIRE(sums == 31*max_tokens);
}


TEST_CASE("Pipeline(SPSPSPSP).Runtime.Irregular.Subflow.1thread" * doctest::timeout(300)){
  pipeline_spspspsp_runtime_irregular_subflow(1);
}

TEST_CASE("Pipeline(SPSPSPSP).Runtime.Irregular.Subflow.2threads" * doctest::timeout(300)){
  pipeline_spspspsp_runtime_irregular_subflow(2);
}

TEST_CASE("Pipeline(SPSPSPSP).Runtime.Irregular.Subflow.3threads" * doctest::timeout(300)){
  pipeline_spspspsp_runtime_irregular_subflow(3);
}

TEST_CASE("Pipeline(SPSPSPSP).Runtime.Irregular.Subflow.4threads" * doctest::timeout(300)){
  pipeline_spspspsp_runtime_irregular_subflow(4);
}

TEST_CASE("Pipeline(SPSPSPSP).Runtime.Irregular.Subflow.5threads" * doctest::timeout(300)){
  pipeline_spspspsp_runtime_irregular_subflow(5);
}

TEST_CASE("Pipeline(SPSPSPSP).Runtime.Irregular.Subflow.6threads" * doctest::timeout(300)){
  pipeline_spspspsp_runtime_irregular_subflow(6);
}

TEST_CASE("Pipeline(SPSPSPSP).Runtime.Irregular.Subflow.7threads" * doctest::timeout(300)){
  pipeline_spspspsp_runtime_irregular_subflow(7);
}

TEST_CASE("Pipeline(SPSPSPSP).Runtime.Irregular.Subflow.8threads" * doctest::timeout(300)){
  pipeline_spspspsp_runtime_irregular_subflow(8);
}

// --------------------------------------------------------
// Testcase: ScalablePipeline(SPSPSPSP).Runtime.Subflow
// --------------------------------------------------------

void scalable_pipeline_spspspsp_runtime_subflow(size_t w) {
  
  size_t num_lines = 4;
  size_t subtasks = 8;
  size_t max_tokens = 4096;

  rigel::Executor executor(w);
  rigel::Taskflow taskflow;

  using pipe_t = rigel::Pipe<std::function<void(rigel::Pipeflow&, rigel::Runtime&)>>;
  std::vector<pipe_t> pipes;

  rigel::ScalablePipeline<std::vector<pipe_t>::iterator> sp;
 
  for (size_t subtask = 0; subtask <= subtasks; subtask = subtask == 0 ? subtask + 1 : subtask*2) {
   
    taskflow.clear();
    pipes.clear();
    
    std::atomic<size_t> sums = 0;

    pipes.emplace_back(rigel::PipeType::SERIAL, [max_tokens](rigel::Pipeflow& pf, rigel::Runtime&){
      if (pf.token() == max_tokens) {
        pf.stop();
      }
    });


    pipes.emplace_back(rigel::PipeType::PARALLEL, [subtask, &sums](rigel::Pipeflow&, rigel::Runtime& rt) {
      rt.corun([subtask, &sums](rigel::Subflow& sf) {
        for (size_t i = 0; i < subtask; ++i) {
          sf.emplace([&sums](){
            sums.fetch_add(1, std::memory_order_relaxed);  
          });
        }
      });
    });

    pipes.emplace_back(rigel::PipeType::SERIAL, [subtask, &sums](rigel::Pipeflow&, rigel::Runtime& rt) {
      rt.corun([subtask, &sums](rigel::Subflow& sf) {
        for (size_t i = 0; i < subtask; ++i) {
          sf.emplace([&sums](){
            sums.fetch_add(1, std::memory_order_relaxed);  
          });
        }
      });
    });
    
    pipes.emplace_back(rigel::PipeType::PARALLEL, [subtask, &sums](rigel::Pipeflow&, rigel::Runtime& rt) {
      rt.corun([subtask, &sums](rigel::Subflow& sf) {
        for (size_t i = 0; i < subtask; ++i) {
          sf.emplace([&sums](){
            sums.fetch_add(1, std::memory_order_relaxed);  
          });
        }
      });
    });
    
    pipes.emplace_back(rigel::PipeType::SERIAL, [subtask, &sums](rigel::Pipeflow&, rigel::Runtime& rt) {
      rt.corun([subtask, &sums](rigel::Subflow& sf) {
        for (size_t i = 0; i < subtask; ++i) {
          sf.emplace([&sums](){
            sums.fetch_add(1, std::memory_order_relaxed);  
          });
        }
      });
    });

    pipes.emplace_back(rigel::PipeType::PARALLEL, [subtask, &sums](rigel::Pipeflow&, rigel::Runtime& rt) {
      rt.corun([subtask, &sums](rigel::Subflow& sf) {
        for (size_t i = 0; i < subtask; ++i) {
          sf.emplace([&sums](){
            sums.fetch_add(1, std::memory_order_relaxed);  
          });
        }
      });
    });
    

    pipes.emplace_back(rigel::PipeType::SERIAL, [subtask, &sums](rigel::Pipeflow&, rigel::Runtime& rt) {
      rt.corun([subtask, &sums](rigel::Subflow& sf) {
        for (size_t i = 0; i < subtask; ++i) {
          sf.emplace([&sums](){
            sums.fetch_add(1, std::memory_order_relaxed);  
          });
        }
      });
    });

    pipes.emplace_back(rigel::PipeType::PARALLEL, [subtask, &sums](rigel::Pipeflow&, rigel::Runtime& rt) {
      rt.corun([subtask, &sums](rigel::Subflow& sf) {
        for (size_t i = 0; i < subtask; ++i) {
          sf.emplace([&sums](){
            sums.fetch_add(1, std::memory_order_relaxed);  
          });
        }
      });
    });
    //
    
    sp.reset(num_lines, pipes.begin(), pipes.end());

    taskflow.composed_of(sp).name("pipeline");
    executor.run(taskflow).wait();
    REQUIRE(sums == subtask*max_tokens*7);
  }
}


TEST_CASE("ScalablePipeline(SPSPSPSP).Runtime.Subflow.1thread" * doctest::timeout(300)){
  scalable_pipeline_spspspsp_runtime_subflow(1);
}

TEST_CASE("ScalablePipeline(SPSPSPSP).Runtime.Subflow.2threads" * doctest::timeout(300)){
  scalable_pipeline_spspspsp_runtime_subflow(2);
}

TEST_CASE("ScalablePipeline(SPSPSPSP).Runtime.Subflow.3threads" * doctest::timeout(300)){
  scalable_pipeline_spspspsp_runtime_subflow(3);
}

TEST_CASE("ScalablePipeline(SPSPSPSP).Runtime.Subflow.4threads" * doctest::timeout(300)){
  scalable_pipeline_spspspsp_runtime_subflow(4);
}

TEST_CASE("ScalablePipeline(SPSPSPSP).Runtime.Subflow.5threads" * doctest::timeout(300)){
  scalable_pipeline_spspspsp_runtime_subflow(5);
}

TEST_CASE("ScalablePipeline(SPSPSPSP).Runtime.Subflow.6threads" * doctest::timeout(300)){
  scalable_pipeline_spspspsp_runtime_subflow(6);
}

TEST_CASE("ScalablePipeline(SPSPSPSP).Runtime.Subflow.7threads" * doctest::timeout(300)){
  scalable_pipeline_spspspsp_runtime_subflow(7);
}

TEST_CASE("ScalablePipeline(SPSPSPSP).Runtime.Subflow.8threads" * doctest::timeout(300)){
  scalable_pipeline_spspspsp_runtime_subflow(8);
}

