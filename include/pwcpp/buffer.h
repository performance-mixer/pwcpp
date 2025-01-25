#pragma once

#include "pwcpp/filter/port.h"

#include <cstddef>
#include <expected>
#include <functional>
#include <optional>

#include <pipewire/stream.h>

#include <spa/control/control.h>
#include <spa/pod/iter.h>
#include <spa/pod/pod.h>

namespace pwcpp {

/*! \brief Enqueues a buffer in a pipewire port.
 *
 * \param buffer The buffer to enqueue.
 * \param port The port to enqueue the buffer in.
 */
using pipewire_buffer_enqueue =
    std::function<void(pw_buffer *buffer, struct pwcpp::filter::port *)>;

/*! \brief Converts a pipewire buffer to a spa pod.
 *
 * \param pw_buffer The pipewire buffer to read the pod from.
 * \param the length of the data to read.
 *
 * \return The pod.
 */
using pipewire_pod_converter =
    std::function<std::optional<spa_pod *>(pw_buffer *, size_t)>;

/*! \brief Provides spa data from a pipewire buffer.
 *
 * \param pw_buffer The pipewire buffer to read the spa data from.
 * \param index The index of the spa data to read.
 *
 * \return The spa data.
 */
using spa_data_provider = std::function<std::optional<struct spa_data>(
    pw_buffer *pw_buffer, std::size_t index)>;

/*! \brief A pipewire buffer.
 *
 * Wraps a pipewire buffer and provides convenience functions for buffer
 * processing. Buffers are not constructed manually, instead they are retrieved
 * from a pipewire port in the processing function.
 * Buffers have to be enqueued with the same port to indicate that the buffer
 * has been processed. The buffer provides the method Buffer::finish to enqueue
 * the buffer.
 */
class Buffer {
public:
  /*! \brief Construct a buffer.
   *
   * Buffers are generally not directly constructed, instead they are retrieved
   * from a port.
   *
   * \param buffer The pipewire buffer.
   * \param port The pipewire port.
   * \param buffer_enqueue The function to enqueue the buffer to the port when
   * processing is finished.
   */
  Buffer(pw_buffer *buffer, struct pwcpp::filter::port *port,
         pipewire_buffer_enqueue buffer_enqueue)
      : buffer(buffer), port(port), buffer_enqueue(buffer_enqueue),
        pod_converter([](pw_buffer *pw_buffer,
                         size_t index) -> std::optional<struct spa_pod *> {
          if (pw_buffer->buffer->n_datas > index) {
            return static_cast<struct spa_pod *>(
                spa_pod_from_data(pw_buffer->buffer->datas[index].data,
                                  pw_buffer->buffer->datas[index].maxsize,
                                  pw_buffer->buffer->datas[index].chunk->offset,
                                  pw_buffer->buffer->datas[index].chunk->size));
          }

          return std::nullopt;
        }),
        spa_data_provider(
            [](pw_buffer *pw_buffer,
               std::size_t index) -> std::optional<struct spa_data> {
              struct spa_buffer *spa_buffer;
              spa_buffer = pw_buffer->buffer;
              if (spa_buffer->datas[0].data == NULL) {
                return std::nullopt;
              }

              auto spa_data = spa_buffer->datas[0];
              spa_data.chunk->offset = 0;
              spa_data.chunk->size = 0;
              spa_data.chunk->stride = 1;
              spa_data.chunk->flags = 0;

              return spa_data;
            }) {}

  /*! \brief Construct a buffer for testing.
   *
   * \param buffer_enqueue The function to enqueue the buffer.
   * \param pod_converter The function to convert the buffer to a pod.
   */
  Buffer(pipewire_buffer_enqueue buffer_enqueue,
         pipewire_pod_converter pod_converter)
      : buffer(nullptr), port(nullptr), buffer_enqueue(buffer_enqueue),
        pod_converter(pod_converter) {}

  /*! \brief Get the pod from the data in the buffer at the given
   * index for reading .
   *
   * \param index The index of the pod to get.
   *
   * \return The pod if successful, `std::nullopt` otherwise.
   */
  std::optional<spa_pod *> get_pod(std::size_t index) {
    return pod_converter(buffer, index);
  }

  /*! \brief Get the spa data at the given index for writing.
   *
   * \param index The index of the spa data to get.
   *
   * \return The spa data if successful, `std::nullopt` otherwise.
   */
  std::optional<struct spa_data> get_spa_data(std::size_t index) {
    return spa_data_provider(buffer, index);
  }

  /*! \brief Enqueue the buffer to the port and finish processing.
   *
   * \important This function must be called after processing is finished.
   * Otherwise pipewire will not continue procesing of the buffer. it will
   * not send the buffer along if it is an out port and it will not add new
   * data to the buffer if it is an in port.
   */
  void finish() { buffer_enqueue(buffer, port); }

  pw_buffer *buffer;
  struct pwcpp::filter::port *port;
  pipewire_buffer_enqueue buffer_enqueue;
  pipewire_pod_converter pod_converter;
  pwcpp::spa_data_provider spa_data_provider;
};

} // namespace pwcpp
