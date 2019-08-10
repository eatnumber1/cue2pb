#include "cue2pb/testing/assertions.h"

#include <string>

#include "gtest/gtest.h"
#include "cue2pb/errors.h"
#include "google/protobuf/util/message_differencer.h"
#include "google/protobuf/message.h"

namespace cue2pb {

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
  if (differencer.Compare(a, b)) {
    return AssertionSuccess()
      << "Protos are equal. They are " << a.DebugString();
  } else {
    return AssertionFailure() << "Protos differ: " << differences;
  }
}

}  // namespace cue2pb
