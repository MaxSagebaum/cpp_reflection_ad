std::pair<double, double> func_01_d(double a, double a_d) {
  return std::make_pair(a * a, (a) * (a_d) + (a_d) * (a));
}

std::pair<double, double> func_02_d(double a, double a_d) {
  double b_d = (a) * (a_d) + (a_d) * (a);
  double b = a * a;
  return std::make_pair(b * b, (b) * (b_d) + (b_d) * (b));
}

std::pair<double, double> func_03_d(double a, double a_d) {
  a_d = (a) * (a_d) + (a_d) * (a);
  a = a * a;
  return std::make_pair(a * a, (a) * (a_d) + (a_d) * (a));
}

