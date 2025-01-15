#pragma once

#include "pwcpp/filter/port.h"

#include <cstddef>
#include <expected>
#include <functional>
#include <optional>

#include <pipewire/stream.h>

#include <spa/control/control.h>
#include <spa/pod/iter.h>
#include <spa/pod/pod.h>

namespace pwcpp {

using pipewire_buffer_enqueue =
    std::function<void(pw_buffer *buffer, struct pwcpp::filter::port *)>;

using pipewire_pod_converter =
    std::function<std::optional<spa_pod *>(pw_buffer *, size_t)>;

using spa_data_provider = std::function<std::optional<struct spa_data>(
    pw_buffer *pw_buffer, std::size_t index)>;

class Buffer {
public:
  Buffer(pw_buffer *buffer, struct pwcpp::filter::port *port,
         pipewire_buffer_enqueue buffer_enqueue)
      : buffer(buffer), port(port), buffer_enqueue(buffer_enqueue),
        pod_converter([](pw_buffer *pw_buffer,
                         size_t index) -> std::optional<struct spa_pod *> {
          if (pw_buffer->buffer->n_datas > index) {
            return static_cast<struct spa_pod *>(
                spa_pod_from_data(pw_buffer->buffer->datas[index].data,
                                  pw_buffer->buffer->datas[index].maxsize,
                                  pw_buffer->buffer->datas[index].chunk->offset,
                                  pw_buffer->buffer->datas[index].chunk->size));
          }

          return std::nullopt;
        }),
        spa_data_provider(
            [](pw_buffer *pw_buffer,
               std::size_t index) -> std::optional<struct spa_data> {
              struct spa_buffer *spa_buffer;
              spa_buffer = pw_buffer->buffer;
              if (spa_buffer->datas[0].data == NULL) {
                return std::nullopt;
              }

              auto spa_data = spa_buffer->datas[0];
              spa_data.chunk->offset = 0;
              spa_data.chunk->size = 0;
              spa_data.chunk->stride = 1;
              spa_data.chunk->flags = 0;

              return spa_data;
            }) {}

  Buffer(pipewire_buffer_enqueue buffer_enqueue,
         pipewire_pod_converter pod_converter)
      : buffer(nullptr), port(nullptr), buffer_enqueue(buffer_enqueue),
        pod_converter(pod_converter) {}

  std::optional<spa_pod *> get_pod(std::size_t index) {
    return pod_converter(buffer, index);
  }

  void finish() { buffer_enqueue(buffer, port); }

  std::optional<struct spa_data> get_spa_data(std::size_t index) {
    return spa_data_provider(buffer, index);
  }

  pw_buffer *buffer;
  struct pwcpp::filter::port *port;
  pipewire_buffer_enqueue buffer_enqueue;
  pipewire_pod_converter pod_converter;
  pwcpp::spa_data_provider spa_data_provider;
};

} // namespace pwcpp
