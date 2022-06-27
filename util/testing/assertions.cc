#include "util/testing/assertions.h"

#include <string>

namespace util {

using ::testing::AssertionResult;
using ::testing::AssertionSuccess;
using ::testing::AssertionFailure;

AssertionResult IsOk(const absl::Status &st) {
  if (st.ok()) {
    return AssertionSuccess() << "No error occurred";
  } else {
    return AssertionFailure() << st;
  }
}

}  // namespace util
