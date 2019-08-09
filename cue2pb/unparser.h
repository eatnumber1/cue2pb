#ifndef CUE2PB_UNPARSER_H_
#define CUE2PB_UNPARSER_H_

#include <ostream>

#include "cue2pb/cuesheet.pb.h"
#include "cue2pb/errors.h"

namespace cue2pb {

bool UnparseCuesheet(const Cuesheet &cuesheet, std::ostream *output,
                     GError **error);

}  // namespace cue2pb

#endif  // CUE2PB_UNPARSER_H_
