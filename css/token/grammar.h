#ifndef css_token_grammar_h
#define css_token_grammar_h
#include "css/config.h"

namespace css
{

/// Rules for "simple" (non-composite) CSS tokens.
/// Tokens may consist of multiple code-points but
/// do not have arbitrary, user-provided content
/// that has any affect on the parsed output.
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
} // css namespace

#endif // css_token_grammar_h
