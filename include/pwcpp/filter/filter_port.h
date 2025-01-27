#pragma once

#include "pipewire/filter.h"
#include "pwcpp/buffer.h"
#include "pwcpp/filter/port.h"

#include <functional>
#include <optional>
#include <pipewire/stream.h>

namespace pwcpp::filter {

/*! \brief Dequeue a buffer from a pipewire port.
 *
 * Normally supplied in the default constructor and only overridden for unit
 * tests.
 */
using pipewire_buffer_dequeue =
    std::function<pw_buffer *(struct pwcpp::filter::port *)>;

/*! \brief C++ wrapper for a pipewire port.
 *
 * This class wraps a pipewire port and provides a C++ interface for
 * interacting with it.
 */
class FilterPort {
public:
  /*! \brief Construct a filter port for testing.
   */
  FilterPort(pipewire_buffer_dequeue buffer_dequeue,
             pipewire_buffer_enqueue buffer_enqueue)
      : port(nullptr), buffer_dequeue(buffer_dequeue),
        buffer_enqueue(buffer_enqueue) {}

  /*! \brief Construct a filter port.
   *
   * The construtor is normally not used directly. Pwcpp creates the port when
   * the filter app builder builds the filter.
   *
   * \param port The pipewire port to wrap.
   */
  FilterPort(struct pwcpp::filter::port *port)
      : port(port), buffer_dequeue([](struct pwcpp::filter::port *port) {
          return pw_filter_dequeue_buffer(port);
        }),
        buffer_enqueue([](pw_buffer *buffer, struct pwcpp::filter::port *port) {
          pw_filter_queue_buffer(port, buffer);
        }) {}

  /*! \brief Get the buffer from the port.
   *
   * \return A buffer if one is available, otherwise an empty optional.
   */
  std::optional<pwcpp::Buffer> get_buffer() {
    auto buffer = buffer_dequeue(port);
    if (buffer == nullptr)
      return {};

    return Buffer(buffer, port, buffer_enqueue);
  }

  struct pwcpp::filter::port *port;
  pipewire_buffer_dequeue buffer_dequeue;
  pipewire_buffer_enqueue buffer_enqueue;
};

} // namespace pwcpp::filter
