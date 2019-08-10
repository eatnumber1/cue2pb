#include "cue2pb/unparser.h"

#include <string>
#include <utility>
#include <fstream>
#include <sstream>
#include <cerrno>
#include <cstring>

#include "gtest/gtest.h"
#include "cue2pb/file.h"
#include "cue2pb/text_format.h"
#include "cue2pb/errors.h"
#include "cue2pb/testing/assertions.h"
#include "absl/types/optional.h"
#include "absl/strings/string_view.h"

namespace cue2pb {
namespace {

std::string TestdataToPath(absl::string_view filename) {
  return "cue2pb/testdata/" + std::string(filename);
}

std::ifstream OpenTestdataOrDie(absl::string_view filename) {
  GError *err = nullptr;
  std::ifstream istrm = OpenInputFile(TestdataToPath(filename), &err);
  CHECK_OK(err);
  return istrm;
}

absl::optional<Cuesheet> CuesheetFromProtoString(absl::string_view textproto,
                                                 GError **error) {
  std::istringstream istrm{std::string(textproto)};
  return CuesheetFromTextProto(&istrm, error);
}

Cuesheet CuesheetFromProtoStringOrDie(absl::string_view textproto) {
  GError *err = nullptr;
  auto cuesheet = CuesheetFromProtoString(textproto, &err);
  CHECK_OK(err);

  return std::move(*cuesheet);
}

Cuesheet CuesheetFromProtoFileOrDie(absl::string_view filename) {
  std::ifstream istrm = OpenTestdataOrDie(filename);

  GError *err = nullptr;
  auto cuesheet = CuesheetFromTextProto(&istrm, &err);
  CHECK_OK(err);

  return std::move(*cuesheet);
}

std::string ReadFileEntirelyOrDie(std::string filename) {
  std::ifstream in = OpenTestdataOrDie(filename);
  std::stringstream sstr;
  sstr << in.rdbuf();
  if (in.fail()) {
    GError *err = nullptr;
    SetError(&err, cue2pb::ERR_OS, "Failed to open '%s': %s", filename,
             std::strerror(errno));
    CHECK_OK(err);
  }
  return std::move(sstr).str();
}

absl::optional<std::string> UnparseCuesheet(Cuesheet cuesheet, GError **error) {
  std::stringstream out;
  if (!UnparseCuesheet(cuesheet, &out, error)) return absl::nullopt;
  return out.str();
}

struct CuesheetProtoFiles {
  std::string proto;
  std::string cuesheet;
};

class CuesheetEqualsProtoFilesTest :
    public testing::TestWithParam<CuesheetProtoFiles> {};

struct CuesheetProtoSample {
  std::string cuesheet;
  std::string proto;
};

class CuesheetEqualsProtoTest :
  public testing::TestWithParam<CuesheetProtoSample> {};

TEST_P(CuesheetEqualsProtoFilesTest, MatchFiles) {
  auto files = GetParam();

  std::string expected = ReadFileEntirelyOrDie(files.cuesheet);
  Cuesheet incue = CuesheetFromProtoFileOrDie(files.proto);

  GError *err = nullptr;
  auto found = UnparseCuesheet(incue, &err);
  ASSERT_TRUE(IsOk(err));

  EXPECT_EQ(expected, *found);
}

TEST_P(CuesheetEqualsProtoTest, MatchSample) {
  const auto &p = GetParam();
  const std::string &expected = p.cuesheet;
  const std::string &proto = p.proto;

  Cuesheet incue = CuesheetFromProtoStringOrDie(proto);

  GError *err = nullptr;
  auto found = UnparseCuesheet(incue, &err);
  ASSERT_TRUE(IsOk(err));

  EXPECT_EQ(expected, *found);
}

INSTANTIATE_TEST_SUITE_P(
    Examples,
    CuesheetEqualsProtoFilesTest,
    testing::Values(
      CuesheetProtoFiles{"eac_multifile_gapless.textproto",
                         "eac_multifile_gapless.unparsed_cue"},
      CuesheetProtoFiles{"eac_multifile_gaps.textproto",
                         "eac_multifile_gaps.unparsed_cue"},
      CuesheetProtoFiles{"eac_singlefile.textproto",
                         "eac_singlefile.unparsed_cue"},
      CuesheetProtoFiles{"hidden_track.textproto", "hidden_track.unparsed_cue"},
      CuesheetProtoFiles{"full_disc.textproto", "full_disc.unparsed_cue"}));

INSTANTIATE_TEST_SUITE_P(
    CornerCases,
    CuesheetEqualsProtoTest,
    testing::Values(
      CuesheetProtoSample{"", ""}
    )
);

INSTANTIATE_TEST_SUITE_P(
    Quoting,
    CuesheetEqualsProtoTest,
    testing::Values(
      CuesheetProtoSample{
        "REM FOO \"Bar\n",
        R"""(tags { comment_tag { name: "FOO" value: "\"Bar" } })"""
      },
      CuesheetProtoSample{
        "REM FOO \"Foo Bar\"\n",
        R"""(tags { comment_tag { name: "FOO" value: "Foo Bar" } })"""
      },
      CuesheetProtoSample{
        "PERFORMER \"The Specials\"\n",
        R"""(tags { performer: "The Specials" })"""
      },
      CuesheetProtoSample{
        "FILE \"\" WAVE\n",
        R"""(file { path: "" type: TYPE_WAVE })"""
      }
    )
);

}  // namespace
}  // namespace cue2pb
