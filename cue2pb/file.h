#ifndef CUE2PB_FILE_H_
#define CUE2PB_FILE_H_

#include <fstream>
#include <ios>

#include "cue2pb/errors.h"
#include "absl/strings/string_view.h"

namespace cue2pb {

std::ifstream OpenInputFile(
    absl::string_view path, GError **error);
std::ifstream OpenInputFile(
    absl::string_view path, std::ios_base::openmode mode, GError **error);

}  // namespace cue2pb

#endif  // CUE2PB_FILE_H_
