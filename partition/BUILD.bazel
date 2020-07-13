cc_library(
    name = "partition",
    srcs = glob(["src/*.cc"]),
    hdrs = glob(["include/partition/*.h"]),
    strip_include_prefix = "include/",
    linkopts = ["-lstdc++fs"],
    deps = [
        "@glog//:glog",
        "//frame:frame",
        "//event:event",
        "//log:log",
        "//util:util",
    ],
    visibility = ["//visibility:public"],
)

cc_test(
    name = "partition-test",
    srcs = glob(["tests/unit/*.cc"]),
    copts = ["-Iexternal/gtest/include"],
    deps = [
        "@gtest//:gtest_main",
        ":partition"
    ],
)
