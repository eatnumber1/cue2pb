#ifndef CUE2PB_TEXT_FORMAT_H_
#define CUE2PB_TEXT_FORMAT_H_

#include <fstream>
#include <ios>

#include "cue2pb/errors.h"
#include "cue2pb/cuesheet.pb.h"
#include "absl/types/optional.h"

namespace cue2pb {

absl::optional<Cuesheet> CuesheetFromTextProto(std::istream *input,
                                               GError **error);

}  // namespace cue2pb

#endif  // CUE2PB_CUESHEET_H_
