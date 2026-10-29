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

double func_04(double a) {
  double b = func_call(a);
  return b * a;
}

