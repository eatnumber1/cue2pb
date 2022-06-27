#ifndef UTIL_FILE_H_
#define UTIL_FILE_H_

#include <fstream>
#include <ios>
#include <string_view>

#include "absl/status/statusor.h"

namespace util {

absl::StatusOr<std::ifstream> OpenInputFile(std::string_view path);
absl::StatusOr<std::ifstream> OpenInputFile(std::string_view path,
                                      std::ios_base::openmode mode);

}  // namespace util

#endif  // UTIL_FILE_H_
