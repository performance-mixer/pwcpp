#pragma once

#include "pipewire/filter.h"
#include "pwcpp/buffer.h"
#include "pwcpp/filter/port.h"

#include <functional>
#include <optional>
#include <pipewire/stream.h>

namespace pwcpp::filter {

using pipewire_buffer_dequeue =
    std::function<pw_buffer *(struct pwcpp::filter::port *)>;

class FilterPort {
public:
  FilterPort(pipewire_buffer_dequeue buffer_dequeue,
             pipewire_buffer_enqueue buffer_enqueue)
      : port(nullptr), buffer_dequeue(buffer_dequeue),
        buffer_enqueue(buffer_enqueue) {}

  FilterPort(struct pwcpp::filter::port *port)
      : port(port), buffer_dequeue([](struct pwcpp::filter::port *port) {
          return pw_filter_dequeue_buffer(port);
        }),
        buffer_enqueue([](pw_buffer *buffer, struct pwcpp::filter::port *port) {
          pw_filter_queue_buffer(port, buffer);
        }) {}

  std::optional<pwcpp::Buffer> get_buffer() {
    auto buffer = buffer_dequeue(port);
    if (buffer == nullptr)
      return {};

    return Buffer(buffer, port, buffer_enqueue);
  }

  struct pwcpp::filter::port *port;
  pipewire_buffer_dequeue buffer_dequeue;
  pipewire_buffer_enqueue buffer_enqueue;
};

} // namespace pwcpp::filter
