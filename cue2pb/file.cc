#include "cue2pb/file.h"

#include <cstring>
#include <cerrno>

namespace cue2pb {

std::ifstream OpenInputFile(absl::string_view path, GError **error) {
  return OpenInputFile(path, std::ios::in, error);
}

std::ifstream OpenInputFile(
    absl::string_view path, std::ios_base::openmode mode, GError **error) {
  std::ifstream istrm;
  istrm.open(std::string(path), mode);
  if (istrm.fail()) {
    SetError(error, cue2pb::ERR_OS, "Failed to open %s: %s", path,
             std::strerror(errno));
    return istrm;
  }
  g_assert(istrm.is_open());
  return istrm;
}

}  // namespace cue2pb
