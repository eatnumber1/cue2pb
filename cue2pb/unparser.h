#ifndef CUE2PB_UNPARSER_H_
#define CUE2PB_UNPARSER_H_

#include <ostream>

#include "cue2pb/cuesheet.pb.h"
#include "absl/status/status.h"

namespace cue2pb {

absl::Status UnparseCuesheet(const Cuesheet &cuesheet, std::ostream *output);

}  // namespace cue2pb

#endif  // CUE2PB_UNPARSER_H_
