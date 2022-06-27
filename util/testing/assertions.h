#ifndef UTIL_TESTING_ASSERTIONS_H_
#define UTIL_TESTING_ASSERTIONS_H_

#include "gtest/gtest.h"
#include "absl/status/status.h"
#include "absl/status/statusor.h"

namespace util {

testing::AssertionResult IsOk(const absl::Status &);

template <typename T>
testing::AssertionResult IsOk(const absl::StatusOr<T> &st) {
  return IsOk(st.status());
}

}  // namespace util

#endif  // UTIL_TESTING_ASSERTIONS_H_
