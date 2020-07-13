cc_library(
    name = "server",
    srcs = glob(["src/*.cc"]),
    hdrs = glob(["include/server/*.h"]),
    strip_include_prefix = "include/",
    deps = [
        "@glog//:glog",
        "//event:event",
        "//frame:frame",
        "//util:util",
    ],
    visibility = ["//visibility:public"],
)

cc_test(
    name = "server-integration-test",
    srcs = glob(["tests/integration/*.cc"]),
    copts = ["-Iexternal/gtest/include"],
    deps = [
        "@gtest//:gtest_main",
        ":server"
    ],
)
