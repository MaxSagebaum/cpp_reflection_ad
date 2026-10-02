#ifndef REFLECT_DEMO_FORWARD_SOURCE_TRANSFORMATION_H
#define REFLECT_DEMO_FORWARD_SOURCE_TRANSFORMATION_H

#include <meta>

#include "autograd.h"

namespace ad {

  constexpr std::size_t BUFFER_SIZE = 4096;

using std::meta::info;

  template <typename T>
    requires std::is_enum_v<T>
  constexpr std::string_view to_enum_string(T val)
  {
    template for (constexpr auto e :
                  std::define_static_array(std::meta::enumerators_of(^^T)))
    {
      if (val == [:e:])
        return std::meta::identifier_of(e);
    }

    return "<unknown>";
  }

consteval std::string id(std::size_t value) {
  std::string result = {};

  if (value == 0) {
    return "0";
  }

  while (value > 0) {
    result += char('0' + value % 10);
    value /= 10;
  }

  std::reverse(result.begin(), result.end());
  return result;
}

consteval std::string val(std::size_t d) {
  std::string v = "val_";
  v += id(d);
  return v;
}

consteval std::string tang(std::size_t d) {
  std::string v = "val_d_";
  v += id(d);
  return v;
}

// Forward-mode directional derivative of `Fn` w.r.t. input index `Wrt`,
// evaluated at `args`. Compiles to inlined arithmetic (zero-cost).
template <std::meta::info Fn, std::size_t Wrt, typename T = double>
constexpr auto forward_source_transformation() {
  static constexpr auto nodes =
      std::define_static_array(build_marked_nodes<Fn, (1ull << Wrt)>());
  constexpr std::size_t N = nodes.size();

  std::string source = {};

  source += "Nodes: " + id(N) + "\n";

  template for (constexpr auto n : nodes) {
    // Primal (only where the value is actually read by a derivative).
    if constexpr (n.nself) {
      // A guarded node belongs to a branch; skip it unless that branch is
      // taken.
      // TODO: Add guards again
      if constexpr (n.op == OpKind::Input)
        source += val(n.self) + " = arg(" + id(n.self) +");\n";
      else if constexpr (n.op == OpKind::Const)
        source += val(n.self) + " = " + display_string_of(n.leaf) + ";\n";
      else if constexpr (n.op == OpKind::Output)
        source += val(n.self) + " = " + val(n.a) + ";\n";
      // else if constexpr (n.op == OpKind::Add)
      //   val[n.self] = val[n.a] + val[n.b];
      // else if constexpr (n.op == OpKind::Sub)
      //   val[n.self] = val[n.a] - val[n.b];
      else if constexpr (n.op == OpKind::Mul)
        source += val(n.self) + " = " + val(n.a) + " * " + val(n.b) + ";\n";
      // else if constexpr (n.op == OpKind::Div)
      //   val[n.self] = val[n.a] / val[n.b];
      // else if constexpr (n.op == OpKind::Neg)
      //   val[n.self] = -val[n.a];
      // else if constexpr (n.op == OpKind::Sin)
      //   val[n.self] = std::sin(val[n.a]);
      // else if constexpr (n.op == OpKind::Cos)
      //   val[n.self] = std::cos(val[n.a]);
      // else if constexpr (n.op == OpKind::Exp)
      //   val[n.self] = std::exp(val[n.a]);
      // else if constexpr (n.op == OpKind::Log)
      //   val[n.self] = std::log(val[n.a]);
      // else if constexpr (n.op == OpKind::Sqrt)
      //   val[n.self] = std::sqrt(val[n.a]);
      // else if constexpr (n.op == OpKind::Erfc)
      //   val[n.self] = std::erfc(val[n.a]);
      // else if constexpr (n.op == OpKind::Lt)
      //   val[n.self] = (val[n.a] < val[n.b]) ? T{1} : T{0};
      // else if constexpr (n.op == OpKind::Le)
      //   val[n.self] = (val[n.a] <= val[n.b]) ? T{1} : T{0};
      // else if constexpr (n.op == OpKind::Gt)
      //   val[n.self] = (val[n.a] > val[n.b]) ? T{1} : T{0};
      // else if constexpr (n.op == OpKind::Ge)
      //   val[n.self] = (val[n.a] >= val[n.b]) ? T{1} : T{0};
      // else if constexpr (n.op == OpKind::Eq)
      //   val[n.self] = (val[n.a] == val[n.b]) ? T{1} : T{0};
      // else if constexpr (n.op == OpKind::Ne)
      //   val[n.self] = (val[n.a] != val[n.b]) ? T{1} : T{0};
      // else if constexpr (n.op == OpKind::Not)
      //   val[n.self] = (val[n.a] != T{0}) ? T{0} : T{1};
      // // Reads val[b] only when val[a] leaves it undecided -- exactly when the
      // // right operand was lowered as reachable, so its slot is written.
      // else if constexpr (n.op == OpKind::And)
      //   val[n.self] = (val[n.a] != T{0} && val[n.b] != T{0}) ? T{1} : T{0};
      // else if constexpr (n.op == OpKind::Or)
      //   val[n.self] = (val[n.a] != T{0} || val[n.b] != T{0}) ? T{1} : T{0};
      // // Select likewise reads only the branch it takes.
      // else if constexpr (n.op == OpKind::Select)
      //   val[n.self] = (val[n.cond] != T{0}) ? val[n.a] : val[n.b];
      // else if constexpr (n.op == OpKind::Abs)
      //   val[n.self] = (val[n.a] < T{0}) ? -val[n.a] : val[n.a];
      // else if constexpr (n.op == OpKind::Max)
      //   val[n.self] = (val[n.a] < val[n.b]) ? val[n.b] : val[n.a];
      // else if constexpr (n.op == OpKind::Min)
      //   val[n.self] = (val[n.b] < val[n.a]) ? val[n.b] : val[n.a];
      else {
        source += std::string("Error: Primal not handled '") + to_enum_string(n.op) + "'\n";
      }
    }

    // Tangent (activity-gated: a non-varied operand contributes a zero term,
    // which is dropped instead of emitted as `... * 0`).
    if constexpr (n.op == OpKind::Input) {
      source += tang(n.self) + " = arg_d(" + id(n.self) +");\n";
    } else if constexpr (!n.vself) {
      source += tang(n.self) + " = 0.0;\n";
    } else {
      // TODO: Add guards again
      if constexpr (n.op == OpKind::Output) {
        source += tang(n.self) + " = " + tang(n.a) + ";\n";
      // else if constexpr (n.op == OpKind::Neg)
      //   tang[n.self] = -tang[n.a];
      // else if constexpr (n.op == OpKind::Sin)
      //   tang[n.self] = std::cos(val[n.a]) * tang[n.a];
      // else if constexpr (n.op == OpKind::Cos)
      //   tang[n.self] = -std::sin(val[n.a]) * tang[n.a];
      // else if constexpr (n.op == OpKind::Exp)
      //   tang[n.self] = val[n.self] * tang[n.a];
      // else if constexpr (n.op == OpKind::Log)
      //   tang[n.self] = tang[n.a] / val[n.a];
      // else if constexpr (n.op == OpKind::Sqrt)
      //   tang[n.self] = tang[n.a] / (T{2} * val[n.self]);
      // else if constexpr (n.op == OpKind::Erfc)
      //   tang[n.self] = tang[n.a] * -1 * two_over_root_pi *
      //                  (std::exp(-1 * (val[n.a] * val[n.a])));
      // else if constexpr (n.op == OpKind::Add) {
      //   if constexpr (n.va && n.vb)
      //     tang[n.self] = tang[n.a] + tang[n.b];
      //   else if constexpr (n.va)
      //     tang[n.self] = tang[n.a];
      //   else
      //     tang[n.self] = tang[n.b];
      // } else if constexpr (n.op == OpKind::Sub) {
      //   if constexpr (n.va && n.vb)
      //     tang[n.self] = tang[n.a] - tang[n.b];
      //   else if constexpr (n.va)
      //     tang[n.self] = tang[n.a];
      //   else
      //     tang[n.self] = -tang[n.b];
      } else if constexpr (n.op == OpKind::Mul) {
        if constexpr (n.va && n.vb)
          source += tang(n.self) + " = " + tang(n.a) + " * " + val(n.b) + " + " + val(n.a) + " * " + tang(n.b) + ";\n";
        else if constexpr (n.va)
          source += tang(n.self) + " = " + tang(n.a) + " * " + val(n.b) + ";\n";
        else
          source += tang(n.self) + " = " + val(n.a) + " * " + tang(n.b) + ";\n";
      // else if constexpr (n.op == OpKind::Div) {
      //   if constexpr (n.va && n.vb)
      //     tang[n.self] = (tang[n.a] * val[n.b] - val[n.a] * tang[n.b]) /
      //                    (val[n.b] * val[n.b]);
      //   else if constexpr (n.va)
      //     tang[n.self] = tang[n.a] / val[n.b];
      //   else
      //     tang[n.self] = -val[n.a] * tang[n.b] / (val[n.b] * val[n.b]);
      // } else if constexpr (n.op == OpKind::Select) {
      //   if constexpr (n.va && n.vb)
      //     tang[n.self] = (val[n.cond] != T{0}) ? tang[n.a] : tang[n.b];
      //   else if constexpr (n.va)
      //     tang[n.self] = (val[n.cond] != T{0}) ? tang[n.a] : T{0};
      //   else
      //     tang[n.self] = (val[n.cond] != T{0}) ? T{0} : tang[n.b];
      // } else if constexpr (n.op == OpKind::Abs) {
      //   tang[n.self] = (val[n.a] < T{0}) ? -tang[n.a] : tang[n.a];
      // } else if constexpr (n.op == OpKind::Max || n.op == OpKind::Min) {
      //   // Both pass one operand through; they differ only in which way round.
      //   const bool takes_b = (n.op == OpKind::Max) ? (val[n.a] < val[n.b])
      //                                              : (val[n.b] < val[n.a]);
      //   if constexpr (n.va && n.vb)
      //     tang[n.self] = takes_b ? tang[n.b] : tang[n.a];
      //   else if constexpr (n.va)
      //     tang[n.self] = takes_b ? T{0} : tang[n.a];
      //   else
      //     tang[n.self] = takes_b ? tang[n.b] : T{0};
      } else {
        source += std::string("Error: Tangent not handled '") + to_enum_string(n.op) + "'\n";
      }
    }
  }

  return std::define_static_string(source);
}
} // namespace ad

#endif // REFLECT_DEMO_FORWARD_SOURCE_TRANSFORMATION_H
