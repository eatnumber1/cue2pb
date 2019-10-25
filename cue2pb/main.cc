#include <fstream>
#include <string>
#include <iostream>
#include <sysexits.h>
#include <cstdlib>
#include <stdlib.h>
#include <ios>

#include "cue2pb/parser.h"
#include "rhutil/status.h"
#include "cue2pb/unparser.h"
#include "cue2pb/file.h"
#include "cue2pb/text_format.h"
#include "cue2pb/cuesheet.pb.h"
#include "absl/types/span.h"
#include "absl/strings/string_view.h"
#include "absl/strings/str_cat.h"
#include "absl/flags/flag.h"
#include "absl/flags/parse.h"
#include "absl/flags/usage.h"
#include "absl/flags/usage_config.h"
#include "google/protobuf/text_format.h"
#include "google/protobuf/io/zero_copy_stream_impl.h"

ABSL_FLAG(bool, proto_to_cue, false,
          "Convert back from a protobuf to a Cuesheet");
ABSL_FLAG(bool, textformat, false, "Use the text protobuf format");

namespace cue2pb {

using ::google::protobuf::TextFormat;
using ::google::protobuf::io::OstreamOutputStream;
using ::rhutil::Status;
using ::rhutil::OkStatus;
using ::rhutil::UnknownError;
using ::rhutil::InvalidArgumentError;

Status ProtoToCue(absl::string_view protofile) {
  bool textformat = absl::GetFlag(FLAGS_textformat);

  auto mode = std::ios::in;
  if (!textformat) {
    mode |= std::ios::binary;
  }

  ASSIGN_OR_RETURN(std::ifstream istrm, OpenInputFile(protofile, mode));

  Cuesheet cuesheet;
  if (textformat) {
    ASSIGN_OR_RETURN(cuesheet, CuesheetFromTextProto(&istrm));
  } else {
    if (!cuesheet.ParseFromIstream(&istrm)) {
      return UnknownError("Failed to parse binary proto");
    }
  }

  RETURN_IF_ERROR(UnparseCuesheet(cuesheet, &std::cout));

  return OkStatus();
}

Status CueToProto(absl::string_view cuefile) {
  bool textformat = absl::GetFlag(FLAGS_textformat);

  auto mode = std::ios::in;
  if (!textformat) {
    mode |= std::ios::binary;
  }

  ASSIGN_OR_RETURN(std::ifstream istrm, OpenInputFile(cuefile, mode));
  ASSIGN_OR_RETURN(Cuesheet cuesheet, ParseCuesheet(&istrm));

  if (textformat) {
    OstreamOutputStream cout_os(&std::cout);
    if (!TextFormat::Print(cuesheet, &cout_os)) {
      return UnknownError("Failed to print cuesheet");
    }
  } else {
    if (!cuesheet.SerializeToOstream(&std::cout)) {
      return UnknownError("Failed to serialize binary proto");
    }
  }

  return OkStatus();
}

Status Main(absl::Span<absl::string_view> args) {
  if (args.size() != 1) {
    return InvalidArgumentError("No cuefile specified or too many arguments");
  }

  if (absl::GetFlag(FLAGS_proto_to_cue)) {
    return ProtoToCue(args[0]);
  } else {
    return CueToProto(args[0]);
  }
}

}  // namespace cue2pb

int main(int argc, char *argv[]) {
  absl::SetProgramUsageMessage(
      absl::StrCat(
        "Convert CD cuesheets to/from protobufs.\n",
        "Usage: ", getprogname(), " [flags] cuefile"));
  absl::FlagsUsageConfig config;
  config.contains_help_flags = [](absl::string_view path) {
    return path == "cue2pb/main.cc";
  };
  absl::SetFlagsUsageConfig(std::move(config));
  std::vector<char*> cargs = absl::ParseCommandLine(argc, argv);
  std::vector<absl::string_view> args(cargs.begin(), cargs.end());

  using ::rhutil::Status;
  if (Status err = cue2pb::Main(absl::MakeSpan(args).subspan(1)); !err.ok()) {
    if (err.code() == rhutil::StatusCode::kInvalidArgument) {
      std::cerr << absl::ProgramUsageMessage() << std::endl << std::endl;
    }
    std::cerr << err << std::endl;
    return static_cast<int>(err.code());
  }
  return EXIT_SUCCESS;
}
