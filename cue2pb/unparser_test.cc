#include "cue2pb/unparser.h"

#include <string>
#include <string_view>
#include <utility>
#include <fstream>
#include <sstream>
#include <cerrno>
#include <cstring>

#include "gtest/gtest.h"
#include "cue2pb/file.h"
#include "cue2pb/text_format.h"
#include "rhutil/status.h"
#include "rhutil/errno.h"
#include "cue2pb/testing/assertions.h"
#include "rhutil/testing/assertions.h"

namespace cue2pb {

using ::rhutil::StatusOr;

namespace {

std::string TestdataToPath(std::string_view filename) {
  return "cue2pb/testdata/" + std::string(filename);
}

std::ifstream OpenTestdataOrDie(std::string_view filename) {
  return OpenInputFile(TestdataToPath(filename)).ValueOrDie();
}

StatusOr<Cuesheet> CuesheetFromProtoString(std::string_view textproto) {
  std::istringstream istrm{std::string(textproto)};
  return CuesheetFromTextProto(&istrm);
}

Cuesheet CuesheetFromProtoStringOrDie(std::string_view textproto) {
  return CuesheetFromProtoString(textproto).ValueOrDie();
}

Cuesheet CuesheetFromProtoFileOrDie(std::string_view filename) {
  std::ifstream istrm = OpenTestdataOrDie(filename);
  return CuesheetFromTextProto(&istrm).ValueOrDie();
}

std::string ReadFileEntirelyOrDie(std::string filename) {
  std::ifstream in = OpenTestdataOrDie(filename);
  std::stringstream sstr;
  sstr << in.rdbuf();
  if (in.fail()) CHECK_OK(rhutil::ErrnoAsStatus());
  return std::move(sstr).str();
}

StatusOr<std::string> UnparseCuesheet(Cuesheet cuesheet) {
  std::stringstream out;
  RETURN_IF_ERROR(UnparseCuesheet(cuesheet, &out));
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

  auto found_or = UnparseCuesheet(incue);
  ASSERT_TRUE(IsOk(found_or));

  EXPECT_EQ(expected, found_or.ValueOrDie());
}

TEST_P(CuesheetEqualsProtoTest, MatchSample) {
  const auto &p = GetParam();
  const std::string &expected = p.cuesheet;
  const std::string &proto = p.proto;

  Cuesheet incue = CuesheetFromProtoStringOrDie(proto);

  auto found_or = UnparseCuesheet(incue);
  ASSERT_TRUE(IsOk(found_or));

  EXPECT_EQ(expected, found_or.ValueOrDie());
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
