#pragma once

#include "pwcpp/error.h"
#include "pwcpp/filter/parameter.h"

#include <expected>
#include <vector>

#include <spa/pod/pod.h>

namespace pwcpp::filter {

template <typename TData> class ParameterCollection {
public:
  static std::expected<std::vector<Parameter>, error> parse(spa_pod *pod) {}

  bool handle_parameter_updates(std::vector<Parameter> updates) {
    return false;
  }

  std::string to_display() { return ""; }
};

} // namespace pwcpp::filter
