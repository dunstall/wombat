cc_library(
    name = "log",
    srcs = glob(["src/*.cc"]),
    hdrs = glob(["include/log/*.h"]),
    strip_include_prefix = "include/",
    linkopts = ["-lstdc++fs"],
    deps = [
        "@glog//:glog",
    ],
    visibility = ["//visibility:public"],
)

cc_test(
    name = "log-test",
    srcs = glob(["tests/unit/*.cc"]),
    copts = ["-Iexternal/gtest/include"],
    deps = [
        "@gtest//:gtest_main",
        ":log"
    ],
)
