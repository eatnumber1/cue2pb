# cue2pb: Cuesheet 2 Protobuf
cue2pb is a tool and library for converting from a [Cuesheet] to a purpose built
protobuf: [cuesheet.proto].

The primary intent of this code is to provide the [cuesheet.proto], and a way to
get to and from said protobuf.

That said, the cuesheet parser and unparser are written in C++, and are
available either as a C++ library in [parser.h] and [unparser.h]; or if working
in another language or interactively, the [cue2pb] executable can be invoked as
a subprocess and its output captured.

Here's some examples of using `cue2pb` interactively:

Convert a cuesheet to a binary protobuf.
```
$ cue2pb foo.cue
```

Convert a cuesheet to a textual protobuf.
```
$ cue2pb --textformat foo.cue
```

Convert a binary protobuf to a cuesheet.
```
$ cue2pb --proto_to_cue foo.cuepb
```

Convert a textual protobuf to a cuesheet.
```
$ cue2pb --textformat --proto_to_cue foo.textproto
```

If you wish to work with Cuesheet protos from another language, feel free to
send pull requests adding Bazel build rules to generate the protobuf for
additional languages as desired.

[Cuesheet]: https://en.wikipedia.org/wiki/Cue_sheet_(computing)
[cuesheet.proto]: cue2pb/cuesheet.proto
[cue2pb]: cue2pb/main.cc
[parser.h]: cue2pb/parser.h
[unparser.h]: cue2pb/unparser.h
