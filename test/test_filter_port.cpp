#include "pwcpp/filter/filter_port.h"

#include <ftest/count_calls.h>
#include <microtest/microtest.h>

TEST(GetTheBufferFromAPort) {
  ftest::CountCalls<pw_buffer *, struct pwcpp::filter::port *>
      call_counter_enqueue;

  pwcpp::filter::FilterPort port(
      [](struct pwcpp::filter::port *) { return (struct pw_buffer *)42; },
      [&call_counter_enqueue](pw_buffer *buffer,
                              struct pwcpp::filter::port *port) {
        call_counter_enqueue(buffer, port);
      });

  auto buffer = port.get_buffer();
  ASSERT_TRUE(buffer);
  ASSERT_EQ(buffer->buffer, (struct pw_buffer *)42);

  buffer->finish();
  ASSERT_EQ(call_counter_enqueue.call_arguments.size(), 1);
  ASSERT_EQ(std::get<0>(*call_counter_enqueue.call_arguments.begin()),
            (struct pw_buffer *)42);
  ASSERT_EQ(std::get<1>(*call_counter_enqueue.call_arguments.begin()),
            port.port);
}

TEST(GetNoBufferFromAPort) {
  pwcpp::filter::FilterPort port(
      [](struct pwcpp::filter::port *) { return nullptr; },
      [](pw_buffer *buffer, struct pwcpp::filter::port *) {});

  auto buffer = port.get_buffer();
  ASSERT_FALSE(buffer);
}

TEST_MAIN();
