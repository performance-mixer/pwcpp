#pragma once

#include <optional>
#include <string>
#include <variant>

namespace pwcpp::filter {

using variant_type =
    std::variant<int, float, double, std::string, std::nullopt_t>;

class Parameter {
public:
  Parameter(std::string key, size_t id, variant_type value)
      : key(key), id(id), value(value) {}

  std::string key;
  size_t id;
  variant_type value;

  template <typename T> const T &get() const { return std::get<T>(value); }
};

} // namespace pwcpp::filter
