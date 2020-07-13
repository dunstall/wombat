cc_library(
    name = "util",
    srcs = glob(["src/*.cc"]),
    hdrs = glob(["include/util/*.h"]),
    strip_include_prefix = "include/",
    visibility = ["//visibility:public"],
)

cc_test(
    name = "util-test",
    srcs = glob(["tests/unit/*.cc"]),
    copts = ["-Iexternal/gtest/include"],
    deps = [
        "@gtest//:gtest_main",
        ":util"
    ],
)
