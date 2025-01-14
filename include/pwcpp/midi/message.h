#pragma once

#include "pwcpp/midi/control_change.h"

#include <variant>

namespace pwcpp::midi {

using message = std::variant<control_change>;

}
