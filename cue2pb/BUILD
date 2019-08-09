package(default_visibility = ["//cue2pb:internal"])

package_group(
    name = "internal",
    packages = ["//cue2pb/..."],
)

cc_proto_library(
    name = "cuesheet_cc_proto",
    deps = [":cuesheet_proto"],
    visibility = ["//visibility:public"],
)

proto_library(
    name = "cuesheet_proto",
    srcs = ["cuesheet.proto"],
    visibility = ["//visibility:public"],
)

cc_library(
    name = "errors",
    srcs = ["errors.cc"],
    hdrs = ["errors.h"],
    deps = [
        "@glib//:glib",
        "@abseil//absl/strings:str_format",
        "@com_google_protobuf//:protobuf",
    ],
)

cc_library(
    name = "file",
    srcs = ["file.cc"],
    hdrs = ["file.h"],
    deps = [
        ":errors",
        "@abseil//absl/strings",
    ],
)

cc_library(
    name = "text_format",
    srcs = ["text_format.cc"],
    hdrs = ["text_format.h"],
    deps = [
        ":cuesheet_cc_proto",
        ":errors",
        "@abseil//absl/types:optional",
        "@com_google_protobuf//:protobuf",
    ],
)

cc_library(
    name = "parser",
    visibility = ["//visibility:public"],
    srcs = ["parser.cc"],
    hdrs = ["parser.h"],
    deps = [
        ":cuesheet_cc_proto",
        ":errors",
        "@abseil//absl/strings",
        "@abseil//absl/types:optional",
        "@abseil//absl/algorithm:container",
    ],
)

cc_test(
    name = "parser_test",
    srcs = ["parser_test.cc"],
    deps = [
        ":parser",
        ":file",
        ":errors",
        ":text_format",
        "@googletest//:gtest_main",
        "@com_google_protobuf//:protobuf",
    ],
    data = glob([
        "testdata/*.cue",
        "testdata/*.textproto"
    ]),
)

cc_library(
    name = "unparser",
    visibility = ["//visibility:public"],
    srcs = ["unparser.cc"],
    hdrs = ["unparser.h"],
    deps = [
        ":cuesheet_cc_proto",
        ":errors",
        "@abseil//absl/strings",
        "@abseil//absl/strings:str_format",
        "@abseil//absl/types:optional",
    ],
)

cc_binary(
    name = "cue2pb",
    srcs = ["main.cc"],
    deps = [
        ":errors",
        ":parser",
        ":unparser",
        ":text_format",
        ":file",
        ":cuesheet_cc_proto",
        "@abseil//absl/strings",
        "@abseil//absl/types:span",
        "@abseil//absl/flags:parse",
        "@com_google_protobuf//:protobuf",
    ],
)
