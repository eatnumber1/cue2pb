#include "util/file.h"

#include <cassert>

#include "util/errno.h"
#include "util/status_builder.h"
#include "absl/strings/str_format.h"

namespace util {

absl::StatusOr<std::ifstream> OpenInputFile(std::string_view path) {
  return OpenInputFile(path, std::ios::in);
}

absl::StatusOr<std::ifstream> OpenInputFile(std::string_view path,
                                            std::ios_base::openmode mode) {
  std::ifstream istrm;
  istrm.open(std::string(path), mode);
  if (istrm.fail()) {
    return util::StatusBuilder(ErrnoAsStatus()) << "Failed to open " << path;
  }
  assert(istrm.is_open());
  return std::move(istrm);
}

}  // namespace util
