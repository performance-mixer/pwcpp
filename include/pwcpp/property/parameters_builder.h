#pragma once

#include "pwcpp/property/property.h"

#include <string>
#include <tuple>
#include <vector>
#include <memory>

namespace pwcpp::property {
class ParametersBuilder {
public:
  ParametersBuilder &add(std::string name, property_value_type value) {
    _parameters.emplace_back(std::move(name), std::move(value));
    return *this;
  }

  std::shared_ptr<ParametersProperty> build() {
    return std::make_shared<ParametersProperty>(_parameters);
  }

private:
  std::vector<std::tuple<std::string, property_value_type>> _parameters{};
};
}
