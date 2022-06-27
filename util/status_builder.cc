#include "util/status_builder.h"

#include <type_traits>
#include <iostream>

#include "absl/strings/str_format.h"

namespace util {

StatusBuilder UnknownErrorBuilder() {
  return {absl::UnknownError("")};
}
StatusBuilder InvalidArgumentErrorBuilder() {
  return {absl::InvalidArgumentError("")};
}
StatusBuilder UnimplementedErrorBuilder() {
  return {absl::UnimplementedError("")};
}
StatusBuilder InternalErrorBuilder() {
  return {absl::InternalError("")};
}
StatusBuilder FailedPreconditionErrorBuilder() {
  return {absl::FailedPreconditionError("")};
}
StatusBuilder NotFoundErrorBuilder() {
  return {absl::NotFoundError("")};
}

StatusBuilder::StatusBuilder(const absl::Status &original)
  : status_(original)
  {}

StatusBuilder::operator absl::Status() const {
  return status_;
}

std::ostream &operator<<(std::ostream &os, const StatusBuilder &builder) {
  return os << static_cast<absl::Status>(builder);
}

bool StatusBuilder::ok() const {
  return status_.ok();
}

}  // namespace util
