#include "cue2pb/parser.h"

#include <string>
#include <utility>
#include <vector>

#include "absl/algorithm/container.h"
#include "absl/strings/ascii.h"
#include "absl/strings/str_split.h"
#include "absl/strings/numbers.h"

namespace cue2pb {
namespace {

int ParseInt(absl::string_view str, GError **error) {
  int ret = -1;
  if (!absl::SimpleAtoi(str, &ret)) {
    SetError(error, cue2pb::ERR_SYNTAX, "Could not parse '%s' as an int", str);
    return -1;
  }
  return ret;
}

Cuesheet::File *MutableLastFile(Cuesheet *cuesheet, GError **error) {
  int num_files = cuesheet->file_size();
  if (num_files == 0) {
    SetError(error, cue2pb::ERR_SYNTAX, "No files (yet) in this cuesheet");
    return nullptr;
  }
  return cuesheet->mutable_file(num_files - 1);
}

Cuesheet::Track *MutableLastTrack(Cuesheet *cuesheet, GError **error) {
  Cuesheet::File *file = MutableLastFile(cuesheet, error);
  if (!file) return nullptr;

  int num_tracks = file->track_size();
  if (num_tracks == 0) {
    SetError(error, cue2pb::ERR_SYNTAX, "No tracks (yet) in FILE block of '%s'",
             file->path());
    return nullptr;
  }
  return file->mutable_track(num_tracks - 1);
}

absl::optional<std::pair<absl::string_view, absl::string_view>>
    ParseString(absl::string_view cur, GError **error) {
  g_assert(cur[0] == '"');

  int closequote_pos = -1;
  for (int i = 1; i < cur.size(); i++) {
    if (cur[i] == '"' && cur[i - 1] != '\\') {
      closequote_pos = i;
      break;
    }
  }

  if (closequote_pos == -1) {
    SetError(error, cue2pb::ERR_SYNTAX,
             "Couldn't find a closing quote in: '%s'", cur);
    return absl::nullopt;
  }

  return absl::make_optional(
      std::make_pair(
        cur.substr(1, closequote_pos - 1),
        cur.substr(closequote_pos + 1)));
}

// Parses a string that is either quoted (with no trailing characters), or is
// unquoted and consumes the entire input string.
// If an error occurs, str is unmodified.
bool ParseOptionallyQuotedString(absl::string_view *str, GError **error) {
  if ((*str)[0] == '"') {
    auto p = ParseString(*str, error);
    if (!p) return false;
    if (!p->second.empty()) {
      SetError(error, cue2pb::ERR_SYNTAX,
               "Trailing garbage after performer: '%s'", p->second);
      return false;
    }
    *str = p->first;
  }
  return true;
}

absl::optional<Cuesheet::MSF> ParseMSF(absl::string_view cur, GError **error) {
  Cuesheet::MSF msf;

  std::vector<absl::string_view> splits = absl::StrSplit(cur, ':');
  if (splits.size() != 3) {
    SetError(error, cue2pb::ERR_SYNTAX, "Could not parse '%s' as an MSF", cur);
    return absl::nullopt;
  }
  auto minute_str = splits[0];
  auto second_str = splits[1];
  auto frame_str = splits[2];

  GError *err = nullptr;
  int32_t minute = ParseInt(minute_str, &err);
  if (err) {
    g_propagate_error(error, err);
    return absl::nullopt;
  }
  int32_t second = ParseInt(second_str, &err);
  if (err) {
    g_propagate_error(error, err);
    return absl::nullopt;
  }
  int32_t frame = ParseInt(frame_str, &err);
  if (err) {
    g_propagate_error(error, err);
    return absl::nullopt;
  }

  msf.set_minute(minute);
  msf.set_second(second);
  msf.set_frame(frame);

  return absl::make_optional(std::move(msf));
}

bool ParseIndex(absl::string_view cur, Cuesheet *cuesheet, GError **error) {
  using Index = ::cue2pb::Cuesheet::Index;

  auto *track = MutableLastTrack(cuesheet, error);
  if (!track) return false;
  Index *index = track->add_index();

  std::pair<absl::string_view, absl::string_view> splits =
      absl::StrSplit(cur, absl::MaxSplits(' ', 1), absl::SkipEmpty());
  auto indexno_str = splits.first;
  auto msf_str = absl::StripLeadingAsciiWhitespace(splits.second);

  GError *err = nullptr;
  int32_t indexno = ParseInt(indexno_str, &err);
  if (err) {
    g_propagate_error(error, err);
    return false;
  }

  auto msf = ParseMSF(msf_str, error);
  if (!msf) return false;

  index->set_number(indexno);
  *index->mutable_position() = std::move(*msf);

  return true;
}

bool ParseFile(absl::string_view cur, Cuesheet *cuesheet, GError **error) {
  using File = ::cue2pb::Cuesheet::File;

  File *file = cuesheet->add_file();

  auto p = ParseString(cur, error);
  if (!p) return false;
  absl::string_view path = p->first;
  absl::string_view type = absl::StripAsciiWhitespace(p->second);

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
    SetError(error, cue2pb::ERR_SYNTAX,
             "Unknown file type: '%s'", type);
    return false;
  }

