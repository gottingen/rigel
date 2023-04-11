#
# Copyright 2023 The rigel Authors.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      https://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

list(APPEND TF_UNITTESTS 
  utility 
  work_stealing 
  #serializer 
  priorities
  basics 
  asyncs
  subflows
  control_flow
  semaphores
  movable
  cancellation
  algorithms
  compositions
  traversals
  sorting
  pipelines
  scalable_pipelines
  deferred_pipelines
  runtimes
  data_pipelines
  workers
)

foreach(unittest IN LISTS TF_UNITTESTS)
  rigel_cc_test(
          NAME
          ${unittest}
          SRCS
          ${unittest}.cc
          COPTS
          #${TURBO_TEST_COPTS}
          ${CMAKE_CXX_FLAGS}
          DEPS
          ${RIGEL_DYLINK}
  )
endforeach()

# include CUDA tests
if(TF_BUILD_CUDA)
  add_subdirectory(${TF_UTEST_DIR}/cuda)
endif()

# include SYCL tests
if(TF_BUILD_SYCL)
  add_subdirectory(${TF_UTEST_DIR}/sycl)
endif()




