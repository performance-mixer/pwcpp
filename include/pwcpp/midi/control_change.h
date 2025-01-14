#pragma once

#include <cstdint>

namespace pwcpp::midi {

struct control_change {
  uint8_t channel;
  uint8_t cc_number;
  uint8_t value;
};

} // namespace pwcpp::midi
