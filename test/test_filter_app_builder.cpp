#include "pwcpp/error.h"
#include "pwcpp/filter/app_builder.h"
#include "spa/node/io.h"

#include <cstddef>
#include <microtest/microtest.h>

#include <ftest/count_calls.h>

#include <string>

TEST(NoConfigurationIsNotSupported) {
  pwcpp::filter::AppBuilder<std::nullptr_t> builder(
      [](int argc, char *argv[]) {},
      [](auto name, auto media_type, auto media_class, auto properties,
         pwcpp::filter::AppBuilder<std::nullptr_t>::FilterAppPtr app_ptr) {
        return std::make_tuple(nullptr, nullptr);
      },
      [](auto name, auto dsp_format, auto filter) { return nullptr; },
      [](auto name, auto dsp_format, auto filter) { return nullptr; });
  auto result = builder.build();
  ASSERT_FALSE(result.has_value());
  ASSERT(result.error().type == pwcpp::error_type::UNSUPPORTED_CONFIGURATION);
}

TEST(CreateAFilterAppWithInAndOutPort) {
  ftest::CountCalls<int, char **> call_counter_pw_init;
  ftest::CountCalls<std::string, std::string, struct pw_filter *>
      call_counter_in_port_builder;
  ftest::CountCalls<std::string, std::string, struct pw_filter *>
      call_counter_out_port_builder;
  ftest::CountCalls<std::string, std::string, std::string>
      call_counter_filter_builder;

  pwcpp::filter::AppBuilder<std::nullptr_t> builder(
      [&call_counter_pw_init](int argc, char **argv) {
        call_counter_pw_init(argc, argv);
      },
      [&call_counter_filter_builder](
          auto name, auto media_type, auto media_class, auto properties,
          pwcpp::filter::AppBuilder<std::nullptr_t>::FilterAppPtr app_ptr) {
        call_counter_filter_builder(name, media_type, media_class);
        return std::make_tuple((struct pw_main_loop *)4, (struct pw_filter *)5);
      },
      [&call_counter_in_port_builder](auto name, auto dsp_format, auto filter) {
        call_counter_in_port_builder(name, dsp_format, filter);
        return (pwcpp::filter::port *)5;
      },
      [&call_counter_out_port_builder](auto name, auto dsp_format,
                                       auto filter) {
        call_counter_out_port_builder(name, dsp_format, filter);
        return (pwcpp::filter::port *)9;
      });

  char test_argument[] = "test";
  char *test_data[] = {test_argument};
  auto filter_app =
      builder.set_filter_name("arg1")
          .set_media_type("arg2")
          .set_media_class("arg3")
          .add_input_port("test_in", "8 bit raw midi")
          .add_output_port("test_out", "8 bit raw midi")
          .add_arguments(1, test_data)
          .add_signal_processor([](struct spa_io_position *position,
                                   auto &&in_ports, auto &&out_ports,
                                   std::nullptr_t) {})
          .build();

  ASSERT_TRUE(filter_app.has_value());

  // Should have initialized pipewire
  ASSERT_EQ(call_counter_pw_init.call_arguments.size(), 1);
  ASSERT_EQ(std::get<0>(*call_counter_pw_init.call_arguments.begin()), 1);
  ASSERT_EQ(std::get<1>(*call_counter_pw_init.call_arguments.begin())[0],
            test_argument);

  // Should have the filter data
  ASSERT_EQ(call_counter_filter_builder.call_arguments.size(), 1);
  ASSERT_EQ(std::get<0>(*call_counter_filter_builder.call_arguments.begin()),
            "arg1");
  ASSERT_EQ(std::get<1>(*call_counter_filter_builder.call_arguments.begin()),
            "arg2");
  ASSERT_EQ(std::get<2>(*call_counter_filter_builder.call_arguments.begin()),
            "arg3");
  ASSERT_EQ(filter_app.value()->loop, (struct pw_main_loop *)4);
  ASSERT_EQ(filter_app.value()->filter, (struct pw_filter *)5);

  // Should have the in port
  ASSERT_EQ(filter_app.value()->number_of_in_ports(), 1);
  ASSERT_EQ(call_counter_in_port_builder.call_arguments.size(), 1);
  ASSERT_EQ(filter_app.value()->in_ports.begin()->get()->port,
            (pwcpp::filter::port *)5);
  ASSERT_EQ(std::get<0>(*call_counter_in_port_builder.call_arguments.begin()),
            "test_in");
  ASSERT_EQ(std::get<1>(*call_counter_in_port_builder.call_arguments.begin()),
            "8 bit raw midi");
  ASSERT_EQ(std::get<2>(*call_counter_in_port_builder.call_arguments.begin()),
            (struct pw_filter *)5);

  // Should have the out port
  ASSERT_EQ(filter_app.value()->number_of_out_ports(), 1);
  ASSERT_EQ(call_counter_out_port_builder.call_arguments.size(), 1);
  ASSERT_EQ(filter_app.value()->out_ports.begin()->get()->port,
            (pwcpp::filter::port *)9);
  ASSERT_EQ(std::get<0>(*call_counter_out_port_builder.call_arguments.begin()),
            "test_out");
  ASSERT_EQ(std::get<1>(*call_counter_out_port_builder.call_arguments.begin()),
            "8 bit raw midi");
  ASSERT_EQ(std::get<2>(*call_counter_out_port_builder.call_arguments.begin()),
            (struct pw_filter *)5);
}

TEST_MAIN();
