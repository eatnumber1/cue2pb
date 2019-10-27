workspace(name = "cue2pb")

load("@bazel_tools//tools/build_defs/repo:git.bzl", "git_repository")
load("@bazel_tools//tools/build_defs/repo:git.bzl", "new_git_repository")
load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

http_archive(
    name = "bazel_skylib",
    type = "tar.gz",
    url = "https://github.com/bazelbuild/bazel-skylib/releases/download/0.8.0/bazel-skylib.0.8.0.tar.gz",
    sha256 = "2ef429f5d7ce7111263289644d233707dba35e39696377ebab8b0bc701f7818e",
)

git_repository(
  name = "googletest",
  remote = "https://github.com/google/googletest.git",
  commit = "90a443f9c2437ca8a682a1ac625eba64e1d74a8a",
  shallow_since = "1565193450 -0400",
)

http_archive(
    name = "abseil",
    sha256 = "0b62fc2d00c2b2bc3761a892a17ac3b8af3578bd28535d90b4c914b0a7460d4e",
    strip_prefix = "abseil-cpp-20190808",
    urls = ["https://github.com/abseil/abseil-cpp/archive/20190808.zip"],
)

http_archive(
    name = "com_google_protobuf",
    sha256 = "b4fdd8e3733cd88dbe71ebbf553d16e536ff0d5eb1fdba689b8fc7821f65878a",
    strip_prefix = "protobuf-3.9.1",
    urls = ["https://github.com/protocolbuffers/protobuf/releases/download/v3.9.1/protobuf-cpp-3.9.1.zip"],
)
load("@com_google_protobuf//:protobuf_deps.bzl", "protobuf_deps")
protobuf_deps()

git_repository(
    name = "rhutil",
    remote = "https://github.com/eatnumber1/rhutil.git",
    commit = "51a3e7ce251e83c3c8b3d0d610c4ff71d46e3e70",
    shallow_since = "1572160388 -0700"
)
