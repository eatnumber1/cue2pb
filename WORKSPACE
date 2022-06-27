workspace(name = "cue2pb")

load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

http_archive(
  name = "rules_cc",
  urls = ["https://github.com/bazelbuild/rules_cc/archive/262ebec3c2296296526740db4aefce68c80de7fa.zip"],
  strip_prefix = "rules_cc-262ebec3c2296296526740db4aefce68c80de7fa",
)

http_archive(
  name = "com_google_googletest",
  urls = ["https://github.com/google/googletest/archive/011959aafddcd30611003de96cfd8d7a7685c700.zip"],
  strip_prefix = "googletest-011959aafddcd30611003de96cfd8d7a7685c700",
  sha256 = "6a5d7d63cd6e0ad2a7130471105a3b83799a7a2b14ef7ec8d742b54f01a4833c",
)

http_archive(
  name = "com_google_absl",
  urls = ["https://github.com/abseil/abseil-cpp/archive/1ae9b71c474628d60eb251a3f62967fe64151bb2.zip"],
  strip_prefix = "abseil-cpp-1ae9b71c474628d60eb251a3f62967fe64151bb2",
  sha256 = "dd6d0efe513a110f3e6a6bba5f2197e6b637287a1c8409a21946818cec85aaaf",
)

http_archive(
  name = "com_google_protobuf",
  strip_prefix = "protobuf-21.1",
  urls = ["https://github.com/protocolbuffers/protobuf/archive/refs/tags/v21.1.zip"],
  sha256 = "6a2a9b6d00e9a5a28d251f4dc5f2a7e72684a699206ec286ea02ac0b02c22e77",
)
load("@com_google_protobuf//:protobuf_deps.bzl", "protobuf_deps")
protobuf_deps()
