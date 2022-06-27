package(default_testonly = 1)

cc_library(
    name = "assertions",
    hdrs = ["assertions.h"],
    srcs = ["assertions.cc"],
    deps = [
        "@com_google_absl//absl/status",
        "@com_google_absl//absl/status:statusor",
        "@com_google_googletest//:gtest"
    ],
    visibility = ["//visibility:public"],
)

cc_library(
    name = "protobuf_assertions",
    hdrs = ["protobuf_assertions.h"],
    srcs = ["protobuf_assertions.cc"],
    deps = [
        "@com_google_googletest//:gtest",
        "@com_google_protobuf//:protobuf",
    ],
    visibility = ["//visibility:public"],
)
