#include "util/errno.h"

#include <cerrno>
#include <cstring>

#include "util/status_builder.h"

namespace util {

absl::Status ErrnoAsStatus() {
  return ErrnoAsStatus(errno);
}

absl::Status ErrnoAsStatus(int err) {
  if (err == 0) return absl::OkStatus();
  auto code = absl::StatusCode::kUnknown;
  switch (err) {
    case ENOENT:
      code = absl::StatusCode::kNotFound;
      break;
    case EINVAL:
      code = absl::StatusCode::kInvalidArgument;
    // TODO(eatnumber1): Fill in more errors.
    default:
      break;
  }
  return {code, std::strerror(err)};
}

absl::Status ErrorCodeAsStatus(std::error_code code) {
  if (!code) return absl::OkStatus();
  const std::error_category &cat = code.category();
  if (cat == std::generic_category()) {
    return ErrnoAsStatus(code.value());
  }
  return util::StatusBuilder({absl::StatusCode::kUnknown, ""})
      << code << ": " << code.message();
}

absl::Status ProcessExitCodeToStatus(int code) {
  if (code == 0) return absl::OkStatus();
  return util::UnknownErrorBuilder() << "code " << code;
}

}  // namespace util
