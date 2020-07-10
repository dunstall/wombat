cc_library(
    name = "server",
    srcs = glob(["src/*.cc"]),
    hdrs = glob(["include/server/*.h"]),
    strip_include_prefix = "include/",
    deps = [
        "@glog//:glog",
        "//event:event",
        "//record:record",
        "//util:util",
    ],
    visibility = ["//visibility:public"],
)
