#include <fstream>
#include <string>
#include <iostream>
#include <sysexits.h>
#include <cstdlib>
#include <stdlib.h>
#include <ios>

#include <glib.h>

#include "cue2pb/parser.h"
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

bool ProtoToCue(absl::string_view protofile, GError **error) {
  bool textformat = absl::GetFlag(FLAGS_textformat);

  auto mode = std::ios::in;
  if (!textformat) {
    mode |= std::ios::binary;
  }

  std::ifstream istrm = OpenInputFile(protofile, mode, error);
  if (!istrm.is_open()) return false;

  Cuesheet cuesheet;
  if (textformat) {
    auto c = ParseCuesheetFromTextProto(&istrm, error);
    if (!c) return false;
    cuesheet = std::move(*c);
  } else {
    g_assert(false);  // unimplemented
  }

  if (!UnparseCuesheet(cuesheet, &std::cout, error)) return false;

  return true;
}

bool CueToProto(absl::string_view cuefile, GError **error) {
  g_assert(absl::GetFlag(FLAGS_textformat));

  std::ifstream istrm = OpenInputFile(cuefile, error);
  if (!istrm.is_open()) return false;

  absl::optional<Cuesheet> cuesheet = ParseCuesheet(&istrm, error);
  if (!cuesheet) return false;

  OstreamOutputStream cout_os(&std::cout);
  if (!TextFormat::Print(*cuesheet, &cout_os)) {
    SetError(error, cue2pb::ERR_UNKNOWN, "Failed to print cuesheet");
    return false;
  }

  return true;
}

bool Main(absl::Span<absl::string_view> args, GError **error) {
  if (args.size() != 1) {
    SetError(error, cue2pb::ERR_USAGE,
             "No cuefile specified or too many arguments");
    return false;
  }

  if (absl::GetFlag(FLAGS_proto_to_cue)) {
    return ProtoToCue(args[0], error);
  } else {
    return CueToProto(args[0], error);
  }
}

}  // namespace cue2pb

int main(int argc, char *argv[]) {
  absl::SetProgramUsageMessage(
      absl::StrCat(
        "Convert CD cuesheets to/from protobufs.\nUsage: ", getprogname(),
        " cuefile"));
  absl::FlagsUsageConfig config;
  config.contains_help_flags = [](absl::string_view path) {
    return path == "cue2pb/main.cc";
  };
  absl::SetFlagsUsageConfig(std::move(config));
  std::vector<char*> cargs = absl::ParseCommandLine(argc, argv);
  std::vector<absl::string_view> args(cargs.begin(), cargs.end());

  GError *err = nullptr;
  if (!cue2pb::Main(absl::MakeSpan(args).subspan(1), &err)) {
    if (err->domain == CUE2PB_ERROR && err->code == cue2pb::ERR_USAGE) {
      std::cerr << absl::ProgramUsageMessage() << std::endl << std::endl;
    }
    std::cerr << err->message << std::endl;
    return err->code;
  }
  return EXIT_SUCCESS;
}
