#include <iostream>
#include <meta>

#include "throwHelper.hpp"
#include "test_functions.hpp"

#include "../../clang-p2996/clang/include/clang/Tooling/Syntax/Nodes.h"
#include "../autograd.h"

#include <fstream>

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

struct OperatorInfo {
  std::string symbol;
};

consteval OperatorInfo getOperatorInfo(operators op) {
  if (op == operators::op_equals) {
    return {"="};
  } else if (op == operators::op_star) {
    return {"*"};
  } else {
    throw "Unknown operator";
  }
}

struct ExpressionAnalysis {
  static consteval std::ranges::range_value_t<std::string&> const* generatePrimal(info e) {
    info exp = ad::detail::stripCasts(e); // TODO: Handle casts.

    std::string r = "";
    if (is_binary_operator(exp)) {
      auto operands = operands_of(exp);
      auto op = expression_operator_of(exp);
      r += join(
        ExpressionAnalysis::generatePrimal(operands[0]),
        " ",
        getOperatorInfo(op).symbol,
        " ",
        ExpressionAnalysis::generatePrimal(operands[1]));
    } else if (is_literal(exp)) {
      r += display_string_of(constant_of(exp));
    } else if (is_variable_reference(exp)) {
      r += identifier_of(declaration_of(exp));
    } else {
      throwKind(exp);
    }

    return std::define_static_string(r);
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
      r += ";\n";
    } else if (is_expression_statement(stmt)) {
      r += ctx.format(ExpressionAnalysis::generatePrimal(expression_of(stmt)));
    } else if (is_return_statement(stmt)) {
      r += ctx.format(join("return ", ExpressionAnalysis::generatePrimal(return_value_of(stmt))));
    } else {
      throwKind(stmt);
    }

    r += ";\n";
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
      params_s += join(display_string_of(type_of(p)), " ", identifier_of(p), params_s.size() ? ", " : "");
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
};

int main(int nargs, char** args) {

  std::string filename = "derivatives.hpp";

  if (nargs >= 2) {
    filename = args[1];
  }

  std::ofstream out(filename);

  static constexpr auto members = std::define_static_array(members_of( ^^test_functions, access_context::unchecked() ));

  template for (constexpr auto member : members) {
    if (is_function(member)) {
      out << FunctionAnalysis::generatePrimal(member) << std::endl;
    }
  }

  out.close();

  return 0;
}