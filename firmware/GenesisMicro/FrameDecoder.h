#pragma once
#include <stdint.h>
#include <stddef.h>
// Bounded stream assembly. Complete JSON objects also work without newline.
class FrameDecoder {
public:
  static constexpr size_t Capacity = 4096;
  char bytes[Capacity + 1]{};
  size_t used = 0;
  int depth = 0;
  bool quoted = false, escaped = false;
  void reset() { used = 0; depth = 0; quoted = escaped = false; }
  template <typename Handler>
  bool receive(const uint8_t *report, size_t size, Handler handler) {
    if (size != 63 || report[0] != 2 || report[1] > 61) { reset(); return false; }
    for (size_t i = 0; i < report[1]; ++i) {
      char c = char(report[i + 2]);
      if (!used && c != '{') continue;
      if (used == Capacity) { reset(); return false; }
      bytes[used++] = c;
      if (quoted) {
        if (escaped) escaped = false;
        else if (c == '\\') escaped = true;
        else if (c == '"') quoted = false;
      } else {
        if (c == '"') quoted = true;
        else if (c == '{' || c == '[') ++depth;
        else if (c == '}' || c == ']') --depth;
        if (depth < 0) { reset(); return false; }
        if (!depth) { bytes[used] = 0; handler(bytes, used); reset(); }
      }
    }
    return true;
  }
};
