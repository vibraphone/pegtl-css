#define TAO_PEGTL_NAMESPACE css_pegtl
#define DBG_GRAMMAR 1
#define DBG_PARSE 1

#include "tao/pegtl.hpp"
#ifdef DBG_GRAMMAR
#  include "tao/pegtl/analyze.hpp"
#endif
#include "tao/pegtl/contrib/icu/utf8.hpp"

#include "TypeName.h"
using smtk::common::typeName;

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

namespace rule = tao::css_pegtl;

/// Rules for "simple" (non-composite) CSS tokens.
/// Tokens may consist of multiple code-points but are
/// not sequences of other tokens.
/// The only exceptions are token::ident, token::escape, and token::hash
/// which are expressed using other tokens for consistency
/// rather than necessity.
namespace token
{

/// Any code-point sequence that serves as a line terminator.
struct newline :
  rule::sor<
    rule::string<'\n'>,
    rule::string<'\r', '\n'>,
    rule::string<'\r'>,
    rule::string<'\f'>
  >
{
};

struct comment :
  rule::seq<
    rule::string<'/', '*'>,
    rule::until<rule::utf8::string<'*', '/'>>
  >
{
};

struct bad_comment :
  rule::seq<
    rule::string<'/', '*'>,
    rule::minus<rule::star<rule::utf8::any>, rule::utf8::string<'*', '/'>>,
    rule::eof
  >
{
};

/// One or more whitespace code-points.
///
/// We also match comments as whitespace.
struct whitespace :
  rule::plus<
    rule::sor<
      rule::utf8::icu::white_space,
      token::comment,
      token::bad_comment,
      token::newline
    >
  >
{
};

/// Zero or more whitespace code-points (phrased as an optional token).
struct optional_whitespace :
  rule::star<
    token::whitespace
  >
{
};

/// Characters in a string that do not need to be escaped.
///
/// The template parameter is the string terminator.
template<int Delimiter>
struct unescaped_string_data :
  rule::utf8::not_one<Delimiter, '\\', '\n', '\r', '\f'>
{
};

struct line_continuation :
  rule::seq<
    rule::string<'\\'>,
    token::newline
  >
{
};

/// Any character may serve as a delimiter in some contexts.
struct delim :
  rule::utf8::any
{
};

/// A comment-document opening (CDO) sequence of code-points.
struct CDO : rule::string<'<', '!', '-', '-'>
{
};

/// A comment-document closing (CDC) sequence of code-points.
struct CDC : rule::string<'-', '-', '>'>
{
};

/// Match any comparator operator (equality or inequality); used for media range queries.
struct comparator :
  rule::sor<
    rule::istring<'<', '='>,
    rule::istring<'>', '='>,
    rule::istring<'='>,
    rule::istring<'<'>,
    rule::istring<'>'>
  >
{
};

/// Match any less-than comparator operator (equality or inequality); used for media range queries.
struct lte_comparator :
  rule::sor<
    rule::istring<'<', '='>,
    rule::istring<'<'>
  >
{
};

/// Match any greater-than comparator operator (equality or inequality); used for media range queries.
struct gte_comparator :
  rule::sor<
    rule::istring<'>', '='>,
    rule::istring<'>'>
  >
{
};

/// An attribute selector-modifer uses this to indicate a partial (word) match
/// when the attribute is a whitespace-separated list of words.
struct includes :
  rule::string<'~', '='>
{
};

/// An attribute selector-modifer uses this to indicate a partial match where
/// the attribute value begins with the specified string and may be followed
/// by a dash (used for sub-language specifiers).
struct dashmatch :
  rule::string<'|', '='>
{
};

/// An attribute selector-modifer uses this to indicate its value must
/// begin with the specified value.
struct prefixmatch :
  rule::string<'^', '='>
{
};

/// An attribute selector-modifer uses this to indicate its value must
/// end with the specified value.
struct suffixmatch :
  rule::string<'$', '='>
{
};

/// An attribute selector-modifer uses this to indicate its value must
/// contain at least one occurence of the specified value.
struct starmatch :
  rule::string<'*', '='>
{
};

/// A colon is used as a separator in several contexts, including
/// in qualified-rule preludes (where it separates an html tag
/// from a downscoping pseudo-class) and in pseudo-elements (where
/// 2 colons serve as a prefix). However, the base CSS specification
/// simply allows identifiers to contain colons.
struct colon : rule::string<':'>
{
};

/// A single dot code-point. These are used to separate classes from tag names.
struct dot : rule::string<'.'>
{
};

/// A single star (asterisk) code-point. These are used as wildcards in place of element names.
struct star : rule::string<'*'>
{
};

/// A minus sign used for arithmetic numbers
struct minus : rule::string<'-'>
{
};

/// A plus sign used for arithmetic numbers
struct plus : rule::string<'+'>
{
};

/// A forward slash.
struct slash : rule::string<'/'>
{
};

/// An exclamation mark (bang).
struct bang : rule::string<'!'>
{
};

/// An equal sign mark (a.k.a. equals).
struct equal : rule::string<'='>
{
};

/// A single semicolon code-point. These are used to separate declarations.
struct semicolon : rule::string<';'>
{
};

/// A single comma. These are used to separate components in a qualified-rule
/// prelude and values in multi-value property values.
struct comma : rule::string<','>
{
};

struct angle_open : rule::string<'<'>
{
};

struct angle_close : rule::string<'>'>
{
};

struct bracket_open : rule::string<'['>
{
};

struct bracket_close : rule::string<']'>
{
};

struct paren_open : rule::string<'('>
{
};

struct paren_close : rule::string<')'>
{
};

struct curly_open : rule::string<'{'>
{
};

struct curly_close : rule::string<'}'>
{
};

struct non_ascii : rule::utf8::ranges<0xa0, 0x10ffff>
{
};

struct hex_number : rule::rep_min_max<1, 6, rule::utf8::ranges<'0', '9', 'a', 'f', 'A', 'F'>>
{
};

#if 0
/// A floating-point, decimal number
struct number :
  rule::sor<
    rule::seq<
      rule::opt<rule::one<token::minus, token::plus>>,
      rule::plus<rule::utf8::range<'0', '9'>>
    >,
    rule::seq<
      rule::opt<rule::one<token::minus, token::plus>>,
      rule::star<rule::utf8::range<'0', '9'>>,
      token::dot,
      rule::plus<rule::utf8::range<'0', '9'>>
    >
  >
{
};
#else
/// A floating-point, decimal number
struct number :
  rule::seq<
    // Sign
    rule::opt<rule::utf8::one<'+', '-'>>,
    // Mantissa
    rule::sor<
      rule::seq<
        rule::plus<rule::ascii::digit>, rule::string<'.'>, rule::plus<rule::ascii::digit>
      >,
      rule::seq<
        rule::plus<rule::ascii::digit>
      >,
      rule::seq<
        rule::string<'.'>, rule::plus<rule::ascii::digit>
      >
    >,
    // Exponent
    rule::opt<
      rule::seq<
        rule::one<'e', 'E'>,
        rule::opt<rule::utf8::one<'+', '-'>>,
        rule::plus<rule::ascii::digit>
      >
    >
  >
{
};
#endif

/// An escaped character.
///
/// Escapes begin with a backslash and are followed by a character-specifier.
/// Character-specifiers may be hex numbers that specify a unicode code-point
/// or another non-newline character that might otherwise be matched by the
/// tokenizer/parser. The latter is used, for example, as a way to include
/// string-terminators inside strings.
struct escape :
  rule::seq<
    rule::string<'\\'>,
    rule::sor<
      token::hex_number,
      rule::utf8::not_one<'\n', '\r', '\f'>
    >
  >
{
};

struct letters_digits :
  rule::utf8::ranges<'a', 'z', 'A', 'Z', '0', '9'>
{
};

/// The ending characters of an identifier.
struct ident_suffix :
  rule::star<
    rule::sor<
      token::escape,
      token::letters_digits,
      rule::string<'-'>,
      rule::string<'_'>,
      token::non_ascii
    >
  >
{
};

/// A CSS identifier.
///
/// Identifiers may be – depending on context – element/tag names,
/// pseudo-classes, property names, property values, etc.
struct ident :
  rule::seq<
    rule::sor<
      rule::string<'-','-'>,
      rule::seq<
        rule::opt<rule::string<'-'>>,
        rule::sor<
          token::escape,
          token::non_ascii,
          rule::utf8::ranges<'a', 'z', 'A', 'Z', '_'>
        >
      >
    >,
    token::ident_suffix
  >
{
};

/// An ID selector (or an unrestricted selector when using the Selectors syntax)
struct hash :
  rule::seq<
    rule::string<'#'>,
    token::ident_suffix
  >
{
};

/**
 * There is a constraint on the color that it must
 * have either 3 or 6 hex-digits (i.e., [0-9a-fA-F])
 * after the "#"; e.g., "#000" is OK, but "#abcd" is not.
 */
struct hexcolor :
  rule::seq<
    token::hash,
    token::optional_whitespace
  >
{
};

struct length_units :
  rule::sor<
    rule::istring<'p', 'x'>,
    rule::istring<'c', 'm'>,
    rule::istring<'m', 'm'>,
    rule::istring<'i', 'n'>,
    rule::istring<'p', 't'>,
    rule::istring<'p', 'c'>
  >
{
};

struct length :
  rule::seq<
    token::number,
    token::length_units
  >
{
};

struct ems :
  rule::seq<
    token::number,
    rule::istring<'e', 'm'>
  >
{
};

struct exs :
  rule::seq<
    token::number,
    rule::istring<'e', 'x'>
  >
{
};

struct angle_units :
  rule::sor<
    rule::istring<'d', 'e', 'g'>,
    rule::istring<'r', 'a', 'd'>,
    rule::istring<'g', 'r', 'a', 'd'>
  >
{
};

struct angle :
  rule::seq<
    token::number,
    token::angle_units
  >
{
};

struct time_units :
  rule::sor<
    rule::istring<'m', 's'>,
    rule::istring<'s'>
  >
{
};

struct time :
  rule::seq<
    token::number,
    token::time_units
  >
{
};

struct frequency_units :
  rule::sor<
    rule::istring<'h', 'z'>,
    rule::istring<'k', 'h', 'z'>
  >
{
};

struct frequency :
  rule::seq<
    token::number,
    token::frequency_units
  >
{
};

struct percentage :
  rule::seq<
    token::number,
    rule::string<'%'>
  >
{
};

/// A number with "unknown" dimension (i.e., unknown units).
struct dimension :
  rule::seq<
    token::number,
    token::ident
  >
{
};

/// A ratio of two numbers (e.g., an aspect ratio used for media queries).
struct ratio :
  rule::seq<
    token::number,
    token::optional_whitespace,
    token::colon,
    token::optional_whitespace,
    token::number
  >
{
};

struct double_quoted_string :
  rule::seq<
    rule::string<'"'>,
    rule::star<
      rule::sor<
        token::unescaped_string_data<'"'>,
        token::escape,
        token::line_continuation
      >
    >,
    rule::string<'"'>
  >
{
};

struct single_quoted_string :
  rule::seq<
    rule::string<'\''>,
    rule::star<
      rule::sor<
        token::unescaped_string_data<'\''>,
        token::escape,
        token::line_continuation
      >
    >,
    rule::string<'\''>
  >
{
};

struct string :
  rule::sor<
    token::double_quoted_string,
    token::single_quoted_string
  >
{
};

struct url :
  rule::seq<
    rule::istring<'u', 'r', 'l'>,
    token::paren_open,
    token::optional_whitespace,
    rule::opt<
      rule::star<
        rule::sor<
          token::escape,
          rule::utf8::not_one<'"', '\'', '\\', ' '> // FIXME: This should also omit whitespace and non-printable characters.
        >
      >
    >,
    token::optional_whitespace,
    token::paren_close
  >
{
};

struct import_keyword :
  rule::istring<'@', 'i', 'm', 'p', 'o', 'r', 't'>
{
};

struct page_keyword :
  rule::istring<'@', 'p', 'a', 'g', 'e'>
{
};

struct media_keyword :
  rule::istring<'@', 'm', 'e', 'd', 'i', 'a'>
{
};

struct encoding_charset :
  token::double_quoted_string
{
};

struct encoding :
  rule::seq<
    rule::istring<'@', 'c', 'h', 'a', 'r', 's', 'e', 't', ' '>,
    token::encoding_charset,
    token::semicolon
  >
{
};

/// Place this token at the end of mandatory rule::sor<...> parameter-lists
/// to indicate parsing should stop because progress cannot be made.
struct unexpected_input :
  rule::raise<unexpected_input>
{
};

/// Media queries use "not" to invert media lists.
struct not_keyword :
  rule::istring<'n', 'o', 't'>
{
};

/// Media queries use "and" to combine media queries.
struct and_keyword :
  rule::istring<'a', 'n', 'd'>
{
};

/// Media queries use "or" to choose between media queries.
struct or_keyword :
  rule::istring<'o', 'r'>
{
};

/// Media queries use "only" to limit a rule to a single media type.
struct only_keyword :
  rule::istring<'o', 'n', 'l', 'y'>
{
};

} // token namespace

