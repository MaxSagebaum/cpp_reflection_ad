void func_01_b(double a, double& a_b, double ret_b) {

//  return a * a;
  a_b += (a) * ret_b;
  a_b += (a) * ret_b;
  ret_b = 0.0;
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

