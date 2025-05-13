#pragma once

#include "pwcpp/buffer.h"
#include "pwcpp/error.h"
#include "pwcpp/midi/message.h"

#include <array>
#include <cstddef>
#include <expected>
#include <optional>

#include "note.h"

namespace pwcpp::midi {
inline std::optional<midi::message> parse_ump_64(const void *data) {
  if (data == nullptr) {
    return std::nullopt;
  }

  auto *d = static_cast<const uint32_t*>(data);

  const uint8_t message_type = (d[0] >> 28) & 0xf;

  if (message_type < 0x4 || message_type > 0x7) {
    return std::nullopt;
  }

  // midi v2
  if (message_type == 0x4) {
    uint8_t status = d[0] >> 16;
    if (status >= 0xb0 && status <= 0xbf) {
      return control_change{
        .channel = static_cast<unsigned char>((status & 0x0f)),
        .cc_number = static_cast<unsigned char>(d[0] >> 8 & 0x7f), .value = d[1]
      };
    }

    if (status >= 0x80 && status <= 0x8f) {
      return note_off{
        .channel = static_cast<unsigned char>((status & 0x0f)),
        .note = static_cast<unsigned char>(d[0] >> 8 & 0x7f),
        .velocity = static_cast<uint16_t>((d[1] >> 16) & 0xffff)
      };
    }

    if (status >= 0x90 && status <= 0x9f) {
      return note_on{
        .channel = static_cast<unsigned char>((status & 0x0f)),
        .note = static_cast<unsigned char>(d[0] >> 8 & 0x7f),
        .velocity = static_cast<uint16_t>((d[1] >> 16) & 0xffff)
      };
    }
  }

  return std::nullopt;
}

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

  auto sequence = reinterpret_cast<struct spa_pod_sequence*>(pod.value());

  std::array<std::optional<midi::message>, MAX_N> result_messages;
  struct spa_pod_control *pod_control;
  size_t index(0);
  SPA_POD_SEQUENCE_FOREACH(sequence, pod_control) {
    if (index >= MAX_N) {
      return std::unexpected(error::midi_parsing_too_many_messages());
    }

    if (pod_control->type != SPA_CONTROL_UMP) {
      continue;
    }

    const void *data = SPA_POD_BODY(&pod_control->value);
    uint32_t length = SPA_POD_BODY_SIZE(&pod_control->value);

    if (length != 8) {
      continue;
    }

    auto message = parse_ump_64(data);

    if (message.has_value()) {
      result_messages[index++] = message;
    }
  }

  return result_messages;
}
} // namespace pwcpp::midi
