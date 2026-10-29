#include "../fixtures/functions/st_functions.hpp"
#include "source_transformation/string_compare.hpp"
#include "test_simple_include.hpp"

#include <ad/source_transformation/source_transformation.hpp>

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wc23-extensions"
char const results[] =
{
#embed "../results/source_transformation_primal.hpp"
  ,'\0'
};
#pragma clang diagnostic pop

int main(int nargs, char** args) {

  std::string filename = "derivatives.hpp";

  if (nargs >= 2) {
    filename = args[1];
  }

  std::stringstream data = {};

  ad::handleNamespacePrimal<^^test_functions>(data);

  std::ofstream out(filename);
  out << data.str();
  out.close();

  std::string result_str = results;
  stringCompare(data.str(), result_str);

  std::cout << result_str << std::endl;

  TEST_END;
}