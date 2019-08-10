#ifndef CUE2PB_TESTING_ASSERTIONS_H_
#define CUE2PB_TESTING_ASSERTIONS_H_

#include "gtest/gtest.h"
#include "cue2pb/errors.h"
#include "google/protobuf/util/message_differencer.h"
#include "google/protobuf/message.h"

namespace cue2pb {

testing::AssertionResult IsOk(GError *err);

testing::AssertionResult IsEqual(const google::protobuf::Message &a,
                                 const google::protobuf::Message &b);

}  // namespace cue2pb

#endif  // CUE2PB_TESTING_ASSERTIONS_H_
