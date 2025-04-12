#pragma once

#include "pwcpp/midi/control_change.h"

#include <variant>

namespace pwcpp::midi {
using message = std::variant<control_change>;

inline void print(message &message) {
  std::visit([](auto &m) { print(m); }, message);
}
}
