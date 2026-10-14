#pragma once

namespace test_functions {
  double func_01(double a) {
    return a * a;
  }

  double func_02(double a) {
    double b = 2 * a;
    return b * b;
  }

  double func_03(double a) {
    a = 2 * a;
    return a * a;
  }
}