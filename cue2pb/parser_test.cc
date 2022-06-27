#include "cue2pb/parser.h"

#include <string>
#include <utility>
#include <fstream>
#include <sstream>

#include "gtest/gtest.h"
#include "util/file.h"
#include "cue2pb/text_format.h"
#include "absl/status/statusor.h"
#include "util/testing/protobuf_assertions.h"
#include "util/testing/assertions.h"
#include "util/status_macros.h"

namespace cue2pb {

using ::util::IsOk;
using ::util::IsEqual;

namespace {

std::string TestdataToPath(std::string_view filename) {
  return "cue2pb/testdata/" + std::string(filename);
}

std::ifstream OpenTestdataOrDie(std::string_view filename) {
  return util::OpenInputFile(TestdataToPath(filename)).value();
}

Cuesheet CuesheetFromProtoFileOrDie(std::string_view filename) {
  std::ifstream istrm = OpenTestdataOrDie(filename);
  absl::StatusOr<Cuesheet> cuesheet = CuesheetFromTextProto(&istrm);
  CHECK_OK(cuesheet.status());
  return *std::move(cuesheet);
}

Cuesheet CuesheetFromProtoStringOrDie(std::string_view textproto) {
  std::istringstream istrm{std::string(textproto)};
  absl::StatusOr<Cuesheet> cuesheet = CuesheetFromTextProto(&istrm);
  CHECK_OK(cuesheet.status());
  return *std::move(cuesheet);
}

absl::StatusOr<Cuesheet> ParseCuesheetFromString(std::string_view s) {
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

  absl::StatusOr<Cuesheet> found = ParseCuesheet(&istrm);
  ASSERT_TRUE(IsOk(found));
  EXPECT_TRUE(IsEqual(expected, *found));
}

TEST_P(CuesheetEqualsProtoTest, MatchSample) {
  const auto &p = GetParam();
  const Cuesheet &expected = p.expected;
  const std::string &cuesheet = p.cuesheet;

  absl::StatusOr<Cuesheet> found = ParseCuesheetFromString(cuesheet);
  ASSERT_TRUE(IsOk(found));
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
