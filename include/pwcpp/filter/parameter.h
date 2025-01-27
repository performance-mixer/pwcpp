#pragma once

#include <optional>
#include <string>
#include <variant>

namespace pwcpp::filter {

/*! \brief The variant stores the parameter value.
 */
using variant_type =
    std::variant<int, float, double, std::string, std::nullopt_t>;

/*! \brief A pipewire parameter.
 */
class Parameter {
public:
  /*! \brief Construct a parameter.
   *
   * \param key The parameter key.
   * \param id The parameter id.
   * \param value The parameter value.
   */
  Parameter(std::string key, size_t id, variant_type value)
      : key(key), id(id), value(value) {}

  /*! \brief Get the parameter value.
   *
   * \return The parameter value.
   */
  template <typename T> const T &get() const { return std::get<T>(value); }

  /*! \brief The parameter key. */
  std::string key;

  /*! \brief The parameter id.
   *
   * The parameter id is freely choosen when adding the parameter to the filter
   * builder and can be used to identify parameters in code.
   */
  size_t id;

  /*! \brief The parameter value. */
  variant_type value;
};

} // namespace pwcpp::filter
