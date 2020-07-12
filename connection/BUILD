cc_library(
    name = "connection",
    srcs = glob(["src/*.cc"]),
    hdrs = glob(["include/connection/*.h"]),
    strip_include_prefix = "include/",
    linkopts = ["-lstdc++fs"],
    deps = [
        "@glog//:glog",
        "//frame:frame"
    ],
    visibility = ["//visibility:public"],
)

cc_test(
    name = "connection-unit-test",
    srcs = glob(["tests/unit/*.cc"]),
    copts = ["-Iexternal/gtest/include"],
    deps = [
        "@gtest//:gtest_main",
        ":connection"
    ],
)

cc_test(
    name = "connection-integration-test",
    srcs = glob(["tests/integration/*.cc"]),
    copts = ["-Iexternal/gtest/include"],
    deps = [
        "@gtest//:gtest_main",
        ":connection"
    ],
)
