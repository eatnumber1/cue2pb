#include "cue2pb/parser.h"

#include <string>
#include <utility>
#include <fstream>
#include <sstream>

#include "gtest/gtest.h"
#include "cue2pb/file.h"
#include "cue2pb/text_format.h"
#include "cue2pb/errors.h"
#include "absl/types/optional.h"
#include "absl/strings/string_view.h"
#include "google/protobuf/util/message_differencer.h"
#include "google/protobuf/message.h"

namespace cue2pb {
namespace {

using ::google::protobuf::util::MessageDifferencer;
using ::google::protobuf::Message;
using ::testing::AssertionResult;
using ::testing::AssertionSuccess;
using ::testing::AssertionFailure;

AssertionResult IsOk(GError *err) {
  if (err) {
    return AssertionFailure() << err->message;
  } else {
    return AssertionSuccess() << "No error occurred";
  }
}

AssertionResult IsEqual(const Message &a, const Message &b) {
  std::string differences;
  MessageDifferencer differencer;
  differencer.ReportDifferencesToString(&differences);
  differencer.set_repeated_field_comparison(MessageDifferencer::AS_LIST);
  if (differencer.Compare(a, b)) {
    return AssertionSuccess()
      << "Protos are equal. They are " << a.DebugString();
  } else {
    return AssertionFailure() << "Protos differ: " << differences;
  }
}

std::string FilenameToPath(absl::string_view filename) {
  return "cue2pb/testdata/" + std::string(filename);
}

Cuesheet ReadCueTextProtoFileOrDie(absl::string_view filename) {
  GError *err = nullptr;
  std::ifstream istrm = OpenInputFile(FilenameToPath(filename), &err);
  CHECK_OK(err);

  auto cuesheet = ParseCuesheetFromTextProto(&istrm, &err);
  CHECK_OK(err);

  return std::move(*cuesheet);
}

Cuesheet CuesheetFromProtoStringOrDie(absl::string_view textproto) {
  std::istringstream istrm{std::string(textproto)};
  GError *err = nullptr;
  auto cuesheet = ParseCuesheetFromTextProto(&istrm, &err);
  CHECK_OK(err);

  return std::move(*cuesheet);
}

absl::optional<Cuesheet> ParseCuesheetFromString(absl::string_view s, GError **error) {
  std::istringstream istrm{std::string(s)};
  return ParseCuesheet(&istrm, error);
}

std::ifstream OpenInputFileOrDie(absl::string_view filename) {
  GError *err = nullptr;
  std::ifstream istrm = OpenInputFile(FilenameToPath(filename), &err);
  CHECK_OK(err);
  return istrm;
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

  Cuesheet expected = ReadCueTextProtoFileOrDie(files.proto);

  std::ifstream istrm = OpenInputFileOrDie(files.cuesheet);

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
