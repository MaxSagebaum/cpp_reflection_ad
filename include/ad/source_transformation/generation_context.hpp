#pragma once

#include <string>

namespace ad {

struct GenerationContext {
  std::string indent = {};
  int temporary_counter = 0;

  std::string statements = {};

  consteval std::string generateTemporary() {
    temporary_counter += 1;
    // TODO: use to_string: return "temp_" + std::to_string(temporary_counter);
    return "temp_" + int_to_string(temporary_counter);
  }
  consteval std::string format(std::string line) {
    return indent + line;
  }

  consteval void addStatement(std::string stmt) {
    statements += format(stmt) + "\n";
  }

  consteval std::string flushStatements() {
    std::string r = {};
    std::swap(r, statements);
    return r;
  }

  consteval void addIndent() {
    indent += "  ";
  }

  consteval void removeIndent() {
    indent = indent.substr(2);
  }

private:
  consteval std::string int_to_string(int num)  {
    std::array<char, 12> buf{};
    auto [ptr, ec] = std::to_chars(buf.data(), buf.data() + buf.size() - 1, num);

    if (ec != std::errc{}) {
      throw "conversion failed";
    }

    std::string r = {};
    for (auto cur = buf.begin(); cur != ptr; cur++) {
      r += *cur;
    }

    return r;
  }
};

}