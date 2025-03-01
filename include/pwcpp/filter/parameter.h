#pragma once

#include <string>
#include <utility>
#include <variant>
#include <pwcpp/spa/pod/utils.h>

namespace pwcpp::filter {
class Parameter {
public:
  Parameter(std::string key, size_t id,
            pwcpp::spa::pod::param_value_variant value)
    : key(std::move(key)), id(id), value(std::move(value)) {}

  std::string key;
  size_t id;
  pwcpp::spa::pod::param_value_variant value;

  template <typename T>
  const T &get() const { return std::get<T>(value); }
};
} // namespace pwcpp::filter
