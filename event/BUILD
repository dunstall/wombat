cc_library(
    name = "event",
    srcs = glob(["src/*.cc"]),
    hdrs = glob(["include/event/*.h"]),
    strip_include_prefix = "include/",
    linkopts = ["-lstdc++fs"],
    deps = [
        "//frame:frame",
        "//util:util",
    ],
    visibility = ["//visibility:public"],
)

cc_test(
    name = "event-test",
    srcs = glob(["tests/unit/*.cc"]),
    copts = ["-Iexternal/gtest/include"],
    deps = [
        "@gtest//:gtest_main",
        ":event"
    ],
)
