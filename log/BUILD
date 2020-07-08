cc_library(
    name = "log",
    srcs = glob(["src/*.cpp"]),
    hdrs = glob(["include/log/*.h"]),
    strip_include_prefix = "include/",
    linkopts = ["-lstdc++fs"],
    visibility = ["//visibility:public"],
)

cc_test(
    name = "log-test",
    srcs = glob(["tests/unit/*.cpp"]),
    copts = ["-Iexternal/gtest/include"],
    linkopts = ["-L/usr/local/lib -lglog"],
    deps = [
        "@gtest//:gtest_main",
        ":log"
    ],
)