  return true;
}

bool ParseTrack(absl::string_view cur, Cuesheet *cuesheet, GError **error) {
  using Track = ::cue2pb::Cuesheet::Track;

  auto *file = MutableLastFile(cuesheet, error);
  if (!file) return false;
  Track *track = file->add_track();

  std::pair<absl::string_view, absl::string_view> splits =
      absl::StrSplit(cur, absl::MaxSplits(' ', 1), absl::SkipEmpty());
  auto trackno_str = splits.first;
  auto type_str = absl::StripLeadingAsciiWhitespace(splits.second);

  GError *err = nullptr;
  int32_t trackno = ParseInt(trackno_str, &err);
  if (err) {
    g_propagate_error(error, err);
    return false;
  }
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
    SetError(error, cue2pb::ERR_SYNTAX, "Unknown track type: '%s'", type_str);
    return false;
  }

  return true;
}

bool ParseComment(absl::string_view cur, Cuesheet *cuesheet, GError **error) {
  // Some comments are special. Ones of the form REM <UPPER_TAG> ... are
  // convention to specify additional tags that are not normally supported in
  // cuesheets. Those are supported explicitly here. All other comments are
  // dropped.

  std::pair<absl::string_view, absl::string_view> splits =
    absl::StrSplit(cur, absl::MaxSplits(' ', 1), absl::SkipEmpty());
  auto key = splits.first;
  auto value = absl::StripLeadingAsciiWhitespace(splits.second);
  if (value.empty() || !absl::c_all_of(key, &absl::ascii_isupper)) {
    // This is a non-tag comment.
    return true;
  }

  ParseOptionallyQuotedString(&value, /*error=*/nullptr);

  Cuesheet::Tags *tags = nullptr;
  auto *track = MutableLastTrack(cuesheet, /*error=*/nullptr);
  if (track) {
    tags = track->mutable_tags();
  } else {
    tags = cuesheet->mutable_tags();
  }
  Cuesheet::CommentTag *tag = tags->add_comment_tag();
  tag->set_name(std::string(key));
  tag->set_value(std::string(value));

  return true;
}

bool ParsePerformer(absl::string_view cur, Cuesheet *cuesheet, GError **error) {
  if (!ParseOptionallyQuotedString(&cur, error)) return false;

  auto *track = MutableLastTrack(cuesheet, /*error=*/nullptr);
  if (track) {
    track->mutable_tags()->set_performer(std::string(cur));
  } else {
    cuesheet->mutable_tags()->set_performer(std::string(cur));
  }

  return true;
}

bool ParseTitle(absl::string_view cur, Cuesheet *cuesheet, GError **error) {
  if (!ParseOptionallyQuotedString(&cur, error)) return false;

  auto *track = MutableLastTrack(cuesheet, /*error=*/nullptr);
  if (track) {
    track->mutable_tags()->set_title(std::string(cur));
  } else {
    cuesheet->mutable_tags()->set_title(std::string(cur));
  }

  return true;
}

bool ParseSongwriter(absl::string_view cur, Cuesheet *cuesheet,
                     GError **error) {
  if (!ParseOptionallyQuotedString(&cur, error)) return false;

  auto *track = MutableLastTrack(cuesheet, /*error=*/nullptr);
  if (track) {
    track->mutable_tags()->set_songwriter(std::string(cur));
  } else {
    cuesheet->mutable_tags()->set_songwriter(std::string(cur));
  }

  return true;
}

