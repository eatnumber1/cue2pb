package(
    default_visibility = ["//cue2pb:internal"],
    default_testonly = 1,
)

cc_library(
    name = "assertions",
    hdrs = ["assertions.h"],
    srcs = ["assertions.cc"],
    deps = [
        "//cue2pb:errors",
        "@googletest//:gtest",
        "@com_google_protobuf//:protobuf",
    ],
)
