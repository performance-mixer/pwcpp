#pragma once

#include "pwcpp/error.h"
#include "pwcpp/filter/port.h"
#include "pwcpp/midi/control_change.h"
#include "pwcpp/midi/message.h"

#include <array>
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
        }) {}

  Buffer(pipewire_buffer_enqueue buffer_enqueue,
         pipewire_pod_converter pod_converter)
      : buffer(nullptr), port(nullptr), buffer_enqueue(buffer_enqueue),
        pod_converter(pod_converter) {}

  std::optional<spa_pod *> get_pod() { return pod_converter(buffer, 0); }

  std::expected<std::array<std::optional<midi::message>, 4>, error>
  parse_midi() {
    auto pod = pod_converter(buffer, 0);

    if (!pod.has_value()) {
      return std::expected<std::array<std::optional<midi::message>, 4>, error>(
          {std::nullopt, std::nullopt, std::nullopt, std::nullopt});
    }

    if (!spa_pod_is_sequence(pod.value())) {
      return std::unexpected(error::midi_parsing_pod_not_a_sequence());
    }

    auto sequence = reinterpret_cast<struct spa_pod_sequence *>(pod.value());

    std::array<std::optional<midi::message>, 4> result_messages{
        std::nullopt, std::nullopt, std::nullopt, std::nullopt};

    struct spa_pod_control *pod_control;
    size_t index(0);
    SPA_POD_SEQUENCE_FOREACH(sequence, pod_control) {
      if (index >= 4) {
        return std::unexpected(error::midi_parsing_too_many_messages());
      }

      if (pod_control->type != SPA_CONTROL_Midi) {
        continue;
      }

      uint8_t *data = nullptr;
      uint32_t length;
      spa_pod_get_bytes(&pod_control->value, (const void **)&data, &length);

      if (length != 3) {
        continue;
      }

      if (data[0] < 0b10000000) {
        continue;
      }

      uint8_t channel = data[0] & 0b00001111;
      uint8_t message_type = data[0] & 0b11110000;

      if (message_type != 0b10110000) {
        continue;
      }

      result_messages[index] = midi::control_change{channel, data[1], data[2]};

      index++;
    }

    return result_messages;
  }

  void finish() { buffer_enqueue(buffer, port); }

  pw_buffer *buffer;
  struct pwcpp::filter::port *port;
  pipewire_buffer_enqueue buffer_enqueue;
  pipewire_pod_converter pod_converter;
};

} // namespace pwcpp
