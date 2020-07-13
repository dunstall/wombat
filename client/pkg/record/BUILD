load("@io_bazel_rules_go//go:def.bzl", "go_library")
load("@io_bazel_rules_go//go:def.bzl", "go_test")

go_library(
    name = "record",
    srcs = glob(["*.go"]),
    importpath = "github.com/dunstall/wombat/pkg/record",
    visibility = ["//visibility:public"],
)

go_test(
    name = "record-test",
    srcs = glob(["*.go"]),
)
