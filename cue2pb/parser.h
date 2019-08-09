#ifndef CUE2PB_PARSER_H_
#define CUE2PB_PARSER_H_

#include <istream>

#include "cue2pb/cuesheet.pb.h"
#include "cue2pb/errors.h"
#include "absl/types/optional.h"

namespace cue2pb {

absl::optional<Cuesheet> ParseCuesheet(std::istream *input, GError **error);

}  // namespace cue2pb

#endif  // CUE2PB_PARSER_H_
