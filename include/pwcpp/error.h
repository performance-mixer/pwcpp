#pragma once

#include <string>

namespace pwcpp {

/*! \brief An error type.
 *
 * The error type is used to categorize errors.
 */
enum class error_type {
  UNSUPPORTED_CONFIGURATION,
  MIDI_PARSING_POD_NOT_A_SEQUENCE,
  MIDI_PARSING_POD_CONTAINS_TOO_MANY_MESSAGES,
  NOT_IMPLEMENTED,
  ERROR_HANDLING_PROPERTY,
};

/*! \brief An error.
 *
 * Indicates an error during procesisng. The error type allows automatic error
 * handling, the message is a human readable description of the error. The
 * class provides static methods to create common errors.
 */
struct error {
  /*! \brief The error message. */
  std::string message;

  /*! \brief The error type. */
  error_type type;

  /*! \brief Create an error for unsupported configuration. */
  static struct error configuration() {
    return {"Unsupported configuration", error_type::UNSUPPORTED_CONFIGURATION};
  }

  /*! \brief Create an error to indicate that the pod is not a sequence. */
  static struct error midi_parsing_pod_not_a_sequence() {
    return {"Midi Pod is not a sequence",
            error_type::MIDI_PARSING_POD_NOT_A_SEQUENCE};
  }

  /*! \brief Create an error to indicate that the pod contains more midi
   * messages than expected. */
  static struct error midi_parsing_too_many_messages() {
    return {"Midi Pod contains too many messages",
            error_type::MIDI_PARSING_POD_CONTAINS_TOO_MANY_MESSAGES};
  }

  /*! \brief Create an error to indicate that the feature is not implemented. */
  static struct error not_implemented() {
    return {"Not implemented", error_type::NOT_IMPLEMENTED};
  }

  /*! \brief Create an error to indicate that a property could not be handled.
   */
  static struct error error_handling_property() {
    return {"Error handling property", error_type::ERROR_HANDLING_PROPERTY};
  }
};

} // namespace pwcpp
