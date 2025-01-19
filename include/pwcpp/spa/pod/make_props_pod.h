#pragma once

#include "pwcpp/error.h"
#include <spa/param/param.h>

#include <expected>
#include <spa/pod/builder.h>
#include <spa/pod/pod.h>

namespace pwcpp::spa::pod {

template <typename T>
inline std::expected<spa_pod *, error>
make_props_pod(u_int8_t *buffer, size_t buffer_size, int props_type, T value) {
  return std::unexpected(error::not_implemented());
}

template <>
inline std::expected<spa_pod *, error>
make_props_pod<int>(u_int8_t *buffer, size_t buffer_size, int props_type,
                    int value) {
  struct spa_pod_builder builder;
  spa_pod_builder_init(&builder, buffer, buffer_size);

  struct spa_pod_frame object_frame;
  spa_pod_builder_push_object(&builder, &object_frame, SPA_TYPE_OBJECT_Props,
                              SPA_PARAM_Props);
  spa_pod_builder_prop(&builder, props_type, 0);
  spa_pod_builder_int(&builder, value);

  return static_cast<spa_pod *>(spa_pod_builder_pop(&builder, &object_frame));
}

} // namespace pwcpp::spa::pod
