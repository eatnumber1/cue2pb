#include "cue2pb/parser.h"

#include <string>
#include <cassert>
#include <string_view>
#include <utility>
#include <vector>

#include "absl/algorithm/container.h"
#include "absl/strings/ascii.h"
#include "absl/strings/str_split.h"
#include "absl/strings/numbers.h"
#include "absl/strings/str_format.h"

namespace cue2pb {

using ::rhutil::StatusOr;
using ::rhutil::Status;
using ::rhutil::OkStatus;
using ::rhutil::InvalidArgumentError;

namespace {

StatusOr<int> ParseInt(std::string_view str) {
  int ret = -1;
  if (!absl::SimpleAtoi(str, &ret)) {
    return InvalidArgumentError(
        absl::StrFormat("Could not parse '%s' as an int", str));
  }
  return ret;
}

StatusOr<Cuesheet::File*> MutableLastFile(Cuesheet *cuesheet) {
  int num_files = cuesheet->file_size();
  if (num_files == 0) {
    return InvalidArgumentError("No files (yet) in this cuesheet");
  }
  return cuesheet->mutable_file(num_files - 1);
}

StatusOr<Cuesheet::Track*> MutableLastTrack(Cuesheet *cuesheet) {
  ASSIGN_OR_RETURN(Cuesheet::File *file, MutableLastFile(cuesheet));

  int num_tracks = file->track_size();
  if (num_tracks == 0) {
    return InvalidArgumentError(
        absl::StrFormat("No tracks (yet) in FILE block of '%s'", file->path()));
  }
  return file->mutable_track(num_tracks - 1);
}

StatusOr<std::pair<std::string_view, std::string_view>>
    ParseString(std::string_view cur) {
  assert(cur[0] == '"');

  int closequote_pos = -1;
  for (int i = 1; i < cur.size(); i++) {
    if (cur[i] == '"' && cur[i - 1] != '\\') {
      closequote_pos = i;
      break;
    }
  }

  if (closequote_pos == -1) {
    return InvalidArgumentError(
        absl::StrFormat("Couldn't find a closing quote in: '%s'", cur));
  }

  return std::make_pair(
      cur.substr(1, closequote_pos - 1),
      cur.substr(closequote_pos + 1));
}

// Parses a string that is either quoted (with no trailing characters), or is
// unquoted and consumes the entire input string.
// If an error occurs, str is unmodified.
Status ParseOptionallyQuotedString(std::string_view *str) {
  if ((*str)[0] == '"') {
    ASSIGN_OR_RETURN(auto p, ParseString(*str));
    if (!p.second.empty()) {
      return InvalidArgumentError(
          absl::StrFormat("Trailing garbage after performer: '%s'", p.second));
    }
    *str = p.first;
  }
  return OkStatus();
}

StatusOr<Cuesheet::MSF> ParseMSF(std::string_view cur) {
  Cuesheet::MSF msf;

  std::vector<std::string_view> splits = absl::StrSplit(cur, ':');
  if (splits.size() != 3) {
    return InvalidArgumentError(
        absl::StrFormat("Could not parse '%s' as an MSF", cur));
  }
  auto minute_str = splits[0];
  auto second_str = splits[1];
  auto frame_str = splits[2];

  ASSIGN_OR_RETURN(int32_t minute, ParseInt(minute_str));
  ASSIGN_OR_RETURN(int32_t second, ParseInt(second_str));
  ASSIGN_OR_RETURN(int32_t frame, ParseInt(frame_str));

  msf.set_minute(minute);
  msf.set_second(second);
  msf.set_frame(frame);

  return std::move(msf);
}

Status ParseIndex(std::string_view cur, Cuesheet *cuesheet) {
  using Index = ::cue2pb::Cuesheet::Index;

  ASSIGN_OR_RETURN(auto *track, MutableLastTrack(cuesheet));
  Index *index = track->add_index();

  std::pair<std::string_view, std::string_view> splits =
      absl::StrSplit(cur, absl::MaxSplits(' ', 1), absl::SkipEmpty());
  auto indexno_str = splits.first;
  auto msf_str = absl::StripLeadingAsciiWhitespace(splits.second);

  ASSIGN_OR_RETURN(int32_t indexno, ParseInt(indexno_str));
  ASSIGN_OR_RETURN(Cuesheet::MSF msf, ParseMSF(msf_str));

  index->set_number(indexno);
  *index->mutable_position() = std::move(msf);

  return OkStatus();
}

Status ParseFile(std::string_view cur, Cuesheet *cuesheet) {
  using File = ::cue2pb::Cuesheet::File;

  File *file = cuesheet->add_file();

  ASSIGN_OR_RETURN(auto p, ParseString(cur));
  std::string_view path = p.first;
  std::string_view type = absl::StripAsciiWhitespace(p.second);

  file->set_path(std::string(path));

  if (type == "WAVE") {
    file->set_type(File::TYPE_WAVE);
  } else if (type == "MP3") {
    file->set_type(File::TYPE_MP3);
  } else if (type == "AIFF") {
    file->set_type(File::TYPE_AIFF);
  } else if (type == "BINARY") {
    file->set_type(File::TYPE_BINARY);
  } else if (type == "MOTOROLA") {
    file->set_type(File::TYPE_MOTOROLA);
  } else {
    return InvalidArgumentError(
        absl::StrFormat("Unknown file type: '%s'", type));
  }

  return OkStatus();
}

Status ParseTrack(std::string_view cur, Cuesheet *cuesheet) {
  using Track = ::cue2pb::Cuesheet::Track;

  ASSIGN_OR_RETURN(auto *file, MutableLastFile(cuesheet));
  Track *track = file->add_track();

  std::pair<std::string_view, std::string_view> splits =
      absl::StrSplit(cur, absl::MaxSplits(' ', 1), absl::SkipEmpty());
  auto trackno_str = splits.first;
  auto type_str = absl::StripLeadingAsciiWhitespace(splits.second);

  ASSIGN_OR_RETURN(int32_t trackno, ParseInt(trackno_str));
  track->set_number(trackno);

  if (type_str == "AUDIO") {
    track->set_type(Track::TYPE_AUDIO);
  } else if (type_str == "CDG") {
    track->set_type(Track::TYPE_CDG);
  } else if (type_str == "MODE1/2048") {
    track->set_type(Track::TYPE_MODE1_2048);
  } else if (type_str == "MODE1/2352") {
    track->set_type(Track::TYPE_MODE1_2352);
  } else if (type_str == "MODE2/2336") {
    track->set_type(Track::TYPE_MODE2_2336);
  } else if (type_str == "MODE2/2352") {
    track->set_type(Track::TYPE_MODE2_2352);
  } else if (type_str == "CDI_2336") {
    track->set_type(Track::TYPE_CDI_2336);
  } else if (type_str == "CDI_2352") {
    track->set_type(Track::TYPE_CDI_2352);
  } else {
    return InvalidArgumentError(
        absl::StrFormat("Unknown track type: '%s'", type_str));
  }

  return OkStatus();
}

Status ParseComment(std::string_view cur, Cuesheet *cuesheet) {
  // Some comments are special. Ones of the form REM <UPPER_TAG> ... are
  // convention to specify additional tags that are not normally supported in
  // cuesheets. Those are supported explicitly here. All other comments are
  // dropped.

  std::pair<std::string_view, std::string_view> splits =
    absl::StrSplit(cur, absl::MaxSplits(' ', 1), absl::SkipEmpty());
  auto key = splits.first;
  auto value = absl::StripLeadingAsciiWhitespace(splits.second);
  if (value.empty() || !absl::c_all_of(key, &absl::ascii_isupper)) {
    // This is a non-tag comment.
    return OkStatus();
  }

  ParseOptionallyQuotedString(&value).IgnoreError();

  Cuesheet::Tags *tags = nullptr;
  auto track_or = MutableLastTrack(cuesheet);
  if (track_or.ok()) {
    tags = track_or.ValueOrDie()->mutable_tags();
  } else {
    tags = cuesheet->mutable_tags();
  }
  Cuesheet::CommentTag *tag = tags->add_comment_tag();
  tag->set_name(std::string(key));
  tag->set_value(std::string(value));

  return OkStatus();
}

Status ParsePerformer(std::string_view cur, Cuesheet *cuesheet) {
  RETURN_IF_ERROR(ParseOptionallyQuotedString(&cur));

  auto track_or = MutableLastTrack(cuesheet);
  if (track_or.ok()) {
    track_or.ValueOrDie()->mutable_tags()->set_performer(std::string(cur));
  } else {
    cuesheet->mutable_tags()->set_performer(std::string(cur));
  }

  return OkStatus();
}

Status ParseTitle(std::string_view cur, Cuesheet *cuesheet) {
  RETURN_IF_ERROR(ParseOptionallyQuotedString(&cur));

  auto track_or = MutableLastTrack(cuesheet);
  if (track_or.ok()) {
    track_or.ValueOrDie()->mutable_tags()->set_title(std::string(cur));
  } else {
    cuesheet->mutable_tags()->set_title(std::string(cur));
  }

  return OkStatus();
}

Status ParseSongwriter(std::string_view cur, Cuesheet *cuesheet) {
  RETURN_IF_ERROR(ParseOptionallyQuotedString(&cur));

  auto track_or = MutableLastTrack(cuesheet);
  if (track_or.ok()) {
    track_or.ValueOrDie()->mutable_tags()->set_songwriter(std::string(cur));
  } else {
    cuesheet->mutable_tags()->set_songwriter(std::string(cur));
  }

  return OkStatus();
}

Status ParseCatalog(std::string_view cur, Cuesheet *cuesheet) {
  cuesheet->set_catalog(std::string(cur));
  return OkStatus();
}

Status ParseCDTextFile(std::string_view cur, Cuesheet *cuesheet) {
  RETURN_IF_ERROR(ParseOptionallyQuotedString(&cur));
  cuesheet->set_cd_text_file(std::string(cur));
  return OkStatus();
}

Status ParsePregap(std::string_view cur, Cuesheet *cuesheet) {
  ASSIGN_OR_RETURN(auto msf, ParseMSF(cur));
  ASSIGN_OR_RETURN(auto *track, MutableLastTrack(cuesheet));
  *track->mutable_pregap() = std::move(msf);
  return OkStatus();
}

Status ParsePostgap(std::string_view cur, Cuesheet *cuesheet) {
  ASSIGN_OR_RETURN(auto msf, ParseMSF(cur));
  ASSIGN_OR_RETURN(auto *track, MutableLastTrack(cuesheet));
  *track->mutable_postgap() = std::move(msf);
  return OkStatus();
}

Status ParseISRC(std::string_view cur, Cuesheet *cuesheet) {
  ASSIGN_OR_RETURN(auto *track, MutableLastTrack(cuesheet));
  track->set_isrc(std::string(cur));
  return OkStatus();
}

Status ParseFlags(std::string_view cur, Cuesheet *cuesheet) {
  using Track = ::cue2pb::Cuesheet::Track;

  ASSIGN_OR_RETURN(Track *track, MutableLastTrack(cuesheet));

  std::vector<std::string_view> flags =
      absl::StrSplit(cur, ' ', absl::SkipEmpty());
  for (std::string_view flag : flags) {
    if (flag == "DCP") {
      track->add_flag(Track::FLAG_DCP);
    } else if (flag == "4CH") {
      track->add_flag(Track::FLAG_4CH);
    } else if (flag == "PRE") {
      track->add_flag(Track::FLAG_PRE);
    } else {
      return InvalidArgumentError(absl::StrFormat("Unknown flag: '%s'", flag));
    }
  }

  return OkStatus();
}

Status ParseLine(std::string_view line, Cuesheet *cuesheet) {
  line = absl::StripAsciiWhitespace(line);
  if (line.empty()) return OkStatus();

  std::pair<std::string_view, std::string_view> splits =
    absl::StrSplit(line, absl::MaxSplits(' ', 1), absl::SkipEmpty());
  auto command = splits.first;
  auto rest = absl::StripLeadingAsciiWhitespace(splits.second);

  if (command == "CATALOG") {
    return ParseCatalog(rest, cuesheet);
  } else if (command == "CDTEXTFILE") {
    return ParseCDTextFile(rest, cuesheet);
  } else if (command == "FILE") {
    return ParseFile(rest, cuesheet);
  } else if (command == "FLAGS") {
    return ParseFlags(rest, cuesheet);
  } else if (command == "INDEX") {
    return ParseIndex(rest, cuesheet);
  } else if (command == "ISRC") {
    return ParseISRC(rest, cuesheet);
  } else if (command == "PERFORMER") {
    return ParsePerformer(rest, cuesheet);
  } else if (command == "POSTGAP") {
    return ParsePostgap(rest, cuesheet);
  } else if (command == "PREGAP") {
    return ParsePregap(rest, cuesheet);
  } else if (command == "REM") {
    return ParseComment(rest, cuesheet);
  } else if (command == "SONGWRITER") {
    return ParseSongwriter(rest, cuesheet);
  } else if (command == "TITLE") {
    return ParseTitle(rest, cuesheet);
  } else if (command == "TRACK") {
    return ParseTrack(rest, cuesheet);
  }

  return InvalidArgumentError(
      absl::StrFormat("Invalid command: '%s'", command));
}

}  // namespace

StatusOr<Cuesheet> ParseCuesheet(std::istream *input) {
  Cuesheet cuesheet;

  int lineno = 1;
  for (std::string line; std::getline(*input, line); lineno++) {
    Status st = ParseLine(line, &cuesheet);
    if (!st.ok()) {
      return Status(st.code(), absl::StrFormat("Error on line %d: %s", lineno,
                                               st.message()));
    }
  }

  return std::move(cuesheet);
}

}  // namespace cue2pb
