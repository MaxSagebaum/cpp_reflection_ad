#include "../fixtures/functions/st_functions.hpp"
#include "test_simple_include.hpp"

#include <ad/source_transformation/source_transformation.hpp>

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wc23-extensions"
char const results[] =
{
#embed "../results/source_transformation.hpp"
  ,'\0'
};
#pragma clang diagnostic pop

int main(int nargs, char** args) {

  std::string filename = "derivatives.hpp";

  if (nargs >= 2) {
    filename = args[1];
  }

  std::stringstream data = {};

  static constexpr auto members = std::define_static_array(members_of( ^^test_functions, access_context::unchecked() ));

  template for (constexpr auto member : members) {
    if (is_function(member)) {
      data << FunctionAnalysis::generatePrimal(member) << std::endl;
      data << FunctionAnalysis::generateForward(member) << std::endl;
      data << FunctionAnalysis::generateReverseJoined(member) << std::endl;
    }
  }

  std::ofstream out(filename);
  out << data.str();
  out.close();

  std::string result_str = results;
  size_t min_size = std::min(data.str().size(), result_str.size());
  for (size_t i = 0; i < min_size; i += 1) {
    if (data.str()[i] != result_str[i]) {
      std::cout << std::format("Difference at: {} ('{}' <-> '{}')", i, data.str()[i], result_str[i]) << std::endl;
    }
  }
  EXPECT_EQUAL_ARRAY(data.str(), result_str);
  std::cout << result_str << std::endl;

  TEST_END;
}