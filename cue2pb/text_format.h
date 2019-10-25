#ifndef CUE2PB_TEXT_FORMAT_H_
#define CUE2PB_TEXT_FORMAT_H_

#include <fstream>
#include <ios>

#include "cue2pb/cuesheet.pb.h"
#include "rhutil/status.h"

namespace cue2pb {

rhutil::StatusOr<Cuesheet> CuesheetFromTextProto(std::istream *input);

}  // namespace cue2pb

#endif  // CUE2PB_CUESHEET_H_
