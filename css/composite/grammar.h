#ifndef css_composite_grammar_h
#define css_composite_grammar_h
#include "css/token/grammar.h"

namespace css
{

/// Rules for "composite" CSS grammar elements.
/// These rules are composed of css::token and
/// css::rule elements rather than being standalone
/// rules.
///
/// Much of the grammar is based on the non-normative
/// but informative https://www.w3.org/TR/CSS22/grammar.html .
/// A notable exception is @media formatting, taken from MDN's
/// documentation.
namespace composite
{

/// The important keyword
struct important :
  rule::seq<
    token::bang,
    rule::star<
      token::whitespace
    >,
    rule::istring<'i', 'm', 'p', 'o', 'r', 't', 'a', 'n', 't'>
  >
{
};

struct function_open :
  rule::seq<
    token::ident,
    token::paren_open
  >
{
};

struct function_close : token::paren_close
{
};

// forward declaration
struct function;

struct term :
  rule::sor<
    composite::function,
    rule::seq<
      rule::sor<
        token::percentage,
        token::length,
        token::ems,
        token::exs,
        token::angle,
        token::time,
        token::frequency,
        token::string,
        token::ident,
        token::dimension,
        token::number,
        token::url
      >,
      token::optional_whitespace
    >,
    token::hexcolor,
    token::ident
  >
{
};

/// An operator is either division ('/') or accumulation (',').
struct operator_rule :
  rule::seq<
    rule::sor<
      token::slash,
      token::comma
    >,
    token::optional_whitespace
  >
{
};

/// An expression is a collection of terms which may be separated by operators.
struct expr :
  rule::seq<
    composite::term,
    rule::star<
      rule::seq<
        rule::opt<composite::operator_rule>,
        composite::term
      >
    >
  >
{
};

/// A function is an identifier followed by "arguments" (an expr).
struct function :
  rule::seq<
    composite::function_open,
    token::optional_whitespace,
    composite::expr,
    composite::function_close,
    token::optional_whitespace
  >
{
};

/// Priority
struct prio :
  rule::seq<
    composite::important,
    token::optional_whitespace
  >
{
};

/// A property keyword
struct property : token::ident
{
};

/// A property's value as a function or expression.
struct property_value :
  rule::sor<
    composite::function,
    composite::expr
  >
{
};

/// A property declaration (property name, value, and optional priority/importance).
struct declaration :
  rule::seq<
    composite::property,
    token::optional_whitespace,
    token::colon,
    token::optional_whitespace,
    composite::property_value,
    rule::opt<composite::prio>
  >
{
};

struct pseudo :
  rule::seq<
    token::colon,
    rule::opt<token::colon>, // pseudo classes use 2 colons now.
    rule::sor<
      composite::function,
      token::ident
    >
  >
{
};

struct attrib :
  rule::seq<
    token::bracket_open,
    token::optional_whitespace,
    token::ident,
    token::optional_whitespace,
    rule::opt<
      rule::seq<
        rule::sor<
          token::equal,
          token::includes,
          token::dashmatch,
          token::prefixmatch,
          token::suffixmatch,
          token::starmatch
        >,
        token::optional_whitespace,
        rule::sor<
          token::ident,
          rule::seq<
            token::string,
            rule::opt<
              rule::seq<
                token::optional_whitespace,
                rule::one<'i', 's'>
              >
            >
          >
        >,
        token::optional_whitespace
      >
    >,
    token::bracket_close
  >
{
};

struct element_name :
  rule::sor<
    token::ident,
    token::star
  >
{
};

struct class_modifier :
  rule::seq<
    token::dot,
    token::ident
  >
{
};

struct combinator :
  rule::seq<
    rule::sor<
      token::plus,
      token::angle_close
    >,
    token::optional_whitespace
  >
{
};

struct selector_modifier :
  rule::sor<
    token::hash,
    composite::class_modifier,
    composite::attrib,
    composite::pseudo
  >
{
};

struct simple_selector :
  rule::sor<
    rule::seq<
      composite::element_name,
      rule::star<composite::selector_modifier>
    >,
    rule::plus<composite::selector_modifier>
  >
{
};

struct selector :
  rule::seq<
    composite::simple_selector,
    rule::opt<
      rule::sor<
        rule::seq<
          composite::combinator,
          composite::selector
        >,
        rule::seq<
          token::whitespace,
          rule::opt<
            rule::seq<
              rule::opt<composite::combinator>,
              composite::selector
            >
          >
        >
      >
    >
  >
{
};

struct ruleset :
  rule::seq<
    composite::selector,
    rule::star<
      rule::seq<
        token::comma,
        token::optional_whitespace,
        composite::selector
      >
    >,
    token::curly_open,
    token::optional_whitespace,
    rule::opt<composite::declaration>,
    rule::star<
      rule::seq<
        token::semicolon,
        token::optional_whitespace,
        rule::opt<composite::declaration>
      >
    >,
    token::curly_close,
    token::optional_whitespace
  >
{
};

struct pseudo_page :
  rule::seq<
    token::colon,
    token::ident,
    token::optional_whitespace
  >
{
};


struct page :
  rule::seq<
    token::page_keyword,
    token::optional_whitespace,
    rule::opt<composite::pseudo_page>,
    token::curly_open,
    token::optional_whitespace,
    rule::opt<composite::declaration>,
    rule::star<
      rule::seq<
        token::semicolon,
        token::optional_whitespace,
        rule::opt<composite::declaration>
      >
    >,
    token::curly_close,
    token::optional_whitespace
  >
{
};

struct media_type : token::ident {};
struct mf_name : token:: ident {};

struct mf_value :
  rule::sor<
    token::dimension,
    token::ratio,
    token::number,
    token::ident
  >
{
};

struct mf_plain :
  rule::seq<
    composite::mf_name,
    token::optional_whitespace,
    token::colon,
    token::optional_whitespace,
    composite::mf_value,
    token::optional_whitespace
  >
{
};

struct mf_boolean : mf_name {};

/// Match 1 or 2 comparisons between a media property and range values.
///
/// The following forms are allowed:
/// + name [<=|>=|<|>] value
/// + value [<=|>=|<|>] value
/// + value [<=|<] name [<=|<] value
/// + value [>=|>] name [>=|>] value
struct mf_range :
  rule::sor<
    rule::seq<
      composite::mf_name,
      token::optional_whitespace,
      token::comparator,
      token::optional_whitespace,
      composite::mf_value
    >,
    rule::seq<
      composite::mf_value,
      token::optional_whitespace,
      token::comparator,
      token::optional_whitespace,
      composite::mf_name
    >,
    rule::seq<
      composite::mf_value,
      token::optional_whitespace,
      token::lte_comparator,
      token::optional_whitespace,
      composite::mf_name,
      token::optional_whitespace,
      token::lte_comparator,
      token::optional_whitespace,
      composite::mf_value
    >,
    rule::seq<
      composite::mf_value,
      token::optional_whitespace,
      token::gte_comparator,
      token::optional_whitespace,
      composite::mf_name,
      token::optional_whitespace,
      token::gte_comparator,
      token::optional_whitespace,
      composite::mf_value
    >
  >
{
};

struct media_feature :
  rule::seq<
    token::paren_open,
    token::optional_whitespace,
    rule::sor<
      composite::mf_plain,
      composite::mf_boolean,
      composite::mf_range
    >,
    token::optional_whitespace,
    token::paren_close,
    token::optional_whitespace
  >
{
};

struct general_enclosed :
  rule::sor<
    rule::seq<
      composite::function_open,
      rule::until<composite::function_close>
    >,
    rule::seq<
      token::paren_open,
      token::optional_whitespace,
      token::ident,
      rule::until<token::paren_close>
    >
  >
{
};

struct media_condition; // forward declaration

struct media_in_parens :
  rule::sor<
    rule::seq<
      token::paren_open,
      token::optional_whitespace,
      composite::media_condition,
      token::optional_whitespace,
      token::paren_close
    >,
    composite::media_feature,
    composite::general_enclosed
  >
{
};

struct media_not :
  rule::seq<
    token::not_keyword,
    token::whitespace,
    composite::media_in_parens
  >
{
};

struct media_and :
  rule::seq<
    composite::media_in_parens,
    rule::plus<
      token::whitespace,
      token::and_keyword,
      token::whitespace,
      composite::media_in_parens
    >
  >
{
};

struct media_or :
  rule::seq<
    composite::media_in_parens,
    rule::plus<
      token::whitespace,
      token::or_keyword,
      token::whitespace,
      composite::media_in_parens
    >
  >
{
};

struct media_condition :
  rule::sor<
    composite::media_not,
    composite::media_and,
    composite::media_in_parens
  >
{
};

struct media_condition_without_or :
  rule::sor<
    composite::media_not,
    composite::media_and,
    composite::media_in_parens
  >
{
};

struct medium :
  rule::seq<
    rule::sor<
      composite::media_condition,
      rule::seq<
        rule::opt<
          rule::sor<token::not_keyword, token::only_keyword>
        >,
        composite::media_type,
        rule::opt<
          rule::seq<
            token::whitespace,
            token::and_keyword,
            token::whitespace,
            composite::media_condition_without_or
          >
        >
      >
    >,
    token::optional_whitespace
  >
{
};

struct media_list :
  rule::seq<
    composite::medium,
    rule::star<
      rule::seq<
        token::comma,
        token::optional_whitespace,
        composite::medium
      >
    >
  >
{
};

struct media :
  rule::seq<
    token::media_keyword,
    token::whitespace,
    composite::media_list,
    token::curly_open,
    token::optional_whitespace,
    rule::star<composite::ruleset>,
    token::curly_close,
    token::optional_whitespace
  >
{
};

struct import_rule :
  rule::seq<
    token::import_keyword,
    token::optional_whitespace,
    rule::sor<token::string, token::url>,
    token::optional_whitespace,
    rule::opt<composite::media_list>,
    token::semicolon,
    token::optional_whitespace
  >
{
};

struct import_rules :
  rule::star<
    rule::seq<
      composite::import_rule,
      rule::opt<
        rule::sor<
          rule::seq<
            token::CDO,
            token::optional_whitespace
          >,
          rule::seq<
            token::CDC,
            token::optional_whitespace
          >
        >
      >
    >
  >
{
};

/// A stylesheet has an optional encoding, import statements, and
/// then ruleset, media, and page statements.
struct stylesheet :
  rule::seq<
    rule::opt<token::encoding>,
    rule::star<
      rule::sor<
        token::whitespace,
        token::CDO,
        token::CDC
      >
    >,
    composite::import_rules,
    rule::star<
      rule::sor<
        composite::ruleset,
        composite::media,
        composite::page
      >,
      rule::star<
        rule::sor<
          rule::seq<
            token::CDO,
            token::optional_whitespace
          >,
          rule::seq<
            token::CDC,
            token::optional_whitespace
          >
        >
      >
    >,
    rule::eof
  >
{
};

/// An at-rule's initial characters (e.g., `@include`)
struct at : rule::seq<rule::string<'@'>, token::ident>
{
};

struct component_value_list; // Forward declaration

struct curly_block :
  rule::seq<
    token::curly_open,
    rule::opt<composite::component_value_list>,
    token::curly_close
  >
{
};

struct paren_block :
  rule::seq<
    token::paren_open,
    rule::opt<composite::component_value_list>,
    token::paren_close
  >
{
};

struct bracket_block :
  rule::seq<
    token::bracket_open,
    rule::opt<composite::component_value_list>,
    token::bracket_close
  >
{
};

struct simple_block : rule::sor<
  composite::curly_block,
  composite::paren_block,
  composite::bracket_block
>
{
};

struct function_block : rule::seq<
  token::ident,
  rule::if_must<token::paren_open, composite::simple_block>
>
{
};


} // composite namespace
} // css namespace

#endif // css_composite_grammar_h
