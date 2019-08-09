#ifndef CUE2PB_ERRORS_H_
#define CUE2PB_ERRORS_H_

#include <sysexits.h>

#include <glib.h>

#include "absl/strings/str_format.h"
#include "google/protobuf/io/tokenizer.h"

#define CHECK_OK(gerror) \
  do { \
    if (gerror != nullptr) { \
      g_error("%s", gerror->message); \
    } \
  } while(false)

#define CUE2PB_ERROR (::cue2pb::ErrorQuark())

namespace cue2pb {

GQuark ErrorQuark();

enum Error : int {
  ERR_UNKNOWN = EX_SOFTWARE,
  ERR_USAGE = EX_USAGE,
  ERR_OS = EX_OSERR,
  ERR_UNIMPL = 110,
  ERR_SYNTAX = 111,
  ERR_PROTO = 112,
};

template <typename... Args>
void SetError(GError **error, Error code, const absl::FormatSpec<Args...> &fmt,
              Args... args) {
  g_set_error_literal(error, CUE2PB_ERROR, code,
                      absl::StrFormat(fmt, args...).c_str());
}

class GErrorCollector : public ::google::protobuf::io::ErrorCollector {
 public:
  struct Options {
    bool include_warnings = true;
  };

  GErrorCollector(GError **error, GQuark domain, gint code, Options options);

  void AddError(
      int line, google::protobuf::io::ColumnNumber column,
      const std::string &message) override;
  void AddWarning(
      int line, google::protobuf::io::ColumnNumber column,
      const std::string &message) override;

 private:
  void AddMessage(const std::string &message);

  Options options_;
  GQuark domain_;
  gint code_;
  GError **error_;
};

}  // namespace cue2pb

#endif  // CUE2PB_ERRORS_H_
