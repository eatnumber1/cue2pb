#ifndef CUE2PB_FILE_H_
#define CUE2PB_FILE_H_

#include <fstream>
#include <ios>
#include <string_view>

#include "rhutil/status.h"

namespace cue2pb {

rhutil::StatusOr<std::ifstream> OpenInputFile(std::string_view path);
rhutil::StatusOr<std::ifstream> OpenInputFile(std::string_view path,
                                              std::ios_base::openmode mode);

}  // namespace cue2pb

#endif  // CUE2PB_FILE_H_
