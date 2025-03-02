#pragma once

#include "pwcpp/error.h"

#include <expected>
#include <string>
#include <utility>

namespace pwcpp::property {
using property_value_type = std::variant<int, float, double, std::string>;

class Property {
public:
  virtual ~Property() = default;
  virtual std::expected<void, pwcpp::error> add_to_pod_object(
    spa_pod_builder *builder) = 0;
};

class SimpleProperty : public Property {
public:
  explicit
  SimpleProperty(property_value_type value) : value(std::move(value)) {}

  ~SimpleProperty() override = default;

  std::expected<void, pwcpp::error>
  add_to_pod_object(spa_pod_builder *builder) override {
    return std::unexpected(error::not_implemented());
  }

private:
  property_value_type value{};
};

class ParametersProperty : public Property {};
}
