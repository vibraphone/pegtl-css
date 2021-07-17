#ifndef css_parser_actions_h
#define css_parser_actions_h
#include "css/composite/grammar.h"
#include "css/parser/state.h"

namespace css
{
namespace parser
{

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
    sheet.accumulate.prop.name = in.string();
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
    sheet.accumulate.prop.value = in.string();
  }
};

template<>
struct action<composite::important>
{
  template<typename Input>
  static void apply(
    const Input& in,
    stylesheet& sheet)
  {
    sheet.accumulate.prop.important = true;
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
    if (sheet.accumulate.prop.is_set())
    {
      sheet.accumulate.properties.insert(sheet.accumulate.prop);
      sheet.accumulate.prop.clear();
    }
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
    sheet.accumulate.properties.visit(
      [&sheet](const property& p) { sheet.properties[sheet.accumulate.selector].insert(p); }
    );
    sheet.accumulate.properties.clear();
  }
};
#endif // !CSS_DBG_PARSE

} // namespace parser
} // namespace css

#endif // css_parser_actions_h
