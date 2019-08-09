#include "cue2pb/text_format.h"

#include <utility>

#include "google/protobuf/io/zero_copy_stream_impl.h"
#include "google/protobuf/text_format.h"

namespace cue2pb {

using ::google::protobuf::io::IstreamInputStream;
using ::google::protobuf::TextFormat;

absl::optional<Cuesheet> ParseCuesheetFromTextProto(
    std::istream *input, GError **error) {
  IstreamInputStream istrm(input);
  GErrorCollector collector(
      error, CUE2PB_ERROR, cue2pb::ERR_PROTO, /*options=*/{});
  TextFormat::Parser parser;
  Cuesheet cuesheet;

  parser.RecordErrorsTo(&collector);
  if (!parser.Parse(&istrm, &cuesheet)) return absl::nullopt;

  return absl::make_optional(std::move(cuesheet));
}

}  // namespace cue2pb
