#include "util/status_builder.h"

#include "gtest/gtest.h"
#include "absl/status/status.h"

namespace util {
namespace {

TEST(StatusBuilderTest, Ok) {
  util::StatusBuilder sb(absl::OkStatus());
  sb << "an error occurred";
  ASSERT_TRUE(sb.ok());

  absl::Status s(sb);
  EXPECT_EQ(s.code(), absl::StatusCode::kOk);
  EXPECT_EQ(s.ToString(), "OK");
}

TEST(StatusBuilderTest, Bad) {
  util::StatusBuilder sb(absl::UnknownError("error"));
  sb << " an error occurred";
  ASSERT_FALSE(sb.ok());

  absl::Status s(sb);
  EXPECT_EQ(s.code(), absl::StatusCode::kUnknown);
  EXPECT_EQ(s.ToString(), "UNKNOWN: error an error occurred");
}

}  // namespace
}  // namespace util
