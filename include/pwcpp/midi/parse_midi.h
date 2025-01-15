#pragma once

#include "pwcpp/buffer.h"
#include "pwcpp/error.h"
#include "pwcpp/midi/message.h"

#include <array>
#include <cstddef>
#include <expected>
#include <optional>

namespace pwcpp::midi {

template <std::size_t MAX_N>
std::expected<std::array<std::optional<midi::message>, MAX_N>, error>
parse_midi(Buffer &buffer) {
  auto pod = buffer.get_pod(0);

  if (!pod.has_value()) {
    return std::expected<std::array<std::optional<midi::message>, MAX_N>,
                         error>();
  }

  if (!spa_pod_is_sequence(pod.value())) {
    return std::unexpected(error::midi_parsing_pod_not_a_sequence());
  }

  auto sequence = reinterpret_cast<struct spa_pod_sequence *>(pod.value());

  std::array<std::optional<midi::message>, MAX_N> result_messages;
  struct spa_pod_control *pod_control;
  size_t index(0);
  SPA_POD_SEQUENCE_FOREACH(sequence, pod_control) {
    if (index >= MAX_N) {
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

} // namespace pwcpp::midi
