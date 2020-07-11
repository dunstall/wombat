load("@io_bazel_rules_go//go:def.bzl", "go_library")

go_library(
    name = "consumer",
    importpath = "github.com/dunstall/wombat/pkg/consumer",
    srcs = glob(["*.go"]),
    deps = ["//client/pkg/record:record"],
    visibility = ["//visibility:public"],
)
