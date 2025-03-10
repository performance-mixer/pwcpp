#pragma once

#include "pwcpp/error.h"

#include <expected>
#include <string>
#include <utility>
#include <variant>
#include <optional>
#include <ostream>

#include <spa/param/props.h>
#include <spa/pod/builder.h>

namespace pwcpp::property {
using property_value_type = std::variant<
  int, long, float, double, std::string, bool, std::nullopt_t>;

inline std::expected<void, pwcpp::error>
write_property_value(spa_pod_builder *builder, property_value_type value) {
  if (std::holds_alternative<int>(value)) {
    spa_pod_builder_int(builder, std::get<int>(value));
  } else if (std::holds_alternative<long>(value)) {
    spa_pod_builder_long(builder, std::get<long>(value));
  } else if (std::holds_alternative<float>(value)) {
    spa_pod_builder_float(builder, std::get<float>(value));
  } else if (std::holds_alternative<double>(value)) {
    spa_pod_builder_double(builder, std::get<double>(value));
  } else if (std::holds_alternative<std::string>(value)) {
    spa_pod_builder_string(builder, std::get<std::string>(value).c_str());
  } else if (std::holds_alternative<bool>(value)) {
    spa_pod_builder_bool(builder, std::get<bool>(value));
  } else if (std::holds_alternative<std::nullopt_t>(value)) {
    spa_pod_builder_none(builder);
  }

  return {};
}

class Property {
public:
  explicit Property(spa_prop key) : _key(key) {}

  virtual ~Property() = default;
  virtual std::expected<void, pwcpp::error> add_to_pod_object(
    spa_pod_builder *builder) = 0;

protected:
  spa_prop _key{};
};

class SimpleProperty : public Property {
public:
  explicit
  SimpleProperty(spa_prop key, property_value_type value) : Property(key),
                                                            _value(std::move(
                                                              value)) {}

  ~SimpleProperty() override = default;

  std::expected<void, pwcpp::error>
  add_to_pod_object(spa_pod_builder *builder) override {
    spa_pod_builder_prop(builder, _key, 0);
    return write_property_value(builder, _value);
  }

  [[nodiscard]] const property_value_type &value() const { return _value; }

private:
  property_value_type _value{};
};
}

template <typename VType>
inline void print(std::ostream &os, const VType &value) {
  if (std::holds_alternative<int>(value)) {
    os << std::get<int>(value);
  } else if (std::holds_alternative<float>(value)) {
    os << std::get<float>(value);
  } else if (std::holds_alternative<double>(value)) {
    os << std::get<double>(value);
  } else if (std::holds_alternative<std::string>(value)) {
    os << std::get<std::string>(value);
  } else if (std::holds_alternative<bool>(value)) {
    os << (std::get<bool>(value) ? "true" : "false");
  } else if (std::holds_alternative<std::nullopt_t>(value)) {
    os << "nullopt";
  }
}

inline std::ostream &operator<<(std::ostream &os,
                                const pwcpp::property::property_value_type &
                                value) {
  print(os, value);
  return os;
}
