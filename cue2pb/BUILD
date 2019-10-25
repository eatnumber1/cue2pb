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
    name = "file",
    srcs = ["file.cc"],
    hdrs = ["file.h"],
    deps = [
        "@rhutil//rhutil:status",
        "@rhutil//rhutil:errno",
        "@abseil//absl/strings",
        "@abseil//absl/strings:str_format",
    ],
)

cc_library(
    name = "text_format",
    srcs = ["text_format.cc"],
    hdrs = ["text_format.h"],
    deps = [
        ":cuesheet_cc_proto",
        "@rhutil//rhutil:status",
        "@com_google_protobuf//:protobuf",
        "@abseil//absl/strings:str_format",
    ],
)

cc_library(
    name = "parser",
    visibility = ["//visibility:public"],
    srcs = ["parser.cc"],
    hdrs = ["parser.h"],
    deps = [
        ":cuesheet_cc_proto",
        "@rhutil//rhutil:status",
        "@abseil//absl/strings",
        "@abseil//absl/strings:str_format",
        "@abseil//absl/algorithm:container",
    ],
)

cc_test(
    name = "parser_test",
    srcs = ["parser_test.cc"],
    deps = [
        ":parser",
        ":file",
        ":text_format",
        "//cue2pb/testing:assertions",
        "@rhutil//rhutil/testing:assertions",
        "@googletest//:gtest_main",
        "@abseil//absl/strings",
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
        "@rhutil//rhutil:status",
        ":cuesheet_cc_proto",
        "@abseil//absl/strings",
        "@abseil//absl/strings:str_format",
    ],
)

cc_test(
    name = "unparser_test",
    srcs = ["unparser_test.cc"],
    deps = [
        ":unparser",
        ":file",
        ":text_format",
        "//cue2pb/testing:assertions",
        "@rhutil//rhutil/testing:assertions",
        "@rhutil//rhutil:errno",
        "@rhutil//rhutil:status",
        "@googletest//:gtest_main",
        "@abseil//absl/strings",
    ],
    data = glob([
        "testdata/*.unparsed_cue",
        "testdata/*.textproto"
    ]),
)

cc_binary(
    name = "cue2pb",
    srcs = ["main.cc"],
    deps = [
        ":parser",
        ":unparser",
        ":text_format",
        ":file",
        ":cuesheet_cc_proto",
        "@abseil//absl/strings",
        "@abseil//absl/types:span",
        "@abseil//absl/flags:parse",
        "@com_google_protobuf//:protobuf",
        "@rhutil//rhutil:status",
    ],
)