bool ParseCatalog(absl::string_view cur, Cuesheet *cuesheet, GError **error) {
  cuesheet->set_catalog(std::string(cur));
  return true;
}

bool ParseCDTextFile(absl::string_view cur, Cuesheet *cuesheet,
                     GError **error) {
  if (!ParseOptionallyQuotedString(&cur, error)) return false;
  cuesheet->set_cd_text_file(std::string(cur));
  return true;
}

bool ParsePregap(absl::string_view cur, Cuesheet *cuesheet, GError **error) {
  auto msf = ParseMSF(cur, error);
  if (!msf) return false;

  auto *track = MutableLastTrack(cuesheet, error);
  if (!track) return false;

  *track->mutable_pregap() = std::move(*msf);

  return true;
}

bool ParsePostgap(absl::string_view cur, Cuesheet *cuesheet, GError **error) {
  auto msf = ParseMSF(cur, error);
  if (!msf) return false;

  auto *track = MutableLastTrack(cuesheet, error);
  if (!track) return false;

  *track->mutable_postgap() = std::move(*msf);

  return true;
}

bool ParseISRC(absl::string_view cur, Cuesheet *cuesheet, GError **error) {
  auto *track = MutableLastTrack(cuesheet, error);
  if (!track) return false;
  track->set_isrc(std::string(cur));
  return true;
}

bool ParseFlags(absl::string_view cur, Cuesheet *cuesheet, GError **error) {
  using Track = ::cue2pb::Cuesheet::Track;

  Track *track = MutableLastTrack(cuesheet, error);
  if (!track) return false;

  std::vector<absl::string_view> flags =
      absl::StrSplit(cur, ' ', absl::SkipEmpty());
  for (absl::string_view flag : flags) {
    if (flag == "DCP") {
      track->add_flag(Track::FLAG_DCP);
    } else if (flag == "4CH") {
      track->add_flag(Track::FLAG_4CH);
    } else if (flag == "PRE") {
      track->add_flag(Track::FLAG_PRE);
    } else {
      SetError(error, cue2pb::ERR_SYNTAX, "Unknown flag: '%s'", flag);
      return false;
    }
  }

  return true;
}

bool ParseLine(absl::string_view line, Cuesheet *cuesheet, GError **error) {
  line = absl::StripAsciiWhitespace(line);
  if (line.empty()) return true;

  std::pair<absl::string_view, absl::string_view> splits =
    absl::StrSplit(line, absl::MaxSplits(' ', 1), absl::SkipEmpty());
  auto command = splits.first;
  auto rest = absl::StripLeadingAsciiWhitespace(splits.second);

  if (command == "CATALOG") {
    return ParseCatalog(rest, cuesheet, error);
  } else if (command == "CDTEXTFILE") {
    return ParseCDTextFile(rest, cuesheet, error);
  } else if (command == "FILE") {
    return ParseFile(rest, cuesheet, error);
  } else if (command == "FLAGS") {
    return ParseFlags(rest, cuesheet, error);
  } else if (command == "INDEX") {
    return ParseIndex(rest, cuesheet, error);
  } else if (command == "ISRC") {
    return ParseISRC(rest, cuesheet, error);
  } else if (command == "PERFORMER") {
    return ParsePerformer(rest, cuesheet, error);
  } else if (command == "POSTGAP") {
    return ParsePostgap(rest, cuesheet, error);
  } else if (command == "PREGAP") {
    return ParsePregap(rest, cuesheet, error);
  } else if (command == "REM") {
    return ParseComment(rest, cuesheet, error);
  } else if (command == "SONGWRITER") {
    return ParseSongwriter(rest, cuesheet, error);
  } else if (command == "TITLE") {
    return ParseTitle(rest, cuesheet, error);
  } else if (command == "TRACK") {
    return ParseTrack(rest, cuesheet, error);
  }

  SetError(error, cue2pb::ERR_SYNTAX, "Invalid command: '%s'", command);
  return false;
}

}  // namespace

absl::optional<Cuesheet> ParseCuesheet(std::istream *input, GError **error) {
  Cuesheet cuesheet;

  int lineno = 1;
  for (std::string line; std::getline(*input, line); lineno++) {
    if (!ParseLine(line, &cuesheet, error)) {
      g_prefix_error(error, "Error on line %d: ", lineno);
      return absl::nullopt;
    }
  }

  return absl::make_optional(std::move(cuesheet));
}

}  // namespace cue2pb
