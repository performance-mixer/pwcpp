#pragma once

#include "pwcpp/error.h"

#include <expected>
#include <string>
#include <utility>

namespace pwcpp::property {
using property_value_type = std::variant<int, float, double, std::string, bool>;

inline std::expected<void, pwcpp::error>
write_property_value(spa_pod_builder *builder, property_value_type value) {
  if (std::holds_alternative<int>(value)) {
    spa_pod_builder_int(builder, std::get<int>(value));
  } else if (std::holds_alternative<float>(value)) {
    spa_pod_builder_float(builder, std::get<float>(value));
  } else if (std::holds_alternative<double>(value)) {
    spa_pod_builder_double(builder, std::get<double>(value));
  } else if (std::holds_alternative<std::string>(value)) {
    spa_pod_builder_string(builder, std::get<std::string>(value).c_str());
  } else if (std::holds_alternative<bool>(value)) {
    spa_pod_builder_bool(builder, std::get<bool>(value));
  }

  return std::unexpected(error::error_handling_property());
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

  const property_value_type &value() const { return _value; }

private:
  property_value_type _value{};
};

class ParametersProperty : public Property {
public:
  ParametersProperty(
    std::vector<std::tuple<std::string, property_value_type>> parameters) :
    Property(SPA_PROP_params), _parameters(std::move(parameters)) {}

  ~ParametersProperty() override = default;

  std::expected<void, error>
  add_to_pod_object(spa_pod_builder *builder) override {
    if (!_parameters.empty()) {
      spa_pod_builder_prop(builder, _key, 0);
      spa_pod_frame frame;
      spa_pod_builder_push_struct(builder, &frame);
      for (auto &parameter : _parameters) {
        spa_pod_builder_string(builder, std::get<0>(parameter).c_str());
        auto result = write_property_value(builder, std::get<1>(parameter));
        if (!result.has_value()) {
          return std::unexpected(result.error());
        }
      }
    }

    return std::unexpected(error::not_implemented());
  };

private:
  std::vector<std::tuple<std::string, property_value_type>> _parameters{};
};
}
