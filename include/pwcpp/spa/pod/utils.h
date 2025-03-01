#pragma once

#include <string>
#include <tuple>
#include <variant>
#include <vector>
#include <optional>
#include <span>

#include <spa/param/param.h>
#include <spa/param/props.h>
#include <spa/pod/builder.h>
#include <pipewire/loop.h>

namespace pwcpp::spa::pod {
using param_value_variant = std::variant<
  double, std::string, float, bool, int, std::nullopt_t>;

inline spa_pod *build_set_params_message(u_int8_t *buffer,
                                         const size_t buffer_size,
                                         std::span<std::tuple<
                                           std::string, param_value_variant>>
                                         parameters) {
  spa_pod_builder builder{};
  spa_pod_builder_init(&builder, buffer, buffer_size);

  spa_pod_frame object_frame{};
  spa_pod_builder_push_object(&builder, &object_frame, SPA_TYPE_OBJECT_Props,
                              SPA_PARAM_Props);
  spa_pod_builder_prop(&builder, SPA_PROP_params, 0);

  spa_pod_frame struct_frame{};
  spa_pod_builder_push_struct(&builder, &struct_frame);

  for (auto &&[key, value] : parameters) {
    spa_pod_builder_string(&builder, key.c_str());
    if (std::holds_alternative<double>(value)) {
      spa_pod_builder_double(&builder, std::get<double>(value));
    } else if (std::holds_alternative<std::string>(value)) {
      spa_pod_builder_string(&builder, std::get<std::string>(value).c_str());
    } else if (std::holds_alternative<int>(value)) {
      spa_pod_builder_int(&builder, std::get<int>(value));
    } else if (std::holds_alternative<float>(value)) {
      spa_pod_builder_float(&builder, std::get<float>(value));
    } else if (std::holds_alternative<bool>(value)) {
      spa_pod_builder_bool(&builder, std::get<bool>(value));
    } else if (std::holds_alternative<std::nullopt_t>(value)) {
      spa_pod_builder_none(&builder);
    }
  }

  spa_pod_builder_pop(&builder, &struct_frame);

  return static_cast<spa_pod*>(spa_pod_builder_pop(&builder, &object_frame));
}
}
