#include "cue2pb/file.h"

#include <cassert>

#include "rhutil/errno.h"
#include "absl/strings/str_format.h"

namespace cue2pb {

using ::rhutil::StatusOr;
using ::rhutil::Status;

StatusOr<std::ifstream> OpenInputFile(std::string_view path) {
  return OpenInputFile(path, std::ios::in);
}

StatusOr<std::ifstream> OpenInputFile(std::string_view path,
                                      std::ios_base::openmode mode) {
  std::ifstream istrm;
  istrm.open(std::string(path), mode);
  if (istrm.fail()) {
    auto st = rhutil::ErrnoAsStatus();
    return Status(st.code(), absl::StrFormat("Failed to open %s: %s", path,
                                             st.message()));
  }
  assert(istrm.is_open());
  return std::move(istrm);
}

}  // namespace cue2pb
