#include "util/testing/protobuf_assertions.h"

#include <string>

#include "gtest/gtest.h"
#include "google/protobuf/util/message_differencer.h"
#include "google/protobuf/message.h"

namespace util {

using ::google::protobuf::util::MessageDifferencer;
using ::google::protobuf::Message;
using ::testing::AssertionResult;
using ::testing::AssertionSuccess;
using ::testing::AssertionFailure;

AssertionResult IsEqual(const Message &a, const Message &b) {
  std::string differences;
  MessageDifferencer differencer;
  differencer.ReportDifferencesToString(&differences);
  if (differencer.Compare(a, b)) {
    return AssertionSuccess()
      << "Protos are equal. They are " << a.DebugString();
  } else {
    return AssertionFailure() << "Protos differ: " << differences;
  }
}

}  // namespace util
