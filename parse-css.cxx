#include "css/composite/grammar.h"

#include "TypeName.h"
using smtk::common::typeName;

#include "css/token/grammar.h"
#include "css/composite/grammar.h"

#include <chrono>
#include <memory>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <typeinfo>

std::string read_file(const std::string& filename)
{
  std::ifstream ifs(filename.c_str(), std::ios::in | std::ios::binary | std::ios::ate);

  std::ifstream::pos_type filesize = ifs.tellg();
  ifs.seekg(0, std::ios::beg);

  std::vector<char> bytes(filesize);
  ifs.read(bytes.data(), filesize);

  return std::string(bytes.data(), filesize);
}

/// A namespace for cascading style sheets (CSS)
namespace css
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

/// A grammar
struct grammar : composite::stylesheet
{
};

/// The base action that sets what state must be passed to the parser.
///
/// Note that when the `CSS_DBG_PARSE` macro is defined, this action will
/// print all matches, which is useful when debugging failures to match.
template<typename Rule>
struct action
#if !CSS_DBG_PARSE
  : rule::nothing<Rule>
#endif
{
#if CSS_DBG_PARSE
  template< typename Input >
  static void apply(
    const Input& in,
    stylesheet& sheet)
  {
    (void)sheet;
    std::cout << "Token " << typeName<Rule>() << " match \"" << in.string() << "\"\n";
  }
#endif
};

#if !CSS_DBG_PARSE
template<>
struct action<token::encoding_charset>
{
  template< typename Input >
  static void apply(
    const Input& in,
    stylesheet& sheet)
  {
    sheet.encoding = in.string().substr(1, in.string().size() - 2);
  }
};

template<>
struct action<composite::selector>
{
  template<typename Input>
  static void apply(
    const Input& in,
    stylesheet& sheet)
  {
    sheet.accumulate.selector = in.string();
  }
};

template<>
struct action<composite::property>
{
  template<typename Input>
  static void apply(
    const Input& in,
    stylesheet& sheet)
  {
    sheet.accumulate.propertyName = in.string();
  }
};

template<>
struct action<composite::property_value>
{
  template<typename Input>
  static void apply(
    const Input& in,
    stylesheet& sheet)
  {
    sheet.accumulate.propertyValue = in.string();
  }
};

template<>
struct action<composite::declaration>
{
  template<typename Input>
  static void apply(
    const Input& in,
    stylesheet& sheet)
  {
    (void)in;
    sheet.accumulate.properties.map[sheet.accumulate.propertyName] = sheet.accumulate.propertyValue;
    sheet.accumulate.propertyName.clear();
    sheet.accumulate.propertyValue.clear();
  }
};

template<>
struct action<composite::ruleset>
{
  template<typename Input>
  static void apply(
    const Input& in,
    stylesheet& sheet)
  {
    sheet.properties[sheet.accumulate.selector].map.insert(
      sheet.accumulate.properties.map.begin(), sheet.accumulate.properties.map.end());
    sheet.accumulate.properties.map.clear();
  }
};
#endif // !CSS_DBG_PARSE

} // css namepace

int main(int argc, char* argv[])
{
  using namespace tao::css_pegtl;

#ifdef CSS_DBG_GRAMMAR
  if (analyze<css::grammar>() != 0 )
  {
    std::cerr << "CSS grammar: cycles without progress detected!\n";
    return 1;
  }
  std::cout << "CSS grammar: no cycles without progress.\n";
#endif

  std::string filename = argc > 1 ? argv[1] : "example.css";
  std::string filedata = read_file(filename);
  auto source = tao::css_pegtl::memory_input(filedata, filename);

  const auto start = std::chrono::steady_clock::now();
  css::stylesheet sheet;
  try
  {
    bool parsed = tao::css_pegtl::parse<css::grammar, css::action>(source, sheet);
    std::cout
      << "\n\n"
      << "Encoding \"" << sheet.encoding << "\"\n"
      << "Parse result: " << (parsed ? "T" : "F")
      << "\n";
    sheet.valid &= parsed;
  }
  catch (parse_error& e)
  {
    std::cout << "***\n\n\n***\n\n\n";
    (void)e;
    const auto p = e.positions.front();
    std::cerr
      << e.what() << "\n"
      << source.line_as_string(p) << "\n"
      << std::setw( p.byte ) << '^' << "\n";
    sheet.valid = false;
  }
  const auto end = std::chrono::steady_clock::now();
  auto dt = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
  std::size_t numRulesets = 0;

  // Print summary and increment counters.
  if (sheet.valid)
  {
#if !CSS_DBG_PARSE
    std::cout << "\n\n# Summary\n\n";
#endif // !CSS_DBG_PARSE
    for (const auto& sel : sheet.properties)
    {
      numRulesets += sel.second.map.size();
#if !CSS_DBG_PARSE
      std::cout << "Selector <" << sel.first << ">\n";
      for (const auto& prop : sel.second.map)
      {
        std::cout << "    " << prop.first << ": " << prop.second << ";\n";
      }
#endif // !CSS_DBG_PARSE
    }
  }
  std::cout
    << "Parse took " << dt << "µs";
#if !CSS_DBG_PARSE
  if (sheet.valid)
  {
    std::cout
      << " for " << sheet.properties.size() << " selectors"
      << " and " << numRulesets << " rulesets.";
  }
#endif // !CSS_DBG_PARSE
  std::cout << "\n";

  return sheet.valid ? 0 : 1;
}
