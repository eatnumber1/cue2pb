#ifndef UTIL_ERRNO_H_
#define UTIL_ERRNO_H_

#include <system_error>

#include "absl/status/status.h"

namespace util {

absl::Status ErrnoAsStatus();
absl::Status ErrnoAsStatus(int err);

absl::Status ErrorCodeAsStatus(std::error_code code);

absl::Status ProcessExitCodeToStatus(int code);

}  // namespace rhutil

#endif  // UTIL_ERRNO_H_
