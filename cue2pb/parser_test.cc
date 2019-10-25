#include "cue2pb/parser.h"

#include <string>
#include <utility>
#include <fstream>
#include <sstream>

#include "gtest/gtest.h"
#include "cue2pb/file.h"
#include "cue2pb/text_format.h"
#include "rhutil/status.h"
#include "cue2pb/testing/assertions.h"
#include "rhutil/testing/assertions.h"

namespace cue2pb {

using ::rhutil::IsOk;
using ::rhutil::StatusOr;

namespace {

std::string TestdataToPath(std::string_view filename) {
  return "cue2pb/testdata/" + std::string(filename);
}

std::ifstream OpenTestdataOrDie(std::string_view filename) {
  return OpenInputFile(TestdataToPath(filename)).ValueOrDie();
}

Cuesheet CuesheetFromProtoFileOrDie(std::string_view filename) {
  std::ifstream istrm = OpenTestdataOrDie(filename);
  auto cuesheet_or = CuesheetFromTextProto(&istrm);
  CHECK_OK(cuesheet_or.status());
  return std::move(cuesheet_or).ValueOrDie();
}

Cuesheet CuesheetFromProtoStringOrDie(std::string_view textproto) {
  std::istringstream istrm{std::string(textproto)};
  auto cuesheet_or = CuesheetFromTextProto(&istrm);
  CHECK_OK(cuesheet_or.status());
  return std::move(cuesheet_or).ValueOrDie();
}

StatusOr<Cuesheet> ParseCuesheetFromString(std::string_view s) {
  std::istringstream istrm{std::string(s)};
  return ParseCuesheet(&istrm);
}

struct CuesheetProtoFiles {
  std::string cuesheet;
  std::string proto;
};

class CuesheetEqualsProtoFilesTest :
    public testing::TestWithParam<CuesheetProtoFiles> {};

struct CuesheetProtoSample {
  CuesheetProtoSample(std::string_view textproto, std::string cuesheet)
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

  auto found_or = ParseCuesheet(&istrm);
  ASSERT_TRUE(IsOk(found_or));

  EXPECT_TRUE(IsEqual(expected, found_or.ValueOrDie()));
}

TEST_P(CuesheetEqualsProtoTest, MatchSample) {
  const auto &p = GetParam();
  const Cuesheet &expected = p.expected;
  const std::string &cuesheet = p.cuesheet;

  auto found_or = ParseCuesheetFromString(cuesheet);
  ASSERT_TRUE(IsOk(found_or));

  EXPECT_TRUE(IsEqual(expected, found_or.ValueOrDie()));
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

  ASSERT_FALSE(IsOk(ParseCuesheet(&istrm)));
}

TEST(ParseInvalidTest, CommandsOutOfOrder) {
  std::istringstream istrm(R"""(
    FILE "The Specials - Singles.wav" WAVE
    INDEX 01 00:00:00
    TRACK 01 AUDIO
  )""");

  ASSERT_FALSE(IsOk(ParseCuesheet(&istrm)));
}

TEST(ParseInvalidTest, MissingTrack) {
  std::istringstream istrm(R"""(
    FILE "The Specials - Singles.wav" WAVE
    INDEX 01 00:00:00
  )""");

  ASSERT_FALSE(IsOk(ParseCuesheet(&istrm)));
}

}  // namespace
}  // namespace cue2pb
