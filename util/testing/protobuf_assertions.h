#ifndef UTIL_TESTING_PROTOBUF_ASSERTIONS_H_
#define UTIL_TESTING_PROTOBUF_ASSERTIONS_H_

#include "gtest/gtest.h"
#include "google/protobuf/util/message_differencer.h"
#include "google/protobuf/message.h"

namespace util {

testing::AssertionResult IsEqual(const google::protobuf::Message &a,
                                 const google::protobuf::Message &b);

}  // namespace util

#endif  // UTIL_TESTING_PROTOBUF_ASSERTIONS_H_
