#pragma once

#include "pwcpp/midi/control_change.h"
#include "pwcpp/midi/note.h"

#include <variant>

namespace pwcpp::midi {
using message = std::variant<control_change, note_off, note_on>;

inline void print(message &message) {
  std::visit([](auto &m) { print(m); }, message);
}
}
