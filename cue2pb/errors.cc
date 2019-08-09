#include "cue2pb/errors.h"

#include <utility>

namespace cue2pb {

GQuark ErrorQuark() {
  return g_quark_from_static_string("cue2pb-error-quark");
}

GErrorCollector::GErrorCollector(GError **error, GQuark domain, gint code,
                                 Options options)
  : options_(std::move(options)),
    domain_(domain),
    code_(code),
    error_(error) {
  g_assert(error_ == nullptr || *error_ == nullptr);
}

void GErrorCollector::AddMessage(const std::string &message) {
  if (!error_) return;

  if (*error_ == nullptr) {
    *error_ = g_error_new_literal(domain_, code_, message.c_str());
  } else {
    GError *err = *error_;
    gchar *old_message = err->message;
    err->message = g_strdup_printf("%s\n%s", err->message, message.c_str());
    g_free(old_message);
  }
}

void GErrorCollector::AddError(
    int line, google::protobuf::io::ColumnNumber column,
    const std::string &message) {
  line++, column++;  // line and column numbers come in zero-indexed
  AddMessage(absl::StrFormat("Error at line %d:%d: %s", line, column,
                             message));
}

void GErrorCollector::AddWarning(
    int line, google::protobuf::io::ColumnNumber column,
    const std::string &message) {
  line++, column++;  // line and column numbers come in zero-indexed

  if (!options_.include_warnings) return;

  AddMessage(absl::StrFormat("Warning at line %d:%d: %s", line, column,
                             message));
}

}  // namespace cue2pb
