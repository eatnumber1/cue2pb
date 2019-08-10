#include "cue2pb/unparser.h"

#include <utility>
#include <string>

#include "absl/strings/match.h"
#include "absl/strings/string_view.h"
#include "absl/strings/str_format.h"
#include "absl/strings/str_join.h"
#include "absl/types/optional.h"

namespace cue2pb {
namespace {

std::string QuoteIfNeeded(absl::string_view s) {
  if (s.empty() || absl::StrContains(s, " ")) {
    return "\"" + std::string(s) + "\"";
  }
  return std::string(s);
}

absl::optional<std::string> FileTypeToString(
    Cuesheet::File::Type type, GError **error) {
  using File = ::cue2pb::Cuesheet::File;
  std::string type_str;
  switch (type) {
    case File::TYPE_WAVE:
      type_str = "WAVE";
      break;
    case File::TYPE_MP3:
      type_str = "MP3";
      break;
    case File::TYPE_AIFF:
      type_str = "AIFF";
      break;
    case File::TYPE_BINARY:
      type_str = "BINARY";
      break;
    case File::TYPE_MOTOROLA:
      type_str = "MOTOROLA";
      break;
    default:
      SetError(
          error, cue2pb::ERR_SYNTAX, "Unknown file type: '%s'",
          File::Type_Name(type));
      return absl::nullopt;
  }

  return absl::make_optional(std::move(type_str));
}

absl::optional<std::string> TrackTypeToString(
    Cuesheet::Track::Type type, GError **error) {
  using Track = ::cue2pb::Cuesheet::Track;
  std::string type_str;
  switch (type) {
    case Track::TYPE_AUDIO:
      type_str = "AUDIO";
      break;
    case Track::TYPE_CDG:
      type_str = "CDG";
      break;
    case Track::TYPE_MODE1_2048:
      type_str = "MODE1/2048";
      break;
    case Track::TYPE_MODE1_2352:
      type_str = "MODE1/2352";
      break;
    case Track::TYPE_MODE2_2336:
      type_str = "MODE2/2336";
      break;
    case Track::TYPE_MODE2_2352:
      type_str = "MODE2/2352";
      break;
    case Track::TYPE_CDI_2336:
      type_str = "CDI/2336";
      break;
    case Track::TYPE_CDI_2352:
      type_str = "CDI/2352";
      break;
    default:
      SetError(
          error, cue2pb::ERR_SYNTAX, "Unknown track type: '%s'",
          Track::Type_Name(type));
      return absl::nullopt;
  }

  return absl::make_optional(std::move(type_str));
}

absl::optional<std::string> FlagToString(
    Cuesheet::Track::Flag flag, GError **error) {
  using Track = ::cue2pb::Cuesheet::Track;
  std::string flag_str;
  switch (flag) {
    case Track::FLAG_DCP:
      flag_str = "DCP";
      break;
    case Track::FLAG_4CH:
      flag_str = "4CH";
      break;
    case Track::FLAG_PRE:
      flag_str = "PRE";
      break;
    default:
      SetError(
          error, cue2pb::ERR_SYNTAX, "Unknown track flag: '%s'",
          Track::Flag_Name(flag));
      return absl::nullopt;
  }

  return absl::make_optional(std::move(flag_str));
}

std::string MSFToString(const Cuesheet::MSF &msf) {
  return absl::StrFormat("%02d:%02d:%02d", msf.minute(), msf.second(),
                         msf.frame());
}

bool IsZeroMSF(const Cuesheet::MSF &msf) {
  return msf.minute() == 0 && msf.second() == 0 && msf.frame() == 0;
}

bool UnparseTags(
    const Cuesheet::Tags &tags, std::ostream *output, GError **error) {
  for (const Cuesheet::CommentTag &tag : tags.comment_tag()) {
    *output << "REM " << tag.name() << " " << QuoteIfNeeded(tag.value())
            << std::endl;
  }

  if (!tags.title().empty()) {
    *output << "TITLE " << QuoteIfNeeded(tags.title()) << std::endl;
  }

  if (!tags.performer().empty()) {
    *output << "PERFORMER " << QuoteIfNeeded(tags.performer())
            << std::endl;
  }

  if (!tags.songwriter().empty()) {
    *output << "SONGWRITER " << QuoteIfNeeded(tags.songwriter())
            << std::endl;
  }
  return true;
}

bool UnparseIndex(const Cuesheet::Index &index, std::ostream *output,
                  GError **error) {
  *output << "INDEX " << absl::StrFormat("%02d", index.number()) << " "
          << MSFToString(index.position()) << std::endl;
  return true;
}

bool UnparseTrack(const Cuesheet::Track &track, std::ostream *output,
                  GError **error) {
  auto type = TrackTypeToString(track.type(), error);
  if (!type) return false;

  *output << "TRACK " << absl::StrFormat("%02d", track.number()) << " " << *type
          << std::endl;

  if (!UnparseTags(track.tags(), output, error)) return false;

  if (!track.isrc().empty()) {
    *output << "ISRC " << track.isrc() << std::endl;
  }

  std::vector<std::string> flags;
  for (int i = 0; i < track.flag_size(); i++) {
    auto flag_str = FlagToString(track.flag(i), error);
    if (!flag_str) return false;
    flags.emplace_back(std::move(*flag_str));
  }
  if (!flags.empty()) {
    *output << "FLAGS " << absl::StrJoin(flags, " ") << std::endl;
  }

  if (!IsZeroMSF(track.pregap())) {
    *output << "PREGAP " << MSFToString(track.pregap()) << std::endl;
  }

  if (!IsZeroMSF(track.postgap())) {
    *output << "POSTGAP " << MSFToString(track.postgap()) << std::endl;
  }

  for (const Cuesheet::Index &index : track.index()) {
    if (!UnparseIndex(index, output, error)) return false;
  }

  return true;
}

bool UnparseFile(const Cuesheet::File &file, std::ostream *output,
                 GError **error) {
  auto type = FileTypeToString(file.type(), error);
  if (!type) return false;

  *output << "FILE " << QuoteIfNeeded(file.path()) << " " << *type << std::endl;

  for (const Cuesheet::Track &track : file.track()) {
    if (!UnparseTrack(track, output, error)) return false;
  }

  return true;
}

}  // namespace

bool UnparseCuesheet(const Cuesheet &cuesheet, std::ostream *output,
                     GError **error) {
  if (!UnparseTags(cuesheet.tags(), output, error)) return false;

  if (!cuesheet.catalog().empty()) {
    *output << "CATALOG " << cuesheet.catalog() << std::endl;
  }

  if (!cuesheet.cd_text_file().empty()) {
    *output << "CDTEXTFILE " << QuoteIfNeeded(cuesheet.cd_text_file())
            << std::endl;
  }

  for (const Cuesheet::File &file : cuesheet.file()) {
    if (!UnparseFile(file, output, error)) return false;
  }

  return true;
}

}  // namespace cue2pb
