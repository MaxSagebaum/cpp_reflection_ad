#pragma once

#include <sstream>
#include <fstream>
#include <ranges>
#include <iostream>
#include <meta>

#include "throw_helper.hpp"

#include "../autograd.h"

using namespace std::meta;

consteval std::string join(auto const& ... args) {
  std::string r = "";
  template for (auto arg : {args...}) {
    r += arg;
  }

  return r;
}

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

enum class OperatorKind {
  Assign,
  Mul
};

struct OperatorInfo {
  OperatorKind kind;
  std::string symbol;
};

consteval OperatorInfo getOperatorInfo(operators op) {
  if (op == operators::op_equals) {
    return {OperatorKind::Assign, "="};
  } else if (op == operators::op_star) {
    return {OperatorKind::Mul, "*"};
  } else {
    throw "Unknown operator";
  }
}

struct ExpressionAnalysis {
  static consteval std::ranges::range_value_t<std::string&> const* generatePrimal(info e) {
    info exp = ad::detail::stripCasts(e); // TODO: Handle casts.

    std::string r = "";
    if (is_operator_expression(exp)) {
      auto operands = operands_of(exp);

      if (operands.size() != 2) {
        throw "Only binary handling for now"; // TODO: Implement
      }

      OperatorInfo op = getOperatorInfo(expression_operator_of(exp));
      r += join(
        ExpressionAnalysis::generatePrimal(operands[0]),
        " ",
        op.symbol,
        " ",
        ExpressionAnalysis::generatePrimal(operands[1]));
    } else if (is_literal(exp)) {
      r += display_string_of(constant_of(exp));
    } else if (is_id_expression(exp)) {
      r += identifier_of(declaration_of(exp));
    } else {
      throwKind(exp);
    }

    return std::define_static_string(r);
  }

