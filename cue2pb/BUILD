load("@rules_proto//proto:defs.bzl", "proto_library")

package(default_visibility = ["//cue2pb:internal"])

package_group(
    name = "internal",
    packages = ["//cue2pb/..."],
)

cc_proto_library(
    name = "cuesheet_cc_proto",
    visibility = ["//visibility:public"],
    deps = [":cuesheet_proto"],
)

proto_library(
    name = "cuesheet_proto",
    srcs = ["cuesheet.proto"],
    visibility = ["//visibility:public"],
)

cc_library(
    name = "text_format",
    srcs = ["text_format.cc"],
    hdrs = ["text_format.h"],
    deps = [
        ":cuesheet_cc_proto",
        "@com_google_absl//absl/strings:str_format",
        "@com_google_absl//absl/status:statusor",
        "@com_google_protobuf//:protobuf",
    ],
)

cc_library(
    name = "parser",
    srcs = ["parser.cc"],
    hdrs = ["parser.h"],
    visibility = ["//visibility:public"],
    deps = [
        ":cuesheet_cc_proto",
        "@com_google_absl//absl/algorithm:container",
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/strings:str_format",
        "@com_google_absl//absl/status:statusor",
        "@com_google_absl//absl/status:status",
        "//util:status_builder",
        "//util:status_macros",
    ],
)

cc_test(
    name = "parser_test",
    srcs = ["parser_test.cc"],
    data = glob([
        "testdata/*.cue",
        "testdata/*.textproto",
    ]),
    deps = [
        ":parser",
        ":text_format",
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/status:statusor",
        "@com_google_googletest//:gtest_main",
        "//util:file",
        "//util/testing:assertions",
        "//util:status_macros",
        "//util/testing:protobuf_assertions",
    ],
)

cc_library(
    name = "unparser",
    srcs = ["unparser.cc"],
    hdrs = ["unparser.h"],
    visibility = ["//visibility:public"],
    deps = [
        ":cuesheet_cc_proto",
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/strings:str_format",
        "@com_google_absl//absl/status",
        "@com_google_absl//absl/status:statusor",
        "//util:status_builder",
        "//util:status_macros",
    ],
)

cc_test(
    name = "unparser_test",
    srcs = ["unparser_test.cc"],
    data = glob([
        "testdata/*.unparsed_cue",
        "testdata/*.textproto",
    ]),
    deps = [
        ":text_format",
        ":unparser",
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/status:statusor",
        "@com_google_googletest//:gtest_main",
        "//util:errno",
        "//util:file",
        "//util:status_macros",
        "//util/testing:assertions",
        "//util/testing:protobuf_assertions",
    ],
)

cc_binary(
    name = "cue2pb",
    srcs = ["main.cc"],
    deps = [
        ":cuesheet_cc_proto",
        ":parser",
        ":text_format",
        ":unparser",
        "@com_google_absl//absl/flags:parse",
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/types:span",
        "@com_google_absl//absl/status",
        "@com_google_absl//absl/debugging:symbolize",
        "@com_google_absl//absl/debugging:failure_signal_handler",
        "@com_google_protobuf//:protobuf",
        "//util:file",
        "//util:status_macros",
    ],
)
