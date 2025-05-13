#pragma once

#include <cstdint>
#include <iostream>
#include <ostream>

namespace pwcpp::midi {
struct note_on {
  uint8_t channel;
  uint8_t note;
  uint16_t velocity;
};

struct note_off {
  uint8_t channel;
  uint8_t note;
  uint16_t velocity;
};

inline void print(const note_on &note) {
  std::cout << "note_on{channel = " << static_cast<int>(note.channel) <<
    ", note = " << static_cast<int>(note.note) << ", velocity = " << note.
    velocity << "}" << std::endl;
}

inline void print(const note_off &note) {
  std::cout << "note_off{channel = " << static_cast<int>(note.channel) <<
    ", note = " << static_cast<int>(note.note) << ", velocity = " << note.
    velocity << "}" << std::endl;
}
}
