#pragma once

#include "../meta.h"

#include "expression_analysis.hpp"
#include "statement_analysis.hpp"
#include "function_analysis.hpp"

namespace ad {
using std::meta::info;
namespace m = ad::meta;
namespace su = ad::string_util;

struct Ignore {};

#if __has_feature(annotation_attributes)
consteval bool hasIgnore(info o) {
  return 0 != std::meta::annotations_of_with_type(o, ^^ad::Ignore).size();
}
#endif


template<info o, typename Stream>
void handleNamespacePrimal(Stream& out) {
  static constexpr auto members = std::define_static_array(ad::m::members_of( o, ad::m::access_context::unchecked() ));

  template for (constexpr auto member : members) {
    if (is_function(member)) { // TODO: Add ignore when attributes are available
      out << ad::FunctionAnalysis::generatePrimal(member) << std::endl;
    }
  }
}

template<info o, typename Stream>
void handleNamespaceForward(Stream& out) {
  static constexpr auto members = std::define_static_array(ad::m::members_of( o, ad::m::access_context::unchecked() ));

  template for (constexpr auto member : members) {
    if (is_function(member)) { // TODO: Add ignore when attributes are available
      out << ad::FunctionAnalysis::generateForward(member) << std::endl;
    }
  }
}

template<info o, typename Stream>
void handleNamespaceReverse(Stream& out) {
  static constexpr auto members = std::define_static_array(ad::m::members_of( o, ad::m::access_context::unchecked() ));

  template for (constexpr auto member : members) {
    if (is_function(member)) { // TODO: Add ignore when attributes are available
      out << ad::FunctionAnalysis::generateReverseJoined(member) << std::endl;
    }
  }
}

}

