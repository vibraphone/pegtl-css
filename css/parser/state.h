#ifndef css_parser_state_h
#define css_parser_state_h
#include "css/composite/grammar.h"

namespace css
{
namespace parser
{

struct propertyData
{
  std::map<std::string, std::string> map;
};

struct accumulator
{
  std::string selector;
  propertyData properties;
  std::string propertyName;
  std::string propertyValue;
};

/// State associated with parsing a stylesheet.
struct stylesheet
{
  stylesheet() = default;
  bool valid = true;
  std::string encoding = "utf-8";
  accumulator accumulate;
  std::unordered_map<std::string, propertyData> properties;
};

} // namespace parser
} // namespace css

#endif // css_parser_state_h
