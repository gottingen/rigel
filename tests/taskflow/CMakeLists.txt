#
# Copyright 2023 The titan-search Authors.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#    https://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

list(APPEND TF_UNITTESTS 
  test_utility 
  test_work_stealing 
  #test_serializer 
  test_priorities
  test_basics 
  test_asyncs
  test_dependent_asyncs
  test_subflows
  test_control_flow
  test_semaphores
  test_movable
  test_cancellation
  test_for_each
  test_reduce
  test_transform
  test_sort
  test_scan
  test_find
  test_compositions
  test_traversals
  test_pipelines
  test_scalable_pipelines
  test_deferred_pipelines
  test_deferred_scalable_pipelines
  test_runtimes
  test_data_pipelines
  test_workers
)
include_directories(${PROJECT_SOURCE_DIR}/tests)

foreach(unittest IN LISTS TF_UNITTESTS)
  carbin_cc_test(
          NAME ${unittest}
          SOURCES ${unittest}.cc
          DEPS rigel::taskflow ${CARBIN_DEPS_LINK} ${GTEST_LIB} ${GTEST_MAIN_LIB}
          COPTS ${USER_CXX_FLAGS}
  )
endforeach()