  static consteval std::ranges::range_value_t<std::string&> const* generateForward(info e) {
    info exp = ad::detail::stripCasts(e); // TODO: Handle casts.

    std::string r = "";
    if (is_operator_expression(exp)) {
      auto operands = operands_of(exp);

      if (operands.size() != 2) {
        throw "Only binary handling for now"; // TODO: Implement
      }

      OperatorInfo op = getOperatorInfo(expression_operator_of(exp));

      if (op.kind == OperatorKind::Assign) {
        r += join(
          ExpressionAnalysis::generateForward(operands[0]),
          " ",
          op.symbol,
          " ",
          ExpressionAnalysis::generateForward(operands[1]));
      } else if (op.kind == OperatorKind::Mul) {
        r += join(
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
    } else if (is_literal(exp)) {
      r += "0.0";
    } else if (is_id_expression(exp)) {
      r += join(identifier_of(declaration_of(exp)), "_d");
    } else {
      throwKind(exp);
    }

    return std::define_static_string(r);
  }

  static consteval std::vector<std::string> listOverwrites(info e) {
    info exp = ad::detail::stripCasts(e); // TODO: Handle casts.

    std::vector<std::string> list = {};
    if (is_operator_expression(exp)) {
      auto operands = operands_of(exp);

      if (operands.size() != 2) {
        throw "Only binary handling for now"; // TODO: Implement
      }

      OperatorInfo op = getOperatorInfo(expression_operator_of(exp));
      if (op.kind == OperatorKind::Assign) {
        list.push_back(ExpressionAnalysis::generatePrimal(operands[0]));
      }
    } else if (is_literal(exp)) {
      // Nothing
    } else if (is_id_expression(exp)) {
      // Nothing
    } else {
      throwKind(exp);
    }

    return list;
  }

  static consteval void generateReverse(info e, std::string adj, std::vector<std::string>& rev_updates, std::vector<std::string>& lhs_values) {
    info exp = ad::detail::stripCasts(e); // TODO: Handle casts.

    if (is_operator_expression(exp)) {
      auto operands = operands_of(exp);

      if (operands.size() != 2) {
        throw "Only binary handling for now"; // TODO: Implement
      }

      OperatorInfo op = getOperatorInfo(expression_operator_of(exp));

      if (op.kind == OperatorKind::Assign) {
        if (!adj.empty()) {
          throw "Adjoint needs to be zero for assignment operators.";
        }

        std::string adj = join(generatePrimal(operands[0]), "_b"); // TODO: Remove shortcut that might fail  e.g. a[0] -> a[0]_b
        generateReverse(operands[1], adj, rev_updates, lhs_values);
        lhs_values.push_back(adj);
      } else if (op.kind == OperatorKind::Mul) {
        generateReverse(operands[0], join("(", generatePrimal(operands[1]), ") * ", adj), rev_updates, lhs_values);
        generateReverse(operands[1], join("(", generatePrimal(operands[0]), ") * ", adj), rev_updates, lhs_values);
      } else {
        throw "Unkown operator";
      }
    } else if (is_literal(exp)) {
      // Nothing
    } else if (is_id_expression(exp)) {
      rev_updates.push_back(join(identifier_of(declaration_of(exp)), "_b += ", adj));
    } else {
      throwKind(exp);
    }
  }


};

struct StatementAnalysis {
  static consteval std::ranges::range_value_t<std::string&> const* generatePrimal(info stmt, GenerationContext& ctx) {
    std::string r = "";
    if (is_declaration_statement(stmt)) {
      info v = declared_variable_of(stmt);
      r += ctx.format(join(
        display_string_of(type_of(v)),
        " ",
        identifier_of(v),
        " = ",
        ExpressionAnalysis::generatePrimal(initializer_of(v))));
    } else if (is_expression_statement(stmt)) {
      r += ctx.format(ExpressionAnalysis::generatePrimal(expression_of(stmt)));
    } else if (is_return_statement(stmt)) {
      r += ctx.format(join("return ", ExpressionAnalysis::generatePrimal(return_operand_of(stmt))));
    } else {
      throwKind(stmt);
    }

    r += ";\n";
    return std::define_static_string(r);
  }

  static consteval std::ranges::range_value_t<std::string&> const* generateForward(info stmt, GenerationContext& ctx) {
    std::string r = "";
    bool add_primal = true;
    if (is_declaration_statement(stmt)) {
      info v = declared_variable_of(stmt);
      r += ctx.format(join(
        display_string_of(type_of(v)),
        " ",
        identifier_of(v), "_d",
        " = ",
        ExpressionAnalysis::generateForward(initializer_of(v))));
    } else if (is_expression_statement(stmt)) {
      r += ctx.format(ExpressionAnalysis::generateForward(expression_of(stmt)));
    } else if (is_return_statement(stmt)) {
      r += ctx.format(join(
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
    if (is_declaration_statement(stmt)) {
      // Nothing
    } else if (is_expression_statement(stmt)) {
      std::vector<std::string> overwrites = ExpressionAnalysis::listOverwrites(expression_of(stmt));
      for (std::string cur : overwrites) {
        r += ctx.format(join("store(", cur, ");\n"));
      }
    } else if (is_return_statement(stmt)) {
      // Nothing
    } else {
      throwKind(stmt);
    }
    return std::define_static_string(r);
  }

  static consteval std::ranges::range_value_t<std::string&> const* generateRestore(info stmt, GenerationContext& ctx) {
    std::string r = "";
    if (is_declaration_statement(stmt)) {
      // Nothing
    } else if (is_expression_statement(stmt)) {
      std::vector<std::string> overwrites = ExpressionAnalysis::listOverwrites(expression_of(stmt));
      for (std::string cur : overwrites | std::views::reverse) {
        r += ctx.format(join("restore(", cur, ");\n"));
      }
    } else if (is_return_statement(stmt)) {
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
    if (is_declaration_statement(stmt)) {
      info v = declared_variable_of(stmt);
      std::string adj = join(identifier_of(v), "_b");
      ExpressionAnalysis::generateReverse(initializer_of(v), adj, reverse_stmts, lhs_values);
      lhs_values.push_back(adj);
    } else if (is_expression_statement(stmt)) {
      ExpressionAnalysis::generateReverse(expression_of(stmt), "", reverse_stmts, lhs_values);
    } else if (is_return_statement(stmt)) {
      std::string adj = "ret_b";
      ExpressionAnalysis::generateReverse(return_operand_of(stmt), adj, reverse_stmts, lhs_values);
      lhs_values.push_back(adj);
    } else {
      throwKind(stmt);
    }

    for (std::string cur : reverse_stmts) {
      r += ctx.format(join(cur, ";\n"));
    }
    for (std::string cur : lhs_values) {
      r += ctx.format(join(cur, " = 0.0;\n"));
    }

    return std::define_static_string(r);
  }

  static consteval std::ranges::range_value_t<std::string&> const* generateReverseDecl(info stmt, GenerationContext& ctx) {
    std::string r = "";
    if (is_declaration_statement(stmt)) {
      info v = declared_variable_of(stmt);
      r += ctx.format(join(
        display_string_of(type_of(v)),
        " ",
        identifier_of(v),
        "_b = 0.0;\n"));
    } else if (is_expression_statement(stmt)) {
      // Nothing
    } else if (is_return_statement(stmt)) {
      // Nothing
    } else {
      throwKind(stmt);
    }

    return std::define_static_string(r);
  }
};


struct FunctionAnalysis {
  static consteval auto generatePrimal(info func) {
    GenerationContext ctx = {};
    std::string r = "";
    info ret_type = return_type_of(func);
    auto params = parameters_of(func);

    std::string params_s = "";
    for (info p : params) {
      params_s += join(params_s.size() ? ", " : "", display_string_of(type_of(p)), " ", identifier_of(p));
    }

    r += join(display_string_of(ret_type), " ", identifier_of(func), "(", params_s, ") {\n");

    ctx.addIndent();
    for (info stmt : statements_of(body_of(func))) {
      r += StatementAnalysis::generatePrimal(stmt, ctx);
    }
    ctx.removeIndent();
    r += "}\n";

    return std::define_static_string(r);
  }

  static consteval auto generateForward(info func) {
    GenerationContext ctx = {};
    std::string r = "";
    info ret_type = return_type_of(func);
    auto params = parameters_of(func);

    std::string params_s = "";
    for (info p : params) {
      params_s += join(
        params_s.size() ? ", " : "",
        display_string_of(type_of(p)), " ", identifier_of(p), ", ",
        display_string_of(type_of(p)), " ", identifier_of(p), "_d");
    }

    r += join("std::pair<", display_string_of(ret_type), ", ", display_string_of(ret_type), "> ", identifier_of(func), "_d(", params_s, ") {\n");

    ctx.addIndent();
    for (info stmt : statements_of(body_of(func))) {
      r += StatementAnalysis::generateForward(stmt, ctx);
    }
    ctx.removeIndent();
    r += "}\n";

    return std::define_static_string(r);
  }

  static consteval auto generateReverseJoined(info func) {
    GenerationContext ctx = {};
    std::string r = "";
    info ret_type = return_type_of(func);
    auto params = parameters_of(func);

    std::string params_s = "";
    for (info p : params) {
      params_s += join(
        params_s.size() ? ", " : "",
        display_string_of(type_of(p)), " ", identifier_of(p), ", ",
        display_string_of(type_of(p)), "& ", identifier_of(p), "_b");
    }
    if (!is_void_type(ret_type)) {
      params_s += join(
        params_s.size() ? ", " : "",
        display_string_of(ret_type), " ret_b");
    }

    // Reverse functions do not return values.
    r += join("void ", identifier_of(func), "_b(", params_s, ") {\n");

    ctx.addIndent();
    for (info stmt : statements_of(body_of(func))) {
      if (is_return_statement(stmt)) {
        // Handling of return statements is not required.
        continue;
      }

      r += StatementAnalysis::generateStore(stmt, ctx);
      r += StatementAnalysis::generateReverseDecl(stmt, ctx);
      r += StatementAnalysis::generatePrimal(stmt, ctx);
    }

    for (info stmt : statements_of(body_of(func)) | std::views::reverse ) {
      r += "\n";
      r += join("//", StatementAnalysis::generatePrimal(stmt, ctx));
      r += StatementAnalysis::generateRestore(stmt, ctx);
      r += StatementAnalysis::generateReverse(stmt, ctx);
    }
    ctx.removeIndent();
    r += "}\n";

    return std::define_static_string(r);
  }
};

