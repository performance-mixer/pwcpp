#include <string>

#include <pwcpp/filter/app_builder.h>
#include <pwcpp/midi/parse_midi.h>

int main(int argc, char *argv[]) {
  std::string dsp_format = "32 bit raw UMP";

  pwcpp::filter::AppBuilder<std::nullptr_t> builder;
  builder.set_filter_name("parse midi").set_media_type("Midi").
          set_media_class("Midi/Sink").add_arguments(argc, argv).
          add_input_port("input", dsp_format).add_signal_processor(
            [](auto position, const auto &in_ports, const auto &,
               auto &parameters, std::nullptr_t) {
              for (auto &&port : in_ports) {
                auto buffer = port->get_buffer();
                if (buffer.has_value()) {
                  auto buffer_midi_messages = pwcpp::midi::parse_midi<16>(buffer.value());
                  if (buffer_midi_messages.has_value()) {
                    for (auto &&midi_message : buffer_midi_messages.value()) {
                      if (midi_message.has_value()) {
                        pwcpp::midi::print(midi_message.value());
                      }
                    }
                  }

                  buffer.value().finish();
                }
              }
            });

  auto filter_app = builder.build();
  if (filter_app.has_value()) {
    filter_app.value()->run();
  }
}