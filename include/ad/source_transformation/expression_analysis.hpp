#pragma once

#include <sstream>
#include <fstream>
#include <ranges>
#include <iostream>

#include "../autograd.h"
#include "../meta.h"
#include "operator_info.hpp"
#include "string_util.hpp"
#include "throw_helper.hpp"

namespace ad {
using std::meta::info;
namespace m = ad::meta;
namespace su = ad::string_util;

struct ExpressionAnalysis {
  static consteval std::ranges::range_value_t<std::string&> const* generatePrimal(info e) {
    info exp = ad::detail::stripCasts(e); // TODO: Handle casts.

    std::string r = "";
    if (m::is_operator_expression(exp)) {
      auto operands = operands_of(exp);

      if (operands.size() != 2) {
        throw "Only binary handling for now"; // TODO: Implement
      }

      OperatorInfo op = getOperatorInfo(m::expression_operator_of(exp));
      r += su::join(
        ExpressionAnalysis::generatePrimal(operands[0]),
        " ",
        op.symbol,
        " ",
        ExpressionAnalysis::generatePrimal(operands[1]));
    } else if (m::is_literal(exp)) {
      r += m::display_string_of(constant_of(exp));
    } else if (m::is_id_expression(exp)) {
      r += identifier_of(declaration_of(exp));
    } else {
      throwKind(exp);
    }

    return std::define_static_string(r);
  }

  static consteval std::ranges::range_value_t<std::string&> const* generateForward(info e) {
    info exp = ad::detail::stripCasts(e); // TODO: Handle casts.

    std::string r = "";
    if (m::is_operator_expression(exp)) {
      auto operands = operands_of(exp);

      if (operands.size() != 2) {
        throw "Only binary handling for now"; // TODO: Implement
      }

      OperatorInfo op = getOperatorInfo(m::expression_operator_of(exp));

      if (op.kind == OperatorKind::Assign) {
        r += su::join(
          ExpressionAnalysis::generateForward(operands[0]),
          " ",
          op.symbol,
          " ",
          ExpressionAnalysis::generateForward(operands[1]));
      } else if (op.kind == OperatorKind::Mul) {
        r += su::join(
          "(",
          ExpressionAnalysis::generatePrimal(operands[0]),
          ") * (",
          ExpressionAnalysis::generateForward(operands[1]),
          ") + (",
          ExpressionAnalysis::generateForward(operands[0]),
          ") * (",
          ExpressionAnalysis::generatePrimal(operands[1]),
          ")");
      } else {
        throw "Unkown operator";
      }
    } else if (m::is_literal(exp)) {
      r += "0.0";
    } else if (m::is_id_expression(exp)) {
      r += su::join(identifier_of(declaration_of(exp)), "_d");
    } else {
      throwKind(exp);
    }

    return std::define_static_string(r);
  }

  static consteval std::vector<std::string> listOverwrites(info e) {
    info exp = ad::detail::stripCasts(e); // TODO: Handle casts.

    std::vector<std::string> list = {};
    if (m::is_operator_expression(exp)) {
      auto operands = operands_of(exp);

      if (operands.size() != 2) {
        throw "Only binary handling for now"; // TODO: Implement
      }

      OperatorInfo op = getOperatorInfo(m::expression_operator_of(exp));
      if (op.kind == OperatorKind::Assign) {
        list.push_back(ExpressionAnalysis::generatePrimal(operands[0]));
      }
    } else if (m::is_literal(exp)) {
      // Nothing
    } else if (m::is_id_expression(exp)) {
      // Nothing
    } else {
      throwKind(exp);
    }

    return list;
  }

  static consteval void generateReverse(info e, std::string adj, std::vector<std::string>& rev_updates, std::vector<std::string>& lhs_values) {
    info exp = ad::detail::stripCasts(e); // TODO: Handle casts.

    if (m::is_operator_expression(exp)) {
      auto operands = operands_of(exp);

      if (operands.size() != 2) {
        throw "Only binary handling for now"; // TODO: Implement
      }

      OperatorInfo op = getOperatorInfo(m::expression_operator_of(exp));

      if (op.kind == OperatorKind::Assign) {
        if (!adj.empty()) {
          throw "Adjoint needs to be zero for assignment operators.";
        }

        std::string adj = su::join(generatePrimal(operands[0]), "_b"); // TODO: Remove shortcut that might fail  e.g. a[0] -> a[0]_b
        generateReverse(operands[1], adj, rev_updates, lhs_values);
        lhs_values.push_back(adj);
      } else if (op.kind == OperatorKind::Mul) {
        generateReverse(operands[0], su::join("(", generatePrimal(operands[1]), ") * ", adj), rev_updates, lhs_values);
        generateReverse(operands[1], su::join("(", generatePrimal(operands[0]), ") * ", adj), rev_updates, lhs_values);
      } else {
        throw "Unkown operator";
      }
    } else if (m::is_literal(exp)) {
      // Nothing
    } else if (m::is_id_expression(exp)) {
      rev_updates.push_back(su::join(m::identifier_of(declaration_of(exp)), "_b += ", adj));
    } else {
      throwKind(exp);
    }
  }
};
}