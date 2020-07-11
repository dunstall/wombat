load("@io_bazel_rules_go//go:def.bzl", "go_library")
load("@io_bazel_rules_go//go:def.bzl", "go_test")

go_library(
    name = "record",
    importpath = "github.com/dunstall/wombat/pkg/record",
    srcs = glob(["*.go"]),
    visibility = ["//visibility:public"],
)

go_test (
    name = "record-test",
    srcs = glob(["*.go"]),
)
