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

file(GLOB_RECURSE RIGEL_METRICS_SRC
        "metrics/*.cc")

file(GLOB_RECURSE RIGEL_CORE_SRC "core/*.cc")
file(GLOB_RECURSE RIGEL_COROUTINE_SRC "coroutine/*.cc")
file(GLOB_RECURSE RIGEL_HTTP_SRC "http/*.cc")
file(GLOB_RECURSE RIGEL_JSON_SRC "json/*.cc")
file(GLOB_RECURSE RIGEL_NET_SRC "net/*.cc")
file(GLOB_RECURSE RIGEL_UTIL_SRC "util/*.cc")
file(GLOB_RECURSE RIGEL_WEB_SRC "websocket/*.cc")

set(RIGEL_ALL_SRC
        ${RIGEL_CORE_SRC}
        ${RIGEL_COROUTINE_SRC}
        ${RIGEL_HTTP_SRC}
        ${RIGEL_JSON_SRC}
        ${RIGEL_NET_SRC}
        ${RIGEL_UTIL_SRC}
        ${RIGEL_WEB_SRC}
        )

rigel_cc_library(
        NAME
        rigel
        SRCS
        ${RIGEL_ALL_SRC}
        COPTS
        ${RIGEL_DEFAULT_COPTS}
        DEPS
        ${RIGEL_DYLINK}
)

if(RIGEL_BUILD_TESTING)
    add_subdirectory(workflow)
endif()
