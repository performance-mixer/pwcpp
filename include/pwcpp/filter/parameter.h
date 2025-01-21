#pragma once

#include "pwcpp/error.h"
#include "pwcpp/filter/app.h"

#include <spa/pod/pod.h>

#include <expected>

namespace pwcpp::filter {

template <typename TParams, typename TData> class ParameterCollection {
  std::expected<TParams, error> parse_and_mix_parameters(spa_pod *pod,
                                                         App<TData> &app) {}

  bool handle_parameter_updates(TParams &params, App<TData> &app) {
    return false;
  }

  std::string to_display(TParams &params) { return ""; }
};

} // namespace pwcpp::filter
