#pragma once

#include "../meta.h"

#include "expression_analysis.hpp"
#include "statement_analysis.hpp"
#include "function_analysis.hpp"

namespace ad {
using std::meta::info;
namespace m = ad::meta;
namespace su = ad::string_util;

template<info o, typename Stream>
void handleNamespace(Stream& out) {
  static constexpr auto members = std::define_static_array(ad::m::members_of( o, ad::m::access_context::unchecked() ));

  template for (constexpr auto member : members) {
    if (is_function(member)) {
      out << ad::FunctionAnalysis::generatePrimal(member) << std::endl;
      out << ad::FunctionAnalysis::generateForward(member) << std::endl;
      out << ad::FunctionAnalysis::generateReverseJoined(member) << std::endl;
    }
  }
}

}

