cc_library(
    name = "broker",
    srcs = glob(["src/*.cpp"], exclude = ["src/main.cpp"],),
    hdrs = glob(["include/broker/*.h"]),
    strip_include_prefix = "include/",
    deps = ["//record:record", "//util:util", "//log:log", "//server:server"],
)

cc_binary(
    name = "wombat-broker",
    srcs = ["src/main.cpp"],
    deps = [
        ":broker",
    ],
    linkopts = ["-pthread -L/usr/local/lib -lglog"],
)
