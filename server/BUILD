cc_library(
    name = "server",
    srcs = glob(["src/*.cc"]),
    hdrs = glob(["include/server/*.h"]),
    strip_include_prefix = "include/",
    deps = ["//record:record", "//util:util", "@glog//:glog"],
    visibility = ["//visibility:public"],
)
