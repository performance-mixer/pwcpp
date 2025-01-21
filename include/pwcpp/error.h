#pragma once

#include <string>

namespace pwcpp {

enum class error_type {
  UNSUPPORTED_CONFIGURATION,
  MIDI_PARSING_POD_NOT_A_SEQUENCE,
  MIDI_PARSING_POD_CONTAINS_TOO_MANY_MESSAGES,
  NOT_IMPLEMENTED,
  ERROR_HANDLING_PROPERTY,
};

struct error {
  std::string message;
  error_type type;

  static struct error configuration() {
    return {"Unsupported configuration", error_type::UNSUPPORTED_CONFIGURATION};
  }

  static struct error midi_parsing_pod_not_a_sequence() {
    return {"Midi Pod is not a sequence",
            error_type::MIDI_PARSING_POD_NOT_A_SEQUENCE};
  }

  static struct error midi_parsing_too_many_messages() {
    return {"Midi Pod contains too many messages",
            error_type::MIDI_PARSING_POD_CONTAINS_TOO_MANY_MESSAGES};
  }

  static struct error not_implemented() {
    return {"Not implemented", error_type::NOT_IMPLEMENTED};
  }

  static struct error error_handling_property() {
    return {"Error handling property", error_type::ERROR_HANDLING_PROPERTY};
  }
};
} // namespace pwcpp
