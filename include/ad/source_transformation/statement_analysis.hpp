#pragma once

#include <ranges>
#include <iostream>

#include "../autograd.h"
#include "../meta.h"
#include "generation_context.hpp"
#include "string_util.hpp"
#include "throw_helper.hpp"

#include "expression_analysis.hpp"

namespace ad {
using std::meta::info;
namespace m = ad::meta;
namespace su = ad::string_util;

struct StatementAnalysis {
  static consteval std::ranges::range_value_t<std::string&> const* generatePrimal(info stmt, GenerationContext& ctx) {
    std::string r = "";
    if (m::is_declaration_statement(stmt)) {
      info v = declared_variable_of(stmt);
      r += ctx.format(su::join(
        m::display_string_of(m::type_of(v)),
        " ",
        m::identifier_of(v),
        " = ",
        ExpressionAnalysis::generatePrimal(initializer_of(v))));
    } else if (m::is_expression_statement(stmt)) {
      r += ctx.format(ExpressionAnalysis::generatePrimal(expression_of(stmt)));
    } else if (m::is_return_statement(stmt)) {
      r += ctx.format(su::join("return ", ExpressionAnalysis::generatePrimal(return_operand_of(stmt))));
    } else {
      throwKind(stmt);
    }

    r += ";\n";
    return std::define_static_string(r);
  }

  static consteval std::ranges::range_value_t<std::string&> const* generateForward(info stmt, GenerationContext& ctx) {
    std::string r = "";
    bool add_primal = true;
    if (m::is_declaration_statement(stmt)) {
      info v = declared_variable_of(stmt);
      r += ctx.format(su::join(
        m::display_string_of(m::type_of(v)),
        " ",
        m::identifier_of(v), "_d",
        " = ",
        ExpressionAnalysis::generateForward(initializer_of(v))));
    } else if (m::is_expression_statement(stmt)) {
      r += ctx.format(ExpressionAnalysis::generateForward(expression_of(stmt)));
    } else if (m::is_return_statement(stmt)) {
      r += ctx.format(su::join(
        "return std::make_pair(",
        ExpressionAnalysis::generatePrimal(return_operand_of(stmt)),
        ", ",
        ExpressionAnalysis::generateForward(return_operand_of(stmt)),
        ")"));
      add_primal = false;
    } else {
      throwKind(stmt);
    }

    r += ";\n";
    if (add_primal) {
      r += generatePrimal(stmt, ctx);
    }
    return std::define_static_string(r);
  }

  static consteval std::ranges::range_value_t<std::string&> const* generateStore(info stmt, GenerationContext& ctx) {
    std::string r = "";
    if (m::is_declaration_statement(stmt)) {
      // Nothing
    } else if (m::is_expression_statement(stmt)) {
      std::vector<std::string> overwrites = ExpressionAnalysis::listOverwrites(expression_of(stmt));
      for (std::string cur : overwrites) {
        r += ctx.format(su::join("store(", cur, ");\n"));
      }
    } else if (m::is_return_statement(stmt)) {
      // Nothing
    } else {
      throwKind(stmt);
    }
    return std::define_static_string(r);
  }

  static consteval std::ranges::range_value_t<std::string&> const* generateRestore(info stmt, GenerationContext& ctx) {
    std::string r = "";
    if (m::is_declaration_statement(stmt)) {
      // Nothing
    } else if (m::is_expression_statement(stmt)) {
      std::vector<std::string> overwrites = ExpressionAnalysis::listOverwrites(expression_of(stmt));
      for (std::string cur : overwrites | std::views::reverse) {
        r += ctx.format(su::join("restore(", cur, ");\n"));
      }
    } else if (m::is_return_statement(stmt)) {
      // Nothing
    } else {
      throwKind(stmt);
    }
    return std::define_static_string(r);
  }

  static consteval std::ranges::range_value_t<std::string&> const* generateReverse(info stmt, GenerationContext& ctx) {
    std::string r = "";
    std::vector<std::string> reverse_stmts = {};
    std::vector<std::string> lhs_values = {};
    if (m::is_declaration_statement(stmt)) {
      info v = declared_variable_of(stmt);
      std::string adj = su::join(m::identifier_of(v), "_b");
      ExpressionAnalysis::generateReverse(initializer_of(v), adj, reverse_stmts, lhs_values);
      lhs_values.push_back(adj);
    } else if (m::is_expression_statement(stmt)) {
      ExpressionAnalysis::generateReverse(expression_of(stmt), "", reverse_stmts, lhs_values);
    } else if (m::is_return_statement(stmt)) {
      std::string adj = "ret_b";
      ExpressionAnalysis::generateReverse(return_operand_of(stmt), adj, reverse_stmts, lhs_values);
      lhs_values.push_back(adj);
    } else {
      throwKind(stmt);
    }

    for (std::string cur : reverse_stmts) {
      r += ctx.format(su::join(cur, ";\n"));
    }
    for (std::string cur : lhs_values) {
      r += ctx.format(su::join(cur, " = 0.0;\n"));
    }

    return std::define_static_string(r);
  }

  static consteval std::ranges::range_value_t<std::string&> const* generateReverseDecl(info stmt, GenerationContext& ctx) {
    std::string r = "";
    if (m::is_declaration_statement(stmt)) {
      info v = declared_variable_of(stmt);
      r += ctx.format(su::join(
        m::display_string_of(m::type_of(v)),
        " ",
        m::identifier_of(v),
        "_b = 0.0;\n"));
    } else if (m::is_expression_statement(stmt)) {
      // Nothing
    } else if (m::is_return_statement(stmt)) {
      // Nothing
    } else {
      throwKind(stmt);
    }

    return std::define_static_string(r);
  }
};

}