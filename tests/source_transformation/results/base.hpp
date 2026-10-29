double func_01(double a) {
  return a * a;
}

std::pair<double, double> func_01_d(double a, double a_d) {
  return std::make_pair(a * a, (a) * (a_d) + (a_d) * (a));
}

void func_01_b(double a, double& a_b, double ret_b) {

//  return a * a;
  a_b += (a) * ret_b;
  a_b += (a) * ret_b;
  ret_b = 0.0;
}

double func_02(double a) {
  double b = a * a;
  return b * b;
}

std::pair<double, double> func_02_d(double a, double a_d) {
  double b_d = (a) * (a_d) + (a_d) * (a);
  double b = a * a;
  return std::make_pair(b * b, (b) * (b_d) + (b_d) * (b));
}

void func_02_b(double a, double& a_b, double ret_b) {
  double b_b = 0.0;
  double b = a * a;

//  return b * b;
  b_b += (b) * ret_b;
  b_b += (b) * ret_b;
  ret_b = 0.0;

//  double b = a * a;
  a_b += (a) * b_b;
  a_b += (a) * b_b;
  b_b = 0.0;
}

double func_03(double a) {
  a = a * a;
  return a * a;
}

std::pair<double, double> func_03_d(double a, double a_d) {
  a_d = (a) * (a_d) + (a_d) * (a);
  a = a * a;
  return std::make_pair(a * a, (a) * (a_d) + (a_d) * (a));
}

void func_03_b(double a, double& a_b, double ret_b) {
  store(a);
  a = a * a;

//  return a * a;
  a_b += (a) * ret_b;
  a_b += (a) * ret_b;
  ret_b = 0.0;

//  a = a * a;
  restore(a);
  a_b += (a) * a_b;
  a_b += (a) * a_b;
  a_b = 0.0;
}

