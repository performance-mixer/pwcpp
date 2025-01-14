#include "pipewire/stream.h"
#include "pwcpp/buffer.h"

#include <algorithm>
#include <optional>

#include <spa/control/control.h>
#include <spa/pod/builder.h>

#include <ftest/count_calls.h>

#include <microtest/microtest.h>
#include <variant>

TEST(GetMidiCCMessaseFromBuffer) {
  char *pod_buffer[4096];
  pwcpp::Buffer buffer(
      [](pw_buffer *, struct pwcpp::filter::port *) {},
      [&pod_buffer](pw_buffer *pw_buffer,
                    size_t index) -> std::optional<struct spa_pod *> {
        if (index != 0)
          return {};

        struct spa_pod_builder builder;

        spa_pod_builder_init(&builder, pod_buffer, 4096);

        struct spa_pod_frame frame;
        spa_pod_builder_push_sequence(&builder, &frame, 0);
        spa_pod_builder_control(&builder, 0, SPA_CONTROL_Midi);

        unsigned char midi_message[3] = {0b10110010, 3, 7};
        spa_pod_builder_bytes(&builder, midi_message, sizeof(midi_message));

        return static_cast<spa_pod *>(spa_pod_builder_pop(&builder, &frame));
      });

  auto midi_messages = buffer.parse_midi();
  ASSERT_TRUE(midi_messages.has_value());
  ASSERT_EQ(std::ranges::count_if(
                midi_messages.value(),
                [](auto &&midi_message) { return midi_message.has_value(); }),
            1);

  auto midi_message = midi_messages.value()[0];
  ASSERT_TRUE(midi_message.has_value());
  ASSERT_TRUE(std::holds_alternative<pwcpp::midi::control_change>(
      midi_message.value()));

  auto control_change =
      std::get<pwcpp::midi::control_change>(midi_message.value());
  ASSERT_EQ(control_change.channel, 2);
  ASSERT_EQ(control_change.cc_number, 3);
  ASSERT_EQ(control_change.value, 7);
}

TEST_MAIN()
