cc_library(
    name = "broker",
    srcs = glob(["src/*.cc"], exclude = ["src/main.cc"],),
    hdrs = glob(["include/broker/*.h"]),
    strip_include_prefix = "include/",
    deps = ["//record:record", "//util:util", "//log:log", "//server:server"],
)

cc_binary(
    name = "wombat-broker",
    srcs = ["src/main.cc"],
    deps = [
        ":broker",
        "@glog//:glog",
    ],
    linkopts = ["-pthread"],
)

cc_test(
    name = "broker-test",
    srcs = glob(["tests/unit/*.cc"]),
    copts = ["-Iexternal/gtest/include"],
    deps = [
        "@gtest//:gtest_main",
        ":broker"
    ],
)
