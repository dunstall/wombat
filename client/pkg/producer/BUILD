load("@io_bazel_rules_go//go:def.bzl", "go_library")

go_library(
    name = "producer",
    srcs = glob(["*.go"]),
    importpath = "github.com/dunstall/wombat/pkg/producer",
    visibility = ["//visibility:public"],
    deps = ["//client/pkg/record"],
)
