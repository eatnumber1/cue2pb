#ifndef RHUTIL_STATUS_H_
#define RHUTIL_STATUS_H_

#include <string>
#include <string_view>
#include <utility>
#include <ostream>
#include <sstream>

#include "absl/base/attributes.h"
#include "absl/strings/str_cat.h"
#include "absl/status/status.h"
#include "absl/strings/cord.h"

namespace util {

class ABSL_MUST_USE_RESULT StatusBuilder {
 public:
   StatusBuilder(const absl::Status &original);

   operator absl::Status() const;

   ABSL_MUST_USE_RESULT bool ok() const;

   template <typename T>
   StatusBuilder &operator<<(const T &value);

 private:
   absl::Status status_;
};

std::ostream& operator<<(std::ostream& os, const StatusBuilder &builder);

StatusBuilder UnknownErrorBuilder();
StatusBuilder InvalidArgumentErrorBuilder();
StatusBuilder UnimplementedErrorBuilder();
StatusBuilder InternalErrorBuilder();
StatusBuilder FailedPreconditionErrorBuilder();
StatusBuilder NotFoundErrorBuilder();

bool IsFailedPrecondition(const absl::Status &st);
bool IsNotFound(const absl::Status &st);

// implementation details below

template <typename T>
StatusBuilder &StatusBuilder::operator<<(const T &value) {
  if (status_.ok()) return *this;
  std::ostringstream strm;
  strm << value;
  absl::Status new_status(status_.code(), absl::StrCat(status_.message(), strm.str()));
  status_.ForEachPayload([&new_status](absl::string_view type_url,
                                       const absl::Cord &payload) {
    new_status.SetPayload(type_url, payload);
  });
  status_ = std::move(new_status);
  return *this;
}

}  // namespace util

#endif  // RHUTIL_STATUS_H_
