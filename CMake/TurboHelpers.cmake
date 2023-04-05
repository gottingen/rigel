#
# Copyright 2020 The Turbo Authors.
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

include(CMakeParseArguments)
include(TurboConfigureCopts)
include(TurboDll)

# The IDE folder for Turbo that will be used if Turbo is included in a CMake
# project that sets
#    set_property(GLOBAL PROPERTY USE_FOLDERS ON)
# For example, Visual Studio supports folders.
if(NOT DEFINED RIGEL_IDE_FOLDER)
  set(RIGEL_IDE_FOLDER Turbo)
endif()

if(RIGEL_USE_SYSTEM_INCLUDES)
  set(RIGEL_INTERNAL_INCLUDE_WARNING_GUARD SYSTEM)
else()
  set(RIGEL_INTERNAL_INCLUDE_WARNING_GUARD "")
endif()

# rigel_cc_library()
#
# CMake function to imitate Bazel's cc_library rule.
#
# Parameters:
# NAME: name of target (see Note)
# HDRS: List of public header files for the library
# SRCS: List of source files for the library
# DEPS: List of other libraries to be linked in to the binary targets
# COPTS: List of private compile options
# DEFINES: List of public defines
# LINKOPTS: List of link options
# PUBLIC: Add this so that this library will be exported under rigel::
# Also in IDE, target will appear in Turbo folder while non PUBLIC will be in Turbo/internal.
# TESTONLY: When added, this target will only be built if both
#           BUILD_TESTING=ON and RIGEL_BUILD_TESTING=ON.
#
# Note:
# By default, rigel_cc_library will always create a library named ${NAME},
# and alias target rigel::${NAME}.  The rigel:: form should always be used.
# This is to reduce namespace pollution.
#
# rigel_cc_library(
#   NAME
#     awesome
#   HDRS
#     "a.h"
#   SRCS
#     "a.cc"
# )
# rigel_cc_library(
#   NAME
#     fantastic_lib
#   SRCS
#     "b.cc"
#   DEPS
#     rigel::awesome # not "awesome" !
#   PUBLIC
# )
#
# rigel_cc_library(
#   NAME
#     main_lib
#   ...
#   DEPS
#     rigel::fantastic_lib
# )
#
# TODO: Implement "ALWAYSLINK"
function(rigel_cc_library)
  cmake_parse_arguments(RIGEL_CC_LIB
    "DISABLE_INSTALL;PUBLIC;TESTONLY"
    "NAME"
    "HDRS;SRCS;COPTS;DEFINES;LINKOPTS;DEPS"
    ${ARGN}
  )

  if(RIGEL_CC_LIB_TESTONLY AND
      NOT ((BUILD_TESTING AND RIGEL_BUILD_TESTING) OR
        (RIGEL_BUILD_TEST_HELPERS AND RIGEL_CC_LIB_PUBLIC)))
    return()
  endif()

  if(RIGEL_ENABLE_INSTALL)
    set(_NAME "${RIGEL_CC_LIB_NAME}")
  else()
    set(_NAME "${RIGEL_CC_LIB_NAME}")
  endif()

  # Check if this is a header-only library
  # Note that as of February 2019, many popular OS's (for example, Ubuntu
  # 16.04 LTS) only come with cmake 3.5 by default.  For this reason, we can't
  # use list(FILTER...)
  set(RIGEL_CC_SRCS "${RIGEL_CC_LIB_SRCS}")
  foreach(src_file IN LISTS RIGEL_CC_SRCS)
    if(${src_file} MATCHES ".*\\.(h|inc)")
      list(REMOVE_ITEM RIGEL_CC_SRCS "${src_file}")
    endif()
  endforeach()

  if(RIGEL_CC_SRCS STREQUAL "")
    set(RIGEL_CC_LIB_IS_INTERFACE 1)
  else()
    set(RIGEL_CC_LIB_IS_INTERFACE 0)
  endif()

  # Determine this build target's relationship to the DLL. It's one of four things:
  # 1. "dll"     -- This target is part of the DLL
  # 2. "dll_dep" -- This target is not part of the DLL, but depends on the DLL.
  #                 Note that we assume any target not in the DLL depends on the
  #                 DLL. This is not a technical necessity but a convenience
  #                 which happens to be true, because nearly every target is
  #                 part of the DLL.
  # 3. "shared"  -- This is a shared library, perhaps on a non-windows platform
  #                 where DLL doesn't make sense.
  # 4. "static"  -- This target does not depend on the DLL and should be built
  #                 statically.
  if (${RIGEL_BUILD_DLL})
    if(RIGEL_ENABLE_INSTALL)
      rigel_internal_dll_contains(TARGET ${_NAME} OUTPUT _in_dll)
    else()
      rigel_internal_dll_contains(TARGET ${RIGEL_CC_LIB_NAME} OUTPUT _in_dll)
    endif()
    if (${_in_dll})
      # This target should be replaced by the DLL
      set(_build_type "dll")
      set(RIGEL_CC_LIB_IS_INTERFACE 1)
    else()
      # Building a DLL, but this target is not part of the DLL
      set(_build_type "dll_dep")
    endif()
  elseif(BUILD_SHARED_LIBS)
    set(_build_type "shared")
  else()
    set(_build_type "static")
  endif()

  # Generate a pkg-config file for every library:
  if(RIGEL_ENABLE_INSTALL)
    if(NOT RIGEL_CC_LIB_TESTONLY)
      if(rigel_VERSION)
        set(PC_VERSION "${rigel_VERSION}")
      else()
        set(PC_VERSION "head")
      endif()
      foreach(dep ${RIGEL_CC_LIB_DEPS})
        if(${dep} MATCHES "^rigel::(.*)")
          # for DLL builds many libs are not created, but add
          # the pkgconfigs nevertheless, pointing to the dll.
          if(_build_type STREQUAL "dll")
            # hide this MATCHES in an if-clause so it doesn't overwrite
            # the CMAKE_MATCH_1 from (${dep} MATCHES "^rigel::(.*)")
            if(NOT PC_DEPS MATCHES "rigel_dll")
              # Join deps with commas.
              if(PC_DEPS)
                set(PC_DEPS "${PC_DEPS},")
              endif()
              # don't duplicate dll-dep if it exists already
              set(PC_DEPS "${PC_DEPS} rigel_dll = ${PC_VERSION}")
              set(LNK_LIB "${LNK_LIB} -labseil_dll")
            endif()
          else()
            # Join deps with commas.
            if(PC_DEPS)
              set(PC_DEPS "${PC_DEPS},")
            endif()
            set(PC_DEPS "${PC_DEPS} ${CMAKE_MATCH_1} = ${PC_VERSION}")
            set(LNK_LIB "${LNK_LIB} -l${_NAME}")
          endif()
        endif()
      endforeach()
      foreach(cflag ${RIGEL_CC_LIB_COPTS})
        if(${cflag} MATCHES "^(-Wno|/wd)")
          # These flags are needed to suppress warnings that might fire in our headers.
          set(PC_CFLAGS "${PC_CFLAGS} ${cflag}")
        elseif(${cflag} MATCHES "^(-W|/w[1234eo])")
          # Don't impose our warnings on others.
        elseif(${cflag} MATCHES "^-m")
          # Don't impose CPU instruction requirements on others, as
          # the code performs feature detection on runtime.
        else()
          set(PC_CFLAGS "${PC_CFLAGS} ${cflag}")
        endif()
      endforeach()
      string(REPLACE ";" " " PC_LINKOPTS "${RIGEL_CC_LIB_LINKOPTS}")
      FILE(GENERATE OUTPUT "${CMAKE_BINARY_DIR}/lib/pkgconfig/${_NAME}.pc" CONTENT "\
prefix=${CMAKE_INSTALL_PREFIX}\n\
exec_prefix=\${prefix}\n\
libdir=${CMAKE_INSTALL_FULL_LIBDIR}\n\
includedir=${CMAKE_INSTALL_FULL_INCLUDEDIR}\n\
\n\
Name: ${_NAME}\n\
Description: Turbo ${_NAME} library\n\
URL: https://abseil.io/\n\
Version: ${PC_VERSION}\n\
Requires:${PC_DEPS}\n\
Libs: -L\${libdir} ${PC_LINKOPTS} $<$<NOT:$<BOOL:${RIGEL_CC_LIB_IS_INTERFACE}>>:${LNK_LIB}>\n\
Cflags: -I\${includedir}${PC_CFLAGS}\n")
      INSTALL(FILES "${CMAKE_BINARY_DIR}/lib/pkgconfig/${_NAME}.pc"
              DESTINATION "${CMAKE_INSTALL_LIBDIR}/pkgconfig")
    endif()
  endif()

  if(NOT RIGEL_CC_LIB_IS_INTERFACE)
    if(_build_type STREQUAL "dll_dep")
      # This target depends on the DLL. When adding dependencies to this target,
      # any depended-on-target which is contained inside the DLL is replaced
      # with a dependency on the DLL.
      add_library(${_NAME} STATIC "")
      target_sources(${_NAME} PRIVATE ${RIGEL_CC_LIB_SRCS} ${RIGEL_CC_LIB_HDRS})
      rigel_internal_dll_targets(
        DEPS ${RIGEL_CC_LIB_DEPS}
        OUTPUT _dll_deps
      )
      target_link_libraries(${_NAME}
        PUBLIC ${_dll_deps}
        PRIVATE
          ${RIGEL_CC_LIB_LINKOPTS}
          ${RIGEL_DEFAULT_LINKOPTS}
      )

      if (RIGEL_CC_LIB_TESTONLY)
        set(_gtest_link_define "GTEST_LINKED_AS_SHARED_LIBRARY=1")
      else()
        set(_gtest_link_define)
      endif()

      target_compile_definitions(${_NAME}
        PUBLIC
          RIGEL_CONSUME_DLL
          "${_gtest_link_define}"
      )

    elseif(_build_type STREQUAL "static" OR _build_type STREQUAL "shared")
      add_library(${_NAME} "")
      target_sources(${_NAME} PRIVATE ${RIGEL_CC_LIB_SRCS} ${RIGEL_CC_LIB_HDRS})
      target_link_libraries(${_NAME}
      PUBLIC ${RIGEL_CC_LIB_DEPS}
      PRIVATE
        ${RIGEL_CC_LIB_LINKOPTS}
        ${RIGEL_DEFAULT_LINKOPTS}
      )
    else()
      message(FATAL_ERROR "Invalid build type: ${_build_type}")
    endif()

    # Linker language can be inferred from sources, but in the case of DLLs we
    # don't have any .cc files so it would be ambiguous. We could set it
    # explicitly only in the case of DLLs but, because "CXX" is always the
    # correct linker language for static or for shared libraries, we set it
    # unconditionally.
    set_property(TARGET ${_NAME} PROPERTY LINKER_LANGUAGE "CXX")

    target_include_directories(${_NAME} ${RIGEL_INTERNAL_INCLUDE_WARNING_GUARD}
      PUBLIC
        "$<BUILD_INTERFACE:${RIGEL_COMMON_INCLUDE_DIRS}>"
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
    )
    target_compile_options(${_NAME}
      PRIVATE ${RIGEL_CC_LIB_COPTS})
    target_compile_definitions(${_NAME} PUBLIC ${RIGEL_CC_LIB_DEFINES})

    # Add all Turbo targets to a a folder in the IDE for organization.
    if(RIGEL_CC_LIB_PUBLIC)
      set_property(TARGET ${_NAME} PROPERTY FOLDER ${RIGEL_IDE_FOLDER})
    elseif(RIGEL_CC_LIB_TESTONLY)
      set_property(TARGET ${_NAME} PROPERTY FOLDER ${RIGEL_IDE_FOLDER}/test)
    else()
      set_property(TARGET ${_NAME} PROPERTY FOLDER ${RIGEL_IDE_FOLDER}/internal)
    endif()

    if(RIGEL_PROPAGATE_CXX_STD)
      # Turbo libraries require C++14 as the current minimum standard. When
      # compiled with C++17 (either because it is the compiler's default or
      # explicitly requested), then Turbo requires C++17.
      target_compile_features(${_NAME} PUBLIC ${RIGEL_INTERNAL_CXX_STD_FEATURE})
    endif()

    # When being installed, we lose the  prefix.  We want to put it back
    # to have properly named lib files.  This is a no-op when we are not being
    # installed.
    if(RIGEL_ENABLE_INSTALL)
      set_target_properties(${_NAME} PROPERTIES
        OUTPUT_NAME "${_NAME}"
        SOVERSION 0
      )
    endif()
  else()
    # Generating header-only library
    add_library(${_NAME} INTERFACE)
    target_include_directories(${_NAME} ${RIGEL_INTERNAL_INCLUDE_WARNING_GUARD}
      INTERFACE
        "$<BUILD_INTERFACE:${RIGEL_COMMON_INCLUDE_DIRS}>"
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
      )

    if (_build_type STREQUAL "dll")
        set(RIGEL_CC_LIB_DEPS rigel_dll)
    endif()

    target_link_libraries(${_NAME}
      INTERFACE
        ${RIGEL_CC_LIB_DEPS}
        ${RIGEL_CC_LIB_LINKOPTS}
        ${RIGEL_DEFAULT_LINKOPTS}
    )
    target_compile_definitions(${_NAME} INTERFACE ${RIGEL_CC_LIB_DEFINES})

    if(RIGEL_PROPAGATE_CXX_STD)
      # Turbo libraries require C++14 as the current minimum standard.
      # Top-level application CMake projects should ensure a consistent C++
      # standard for all compiled sources by setting CMAKE_CXX_STANDARD.
      target_compile_features(${_NAME} INTERFACE ${RIGEL_INTERNAL_CXX_STD_FEATURE})
    endif()
  endif()

  # TODO currently we don't install googletest alongside abseil sources, so
  # installed abseil can't be tested.
  if(NOT RIGEL_CC_LIB_TESTONLY AND RIGEL_ENABLE_INSTALL)
    install(TARGETS ${_NAME} EXPORT ${PROJECT_NAME}Targets
          RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
          LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
          ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    )
  endif()

    add_library(rigel::${RIGEL_CC_LIB_NAME} ALIAS ${_NAME})