namespace composite
{

// From https://www.w3.org/TR/CSS22/grammar.html :

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
/// Note that when the `DBG_PARSE` macro is defined, this action will
/// print all matches, which is useful when debugging failures to match.
template<typename Rule>
struct action
#if !DBG_PARSE
  : rule::nothing<Rule>
#endif
{
#if DBG_PARSE
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

#ifndef DBG_PARSE
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
#endif // DBG_PARSE

} // css namepace

int main(int argc, char* argv[])
{
  using namespace tao::css_pegtl;

#ifdef DBG_GRAMMAR
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
#ifndef DBG_PARSE
    std::cout << "\n\n# Summary\n\n";
#endif // DBG_PARSE
    for (const auto& sel : sheet.properties)
    {
      numRulesets += sel.second.map.size();
#ifndef DBG_PARSE
      std::cout << "Selector <" << sel.first << ">\n";
      for (const auto& prop : sel.second.map)
      {
        std::cout << "    " << prop.first << ": " << prop.second << ";\n";
      }
#endif // DBG_PARSE
    }
  }
  std::cout
    << "Parse took " << dt << "µs";
#ifndef DBG_PARSE
  if (sheet.valid)
  {
    std::cout
      << " for " << sheet.properties.size() << " selectors"
      << " and " << numRulesets << " rulesets.";
  }
#endif // DBG_PARSE
  std::cout << "\n";

  return sheet.valid ? 0 : 1;
}
