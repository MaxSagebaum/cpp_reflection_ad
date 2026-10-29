#pragma once

#include "generation_context.hpp"

#include <fstream>
#include <iostream>
#include <ranges>
#include <sstream>

#include "../autograd.h"
#include "../meta.h"
#include "operator_info.hpp"
#include "string_util.hpp"
#include "throw_helper.hpp"

namespace ad {
using std::meta::info;
namespace m = ad::meta;
namespace su = ad::string_util;

struct ForwardExpression {
  std::string primal;
  std::string fwd;
};

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
    } else if (m::is_function_call(exp)) {
      auto ops = m::operands_of(e);
      std::string_view name = m::identifier_of(m::declaration_of(m::callee_of(exp)));

      std::string ops_s = "";
      for (info o : ops) {
        ops_s += su::join(ops_s.size() ? ", " : "", ExpressionAnalysis::generatePrimal(o));
      }

      r += su::join(name, "(", ops_s , ")");
    } else {
      throwKind(exp);
    }

    return std::define_static_string(r);
  }

  static consteval ForwardExpression generateForward(info e, GenerationContext& ctx) {
    info exp = ad::detail::stripCasts(e); // TODO: Handle casts.

    ForwardExpression r = {};
    if (m::is_operator_expression(exp)) {
      auto operands = operands_of(exp);

      if (operands.size() != 2) {
        throw "Only binary handling for now"; // TODO: Implement
      }

      OperatorInfo op = getOperatorInfo(m::expression_operator_of(exp));
      ForwardExpression op0 = ExpressionAnalysis::generateForward(operands[0], ctx);

      if (op.kind == OperatorKind::Assign && m::is_function_call(operands[1])) {
        ForwardExpression rhs = ExpressionAnalysis::generateForwardFunctionCall(operands[1], ctx);
        r.primal = su::join(op0.primal, " = ", rhs.primal);
        r.fwd = su::join("[", op0.primal, ", ", op0.fwd, "_d] = ", rhs.fwd);
      } else {
        ForwardExpression op1 = ExpressionAnalysis::generateForward(operands[1], ctx);

        if (op.kind == OperatorKind::Assign) {
          r.primal = su::join(op0.primal, " = " , op1.primal);
          r.fwd += su::join(
            op0.fwd,
            " ",
            op.symbol,
            " ",
            op1.fwd);
        } else if (op.kind == OperatorKind::Mul) {
          r.primal = su::join(op0.primal, " * " , op1.primal);
          r.fwd += su::join(
            "(",
            op0.primal,
            ") * (",
            op1.fwd,
            ") + (",
            op0.fwd,
            ") * (",
            op1.primal,
            ")");
        } else {
          throw "Unkown operator";
        }
      }
    } else if (m::is_literal(exp)) {
      r.primal = generatePrimal(exp);
      r.fwd += "0.0";
    } else if (m::is_id_expression(exp)) {
      r.primal = generatePrimal(exp);
      r.fwd = su::join(r.primal, "_d");
    } else if (m::is_function_call(exp)) {
      ForwardExpression call_exp = generateForwardFunctionCall(exp, ctx);
      r.primal = ctx.generateTemporary();
      r.fwd = su::join(r.primal, "_d");
      std::string call = su::join("auto [", r.primal, ", ", r.fwd, "] = ", call_exp.fwd,";");
      ctx.addStatement(call);
    } else {
      throwKind(exp);
    }

    return r;
  }

  static consteval ForwardExpression generateForwardFunctionCall(info exp, GenerationContext& ctx) {
    ForwardExpression r = {};
    auto ops = m::operands_of(exp);
    std::string_view name = m::identifier_of(m::declaration_of(m::callee_of(exp)));

    std::string ops_s_primal = "";
    std::string ops_s_fwd = "";
    for (info o : ops) {
      ForwardExpression o_exp = ExpressionAnalysis::generateForward(o, ctx);
      ops_s_primal += su::join(ops_s_primal.size() ? ", " : "", o_exp.primal);
      ops_s_fwd += su::join(ops_s_fwd.size() ? ", " : "", o_exp.primal, ", ", o_exp.fwd);
    }

    r.primal = su::join( name, "(", ops_s_primal , ")");
    r.fwd = su::join( name, "_d(", ops_s_fwd , ")");

    return r;
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