#ifndef UTIL_STATUS_MACROS_H_
#define UTIL_STATUS_MACROS_H_

#include <utility>

#include "absl/base/attributes.h"
#include "absl/status/status.h"
#include "util/status_builder.h"

#define RETURN_IF_ERROR(expr) \
  if (::absl::Status s = (expr); !s.ok()) return ::util::StatusBuilder(s)

#define CHECK_OK(expr) \
    do { \
      if (auto s = (expr); !s.ok()) ::util::StatusInternalOnlyDie(s); \
    } while (false)
#define CHECK(expr) \
    do { \
      if (!(expr)) \
          ::util::StatusInternalOnlyDie(::absl::InternalError(#expr)); \
    } while (false)

#define ASSIGN_OR_RETURN(decl, expr) \
  decl = ({ \
    auto st = (expr); \
    if (!st.ok()) { return std::move(st).status(); } \
    *std::move(st); \
  })

namespace util {

ABSL_ATTRIBUTE_NORETURN
void StatusInternalOnlyDie(const absl::Status &);

}  // namespace util

#endif  // UTIL_STATUS_MACROS_H_
