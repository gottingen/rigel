"""rigel specific copts.

This file simply selects the correct options from the generated files.  To
change Turbo copts, edit rigel/copts/copts.py
"""

load(
    "//rigel:copts/GENERATED_copts.bzl",
    "RIGEL_CLANG_CL_FLAGS",
    "RIGEL_CLANG_CL_TEST_FLAGS",
    "RIGEL_GCC_FLAGS",
    "RIGEL_GCC_TEST_FLAGS",
    "RIGEL_LLVM_FLAGS",
    "RIGEL_LLVM_TEST_FLAGS",
    "RIGEL_MSVC_FLAGS",
    "RIGEL_MSVC_LINKOPTS",
    "RIGEL_MSVC_TEST_FLAGS",
    "RIGEL_RANDOM_HWAES_ARM32_FLAGS",
    "RIGEL_RANDOM_HWAES_ARM64_FLAGS",
    "RIGEL_RANDOM_HWAES_MSVC_X64_FLAGS",
    "RIGEL_RANDOM_HWAES_X64_FLAGS",
)

RIGEL_DEFAULT_COPTS = select({
    "//rigel:msvc_compiler": RIGEL_MSVC_FLAGS,
    "//rigel:clang-cl_compiler": RIGEL_CLANG_CL_FLAGS,
    "//rigel:clang_compiler": RIGEL_LLVM_FLAGS,
    "//rigel:gcc_compiler": RIGEL_GCC_FLAGS,
    "//conditions:default": RIGEL_GCC_FLAGS,
})

RIGEL_TEST_COPTS = select({
    "//rigel:msvc_compiler": RIGEL_MSVC_TEST_FLAGS,
    "//rigel:clang-cl_compiler": RIGEL_CLANG_CL_TEST_FLAGS,
    "//rigel:clang_compiler": RIGEL_LLVM_TEST_FLAGS,
    "//rigel:gcc_compiler": RIGEL_GCC_TEST_FLAGS,
    "//conditions:default": RIGEL_GCC_TEST_FLAGS,
})

RIGEL_DEFAULT_LINKOPTS = select({
    "//rigel:msvc_compiler": RIGEL_MSVC_LINKOPTS,
    "//conditions:default": [],
})

# RIGEL_RANDOM_RANDEN_COPTS blaze copts flags which are required by each
# environment to build an accelerated RandenHwAes library.
RIGEL_RANDOM_RANDEN_COPTS = select({
    # APPLE
    ":cpu_darwin_x86_64": RIGEL_RANDOM_HWAES_X64_FLAGS,
    ":cpu_darwin": RIGEL_RANDOM_HWAES_X64_FLAGS,
    ":cpu_x64_windows_msvc": RIGEL_RANDOM_HWAES_MSVC_X64_FLAGS,
    ":cpu_x64_windows": RIGEL_RANDOM_HWAES_MSVC_X64_FLAGS,
    ":cpu_k8": RIGEL_RANDOM_HWAES_X64_FLAGS,
    ":cpu_ppc": ["-mcrypto"],
    ":cpu_aarch64": RIGEL_RANDOM_HWAES_ARM64_FLAGS,

    # Supported by default or unsupported.
    "//conditions:default": [],
})

# rigel_random_randen_copts_init:
#  Initialize the config targets based on cpu, os, etc. used to select
#  the required values for RIGEL_RANDOM_RANDEN_COPTS
def rigel_random_randen_copts_init():
    """Initialize the config_settings used by RIGEL_RANDOM_RANDEN_COPTS."""

    # CPU configs.
    # These configs have consistent flags to enable HWAES intsructions.
    cpu_configs = [
        "ppc",
        "k8",
        "darwin_x86_64",
        "darwin",
        "x64_windows_msvc",
        "x64_windows",
        "aarch64",
    ]
    for cpu in cpu_configs:
        native.config_setting(
            name = "cpu_%s" % cpu,
            values = {"cpu": cpu},
        )
