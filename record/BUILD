cc_library(
    name = "record",
    srcs = glob(["src/*.cpp"]),
    hdrs = glob(["include/record/*.h"]),
    strip_include_prefix = "include/",
    visibility = ["//visibility:public"],
)

cc_test(
    name = "record-test",
    srcs = glob(["tests/unit/*.cpp"]),
    copts = ["-Iexternal/gtest/include"],
    deps = [
        "@gtest//:gtest_main",
        ":record"
    ],
)
