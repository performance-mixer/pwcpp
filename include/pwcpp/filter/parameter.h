#pragma once

#include <string>
#include <variant>

namespace pwcpp::filter {

using variant_type = std::variant<int, float, double, std::string>;

class Parameter {
public:
  Parameter(std::string key, variant_type value) : key(key), value(value) {}

  std::string key;
  variant_type value;

  template <typename T> const T &get() const { return std::get<T>(value); }
};

} // namespace pwcpp::filter
