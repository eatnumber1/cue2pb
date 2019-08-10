#include "cue2pb/parser.h"

#include <string>
#include <utility>
#include <fstream>
#include <sstream>

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

Cuesheet CuesheetFromProtoFileOrDie(absl::string_view filename) {
  std::ifstream istrm = OpenTestdataOrDie(filename);

  GError *err = nullptr;
  auto cuesheet = CuesheetFromTextProto(&istrm, &err);
  CHECK_OK(err);

  return std::move(*cuesheet);
}

Cuesheet CuesheetFromProtoStringOrDie(absl::string_view textproto) {
  std::istringstream istrm{std::string(textproto)};
  GError *err = nullptr;
  auto cuesheet = CuesheetFromTextProto(&istrm, &err);
  CHECK_OK(err);

  return std::move(*cuesheet);
}

absl::optional<Cuesheet> ParseCuesheetFromString(absl::string_view s,
                                                 GError **error) {
  std::istringstream istrm{std::string(s)};
  return ParseCuesheet(&istrm, error);
}

struct CuesheetProtoFiles {
  std::string cuesheet;
  std::string proto;
};

class CuesheetEqualsProtoFilesTest :
    public testing::TestWithParam<CuesheetProtoFiles> {};

struct CuesheetProtoSample {
  CuesheetProtoSample(absl::string_view textproto, std::string cuesheet)
      : cuesheet(std::move(cuesheet)) {
    expected = CuesheetFromProtoStringOrDie(textproto);
  }

  Cuesheet expected;
  std::string cuesheet;
};

class CuesheetEqualsProtoTest :
  public testing::TestWithParam<CuesheetProtoSample> {};

TEST_P(CuesheetEqualsProtoFilesTest, MatchFiles) {
  auto files = GetParam();

  Cuesheet expected = CuesheetFromProtoFileOrDie(files.proto);

  std::ifstream istrm = OpenTestdataOrDie(files.cuesheet);

  GError *err = nullptr;
  auto found = ParseCuesheet(&istrm, &err);
  ASSERT_TRUE(IsOk(err));

  EXPECT_TRUE(IsEqual(expected, *found));
}

TEST_P(CuesheetEqualsProtoTest, MatchSample) {
  const auto &p = GetParam();
  const Cuesheet &expected = p.expected;
  const std::string &cuesheet = p.cuesheet;

  GError *err = nullptr;
  auto found = ParseCuesheetFromString(cuesheet, &err);
  ASSERT_TRUE(IsOk(err));

  EXPECT_TRUE(IsEqual(expected, *found));
}

INSTANTIATE_TEST_SUITE_P(
    Examples,
    CuesheetEqualsProtoFilesTest,
    testing::Values(
      CuesheetProtoFiles{"eac_multifile_gapless.cue",
                         "eac_multifile_gapless.textproto"},
      CuesheetProtoFiles{"eac_multifile_gaps.cue", "eac_multifile_gaps.textproto"},
      CuesheetProtoFiles{"eac_singlefile.cue", "eac_singlefile.textproto"},
      CuesheetProtoFiles{"hidden_track.cue", "hidden_track.textproto"},
      CuesheetProtoFiles{"full_disc.cue", "full_disc.textproto"}));

INSTANTIATE_TEST_SUITE_P(
    CornerCases,
    CuesheetEqualsProtoTest,
    testing::Values(
      CuesheetProtoSample("", ""),
      CuesheetProtoSample("", "   \n\t\t\n")
    )
);

INSTANTIATE_TEST_SUITE_P(
    Quoting,
    CuesheetEqualsProtoTest,
    testing::Values(
      CuesheetProtoSample(
        R"""(tags { comment_tag { name: "FOO" value: "\"Bar" } })""",
        R"""(REM FOO "Bar)"""
      ),
      CuesheetProtoSample(
        R"""(tags { comment_tag { name: "FOO" value: "Foo Bar" } })""",
        R"""(REM FOO "Foo Bar")"""
      ),
      CuesheetProtoSample(
        R"""(tags { comment_tag { name: "FOO" value: "Foo Bar" } })""",
        R"""(REM FOO Foo Bar)"""
      ),
      CuesheetProtoSample(
        R"""(tags { title: "" })""",
        R"""(TITLE "")"""
      ),
      CuesheetProtoSample(
        R"""(tags { performer: "The Specials" })""",
        R"""(PERFORMER "The Specials")"""
      ),
      CuesheetProtoSample(
        R"""(file { path: "" type: TYPE_WAVE })""",
        R"""(FILE "" WAVE)"""
      )
    )
);

INSTANTIATE_TEST_SUITE_P(
    Spaces,
    CuesheetEqualsProtoTest,
    testing::Values(
      CuesheetProtoSample(
        R"""(
            file {
              path: "foo.wav" type: TYPE_WAVE
              track {
                type: TYPE_AUDIO number: 1
                index { number: 1 position {} }
              }
            }
        )""",
        R"""(
            FILE  "foo.wav"   WAVE
            TRACK  01 AUDIO
            INDEX 01 00:00:00
        )"""
      )
    )
);


TEST(ParseInvalidTest, MissingFileType) {
  std::istringstream istrm(R"""(
    FILE "The Specials - Singles.wav"
  )""");

  GError *err = nullptr;
  ParseCuesheet(&istrm, &err);
  ASSERT_FALSE(IsOk(err));
}

TEST(ParseInvalidTest, CommandsOutOfOrder) {
  std::istringstream istrm(R"""(
    FILE "The Specials - Singles.wav" WAVE
    INDEX 01 00:00:00
    TRACK 01 AUDIO
  )""");

  GError *err = nullptr;
  ParseCuesheet(&istrm, &err);
  ASSERT_FALSE(IsOk(err));
}

TEST(ParseInvalidTest, MissingTrack) {
  std::istringstream istrm(R"""(
    FILE "The Specials - Singles.wav" WAVE
    INDEX 01 00:00:00
  )""");

  GError *err = nullptr;
  ParseCuesheet(&istrm, &err);
  ASSERT_FALSE(IsOk(err));
}

}  // namespace
}  // namespace cue2pb
