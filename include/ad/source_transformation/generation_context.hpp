#pragma once

#include <string>

namespace ad {

struct GenerationContext {
  std::string indent = {};

  consteval std::string format(std::string line) {
    return indent + line;
  }

  consteval void addIndent() {
    indent += "  ";
  }

  consteval void removeIndent() {
    indent = indent.substr(2);
  }
};

}