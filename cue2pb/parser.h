#ifndef CUE2PB_PARSER_H_
#define CUE2PB_PARSER_H_

#include <istream>

#include "cue2pb/cuesheet.pb.h"
#include "absl/status/statusor.h"

namespace cue2pb {

absl::StatusOr<Cuesheet> ParseCuesheet(std::istream *input);

}  // namespace cue2pb

#endif  // CUE2PB_PARSER_H_
