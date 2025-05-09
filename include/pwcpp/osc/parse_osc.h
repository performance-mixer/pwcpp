#pragma once

#include "pwcpp/buffer.h"
#include "pwcpp/error.h"

#include <oscpp/server.hpp>

#include <array>
#include <cstddef>
#include <expected>
#include <optional>

namespace pwcpp::osc {
template <std::size_t MAX_N>
std::expected<std::array<std::optional<OSCPP::Server::Packet>, MAX_N>, error>
parse_osc(Buffer &buffer) {
  auto pod = buffer.get_pod(0);

  if (!pod.has_value()) {
    std::cout << "pod is empty" << std::endl;
    return std::expected<std::array<std::optional<OSCPP::Server::Packet>, MAX_N>
                         , error>();
  }

  if (!spa_pod_is_sequence(pod.value())) {
    std::cout << "pod is not a sequence" << std::endl;
    return std::unexpected(error::midi_parsing_pod_not_a_sequence());
  }

  auto sequence = reinterpret_cast<struct spa_pod_sequence*>(pod.value());

  std::array<std::optional<OSCPP::Server::Packet>, MAX_N> result_messages;
  struct spa_pod_control *pod_control;
  size_t index(0);
  SPA_POD_SEQUENCE_FOREACH(sequence, pod_control) {
    std::cout << "index: " << index << std::endl;
    if (index >= MAX_N) {
      std::cout << "too many messages" << std::endl;
      return std::unexpected(error::midi_parsing_too_many_messages());
    }

    if (pod_control->type != SPA_CONTROL_OSC) {
      std::cout << "not an osc message" << std::endl;
      continue;
    }

    uint8_t *data = nullptr;
    uint32_t length;
    spa_pod_get_bytes(&pod_control->value, (const void**)&data, &length);
    std::cout << "data: " << data << " length: " << length << std::endl;
    result_messages[index] = OSCPP::Server::Packet(data, length);
  }

  return result_messages;
}
} // namespace pwcpp::osc
