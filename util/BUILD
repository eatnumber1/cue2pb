cc_library(
    name = "status_builder",
    visibility = ["//visibility:public"],
    srcs = ["status_builder.cc"],
    hdrs = ["status_builder.h"],
    deps = [
        "@com_google_absl//absl/strings:str_format",
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/strings:cord",
        "@com_google_absl//absl/status",
        "@com_google_absl//absl/base:core_headers",
    ],
)

cc_library(
    name = "status_macros",
    visibility = ["//visibility:public"],
    hdrs = ["status_macros.h"],
    srcs = ["status_macros.cc"],
    deps = [
        ":status_builder",
        "@com_google_absl//absl/status",
        "@com_google_absl//absl/base:core_headers",
    ],
)

cc_test(
    name = "status_builder_test",
    srcs = ["status_builder_test.cc"],
    deps = [
        ":status_builder",
        "@com_google_absl//absl/status",
        "@com_google_googletest//:gtest_main",
    ],
)

cc_library(
    name = "errno",
    visibility = ["//visibility:public"],
    srcs = ["errno.cc"],
    hdrs = ["errno.h"],
    deps = [
        ":status_builder",
        "@com_google_absl//absl/status",
    ],
)

cc_library(
    name = "file",
    visibility = ["//visibility:public"],
    srcs = ["file.cc"],
    hdrs = ["file.h"],
    deps = [
        ":errno",
        ":status_builder",
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/status:statusor",
        "@com_google_absl//absl/strings:str_format",
    ],
)
