#pragma once

#include <string>

namespace ad::string_util {

consteval std::string join(auto const& ... args) {
  std::string r = "";
  template for (auto arg : {args...}) {
    r += arg;
  }

  return r;
}

}