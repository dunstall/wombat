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

cc_library(
    name = "connection-mock",
    srcs = glob(["tests/mock/*.cc"]),
    hdrs = glob(["tests/mock/connection/*.h"]),
    strip_include_prefix = "tests/mock/",
    linkopts = ["-lstdc++fs"],
    deps = [
        "@gtest//:gtest",
    ],
    visibility = ["//visibility:public"],
)

cc_test(
    name = "connection-unit-test",
    srcs = glob(["tests/unit/*.cc"]),
    copts = ["-Iexternal/gtest/include"],
    deps = [
        "@gtest//:gtest_main",
        ":connection",
        ":connection-mock",
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
