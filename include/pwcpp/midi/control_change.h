#pragma once

#include <cstdint>
#include <iostream>
#include <ostream>

namespace pwcpp::midi {

struct control_change {
  uint8_t channel;
  uint8_t cc_number;
  uint8_t value;
};

inline void print(control_change &control_change) {
  std::cout << "control_change{channel = "
            << static_cast<int>(control_change.channel)
            << ", cc_number = " << static_cast<int>(control_change.cc_number)
            << ", value = " << static_cast<int>(control_change.value)
            << std::endl;
}

} // namespace pwcpp::midi