endfunction()

# rigel_cc_test()
#
# CMake function to imitate Bazel's cc_test rule.
#
# Parameters:
# NAME: name of target (see Usage below)
# SRCS: List of source files for the binary
# DEPS: List of other libraries to be linked in to the binary targets
# COPTS: List of private compile options
# DEFINES: List of public defines
# LINKOPTS: List of link options
#
# Note:
# By default, rigel_cc_test will always create a binary named ${NAME}.
# This will also add it to ctest list as ${NAME}.
#
# Usage:
# rigel_cc_library(
#   NAME
#     awesome
#   HDRS
#     "a.h"
#   SRCS
#     "a.cc"
#   PUBLIC
# )
#
# rigel_cc_test(
#   NAME
#     awesome_test
#   SRCS
#     "awesome_test.cc"
#   DEPS
#     rigel::awesome
#     GTest::gmock
#     GTest::gtest_main
# )
function(rigel_cc_test)
  if(NOT (BUILD_TESTING AND RIGEL_BUILD_TESTING))
    return()
  endif()

  cmake_parse_arguments(RIGEL_CC_TEST
    ""
    "NAME"
    "SRCS;COPTS;DEFINES;LINKOPTS;DEPS"
    ${ARGN}
  )

  set(_NAME "rigel_${RIGEL_CC_TEST_NAME}")

  add_executable(${_NAME} "")
  target_sources(${_NAME} PRIVATE ${RIGEL_CC_TEST_SRCS})
  target_include_directories(${_NAME}
    PUBLIC ${RIGEL_COMMON_INCLUDE_DIRS}
    PRIVATE ${GMOCK_INCLUDE_DIRS} ${GTEST_INCLUDE_DIRS}
  )

  if (${RIGEL_BUILD_DLL})
    target_compile_definitions(${_NAME}
      PUBLIC
        ${RIGEL_CC_TEST_DEFINES}
        RIGEL_CONSUME_DLL
        GTEST_LINKED_AS_SHARED_LIBRARY=1
    )

    # Replace dependencies on targets inside the DLL with rigel_dll itself.
    rigel_internal_dll_targets(
      DEPS ${RIGEL_CC_TEST_DEPS}
      OUTPUT RIGEL_CC_TEST_DEPS
    )
  else()
    target_compile_definitions(${_NAME}
      PUBLIC
        ${RIGEL_CC_TEST_DEFINES}
    )
  endif()
  target_compile_options(${_NAME}
    PRIVATE ${RIGEL_CC_TEST_COPTS}
  )

  target_link_libraries(${_NAME}
    PUBLIC ${RIGEL_CC_TEST_DEPS}
    PRIVATE ${RIGEL_CC_TEST_LINKOPTS}
  )
  # Add all Turbo targets to a folder in the IDE for organization.
  set_property(TARGET ${_NAME} PROPERTY FOLDER ${RIGEL_IDE_FOLDER}/test)

  if(RIGEL_PROPAGATE_CXX_STD)
    # Turbo libraries require C++14 as the current minimum standard.
    # Top-level application CMake projects should ensure a consistent C++
    # standard for all compiled sources by setting CMAKE_CXX_STANDARD.
    target_compile_features(${_NAME} INTERFACE ${RIGEL_INTERNAL_CXX_STD_FEATURE})
  endif()

  add_test(NAME ${_NAME} COMMAND ${_NAME})
endfunction()
