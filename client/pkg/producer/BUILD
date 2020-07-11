load("@io_bazel_rules_go//go:def.bzl", "go_library")

go_library(
    name = "producer",
    importpath = "github.com/dunstall/wombat/pkg/producer",
    srcs = glob(["*.go"]),
    deps = ["//client/pkg/record:record"],
    visibility = ["//visibility:public"],
)
