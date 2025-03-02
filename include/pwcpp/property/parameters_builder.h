#pragma once

#include "pwcpp/property/parameters_property.h"

#include <string>
#include <tuple>
#include <vector>
#include <memory>

namespace pwcpp::property {
template <typename TAppBuilder>
class ParametersBuilder {
public:
  explicit ParametersBuilder(TAppBuilder &builder)
    : builder(builder) {}

  ParametersBuilder &add(std::string name, property_value_type value) {
    _parameters.emplace_back(std::move(name), std::move(value));
    return *this;
  }

  std::shared_ptr<ParametersProperty> build() {
    return std::make_shared<ParametersProperty>(_parameters);
  }

  TAppBuilder &finish() {
    return builder;
  }

private:
  std::vector<std::tuple<std::string, property_value_type>> _parameters{};
  TAppBuilder &builder;
};
}
