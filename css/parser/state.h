#ifndef css_parser_state_h
#define css_parser_state_h
#include "css/composite/grammar.h"

#include <functional> // for hash

namespace css
{

/// CSS properties originate from one of these places.
enum class origin
{
  user_agent, //!< A style sheet provided by the user's browser/viewer/application.
  user,       //!< A style sheet provided by the user (user overrides accepted by agent).
  author,     //!< A style sheet provided by the content author alongside the content.
  animation,  //!< An animation that is modifying the property.
  transition  //!< An animated transition that is modifying the property.
};

namespace parser
{

/// A property name and value.
struct property
{
  std::string name; //!< The property's name (an identifier)
  mutable std::string value; //!< The property's value (NB: This will become a variant in the future).
  mutable origin source = origin::user_agent; //!< What type of stylesheet or animation is providing the value.
  mutable bool important = false; //!< Whether the property has been prioritized as important.

  /// Initialize the property to its default state.
  void clear()
  {
    this->name.clear();
    this->source = origin::user_agent;
    this->important = false;
  }
  /// Returns true when the name is set; false otherwise.
  bool is_set()
  {
    return !name.empty();
  }
};

/// Print property information.
inline std::ostream& operator << (std::ostream& os, const property& p)
{
  os << p.name << ": " << p.value;
  if (p.important)
  {
    os << " !important";
  }
  return os;
}

/// A set of properties stored by hashing their names.
class property_data
{
public:
  std::size_t size() const { return m_data.size(); }
  void insert(const property& p)
  {
    std::size_t key = std::hash<std::string>{}(p.name);
    m_data[key] = p;
  }
  const property* find(const std::string& name) const
  {
    std::size_t key = std::hash<std::string>{}(name);
    auto it = m_data.find(key);
    if (it == m_data.end())
    {
      return nullptr;
    }
    return &it->second;
  }
  void visit(std::function<void(const property&)> visitor) const
  {
    for (const auto& entry : m_data)
    {
      visitor(entry.second);
    }
  }
  void clear()
  {
    m_data.clear();
  }
protected:
  std::map<std::size_t, property> m_data;
};

/// Accumulate state as we parse tokens.
struct accumulator
{
  std::string selector;
  property_data properties;
  property prop;
};

/// State associated with parsing a stylesheet.
struct stylesheet
{
  stylesheet() = default;
  bool valid = true;
  std::string encoding = "utf-8";
  accumulator accumulate;
  std::unordered_map<std::string, property_data> properties;
};

} // namespace parser
} // namespace css

namespace std
{

/// Provide a specialization for hashing properties.
template <> struct hash<css::parser::property>
{
  size_t operator()(const css::parser::property& p) const
  {
    return std::hash<std::string>{}(p.name);
  }
};

} // namespace std

#endif // css_parser_state_h
