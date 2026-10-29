#pragma once

double func_call(double a) {
  return a * a;
}

namespace test_functions {
double func_01(double a) {
  return a * a;
}

double func_02(double a) {
  double b = a * a;
  return b * b;
}

double func_03(double a) {
  a = a * a;
  return a * a;
}

}