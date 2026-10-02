#include <string>
#include <cstring>
#include <iostream>

#include "tests/autograd.h"
#include "tests/forward_source_transformation.h"


double func(double x) {
  return 2 * x * x;
}

int main() {

  std::string_view s = ad::forward_source_transformation<^^func,0>();
  std::cout << s << std::endl;

  return 0;
}
