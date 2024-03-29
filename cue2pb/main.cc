#include <fstream>
#include <string>
#include <iostream>
#include <sysexits.h>
#include <cstdlib>
#include <stdlib.h>
#include <ios>
#include <errno.h>

#include "cue2pb/parser.h"
#include "cue2pb/unparser.h"
#include "util/file.h"
#include "util/status_macros.h"
#include "cue2pb/text_format.h"
#include "cue2pb/cuesheet.pb.h"
#include "absl/types/span.h"
#include "absl/strings/string_view.h"
#include "absl/strings/str_cat.h"
#include "absl/status/status.h"
#include "absl/debugging/symbolize.h"
#include "absl/debugging/failure_signal_handler.h"
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

absl::Status ProtoToCue(absl::string_view protofile) {
  bool textformat = absl::GetFlag(FLAGS_textformat);

  auto mode = std::ios::in;
  if (!textformat) {
    mode |= std::ios::binary;
  }

  ASSIGN_OR_RETURN(std::ifstream istrm, util::OpenInputFile(protofile, mode));

  Cuesheet cuesheet;
  if (textformat) {
    ASSIGN_OR_RETURN(cuesheet, CuesheetFromTextProto(&istrm));
  } else {
    if (!cuesheet.ParseFromIstream(&istrm)) {
      return absl::UnknownError("Failed to parse binary proto");
    }
  }

  RETURN_IF_ERROR(UnparseCuesheet(cuesheet, &std::cout));

  return absl::OkStatus();
}

absl::Status CueToProto(absl::string_view cuefile) {
  bool textformat = absl::GetFlag(FLAGS_textformat);

  auto mode = std::ios::in;
  if (!textformat) {
    mode |= std::ios::binary;
  }

  ASSIGN_OR_RETURN(std::ifstream istrm, util::OpenInputFile(cuefile, mode));
  ASSIGN_OR_RETURN(Cuesheet cuesheet, ParseCuesheet(&istrm));

  if (textformat) {
    OstreamOutputStream cout_os(&std::cout);
    if (!TextFormat::Print(cuesheet, &cout_os)) {
      return absl::UnknownError("Failed to print cuesheet");
    }
  } else {
    if (!cuesheet.SerializeToOstream(&std::cout)) {
      return absl::UnknownError("Failed to serialize binary proto");
    }
  }

  return absl::OkStatus();
}

absl::Status Main(absl::Span<absl::string_view> args) {
  if (args.size() != 1) {
    return absl::InvalidArgumentError("No cuefile specified or too many arguments");
  }

  if (absl::GetFlag(FLAGS_proto_to_cue)) {
    return ProtoToCue(args[0]);
  } else {
    return CueToProto(args[0]);
  }
}

}  // namespace cue2pb

int main(int argc, char *argv[]) {
  absl::InitializeSymbolizer(argv[0]);
  absl::InstallFailureSignalHandler(/*options=*/{});

  absl::SetProgramUsageMessage(
      absl::StrCat(
        "Convert CD cuesheets to/from protobufs.\n",
        "Usage: ", program_invocation_short_name, " [flags] cuefile"));
  absl::FlagsUsageConfig config;
  config.contains_help_flags = [](absl::string_view path) {
    return path == "cue2pb/main.cc";
  };
  absl::SetFlagsUsageConfig(std::move(config));
  std::vector<char*> cargs = absl::ParseCommandLine(argc, argv);
  std::vector<absl::string_view> args(cargs.begin(), cargs.end());

  if (absl::Status err = cue2pb::Main(absl::MakeSpan(args).subspan(1));
      !err.ok()) {
    if (err.code() == absl::StatusCode::kInvalidArgument) {
      std::cerr << absl::ProgramUsageMessage() << std::endl << std::endl;
    }
    std::cerr << err << std::endl;
    return static_cast<int>(err.code());
  }
  return EXIT_SUCCESS;
}
