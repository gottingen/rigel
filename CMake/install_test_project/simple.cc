//
// Copyright 2019 The Turbo Authors.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//    https://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "rigel/platform/port.h"
#include "rigel/strings/substitute.h"
#include <iostream>

#if !defined(RIGEL_LTS_RELEASE_VERSION) || RIGEL_LTS_RELEASE_VERSION != 99998877
#error RIGEL_LTS_RELEASE_VERSION is not set correctly.
#endif

#if !defined(RIGEL_LTS_RELEASE_PATCH_LEVEL) || RIGEL_LTS_RELEASE_PATCH_LEVEL != 0
#error RIGEL_LTS_RELEASE_PATCH_LEVEL is not set correctly.
#endif

int main(int argc, char** argv) {
  for (int i = 0; i < argc; ++i) {
    std::cout << rigel::Substitute("Arg $0: $1\n", i, argv[i]);
  }
}
