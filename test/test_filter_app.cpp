#include <cstdint>
#include <memory>
#include <spa/debug/pod.h>

#include <pwcpp/filter/app.h>
#include <pwcpp/spa/pod/make_props_pod.h>

#include <spa/pod/parser.h>

#include <microtest/microtest.h>

#include <ftest/count_calls.h>

TEST(HandlePropertyUpdate) {
  struct my_user_data {
    int value;
  };

  auto user_data = std::make_shared<my_user_data>();
  user_data->value = 55;

  using my_data_type = std::shared_ptr<my_user_data>;
  using my_app_type = pwcpp::filter::App<my_data_type>;
  my_app_type app;

  app.user_data = user_data;

  ftest::CountCalls<int> call_counter_property_handler;
  ftest::CountCalls<spa_pod *> call_counter_property_parser;
  ftest::CountCalls<int> call_counter_property_to_display;
  ftest::CountCalls<std::string, std::string> call_counter_update_property;

  app.update_property = [&call_counter_update_property](auto &property,
                                                        auto value, auto &app) {
    call_counter_update_property(property.name, value);
  };

  pwcpp::filter::PropertyDefPtr<my_app_type> property =
      std::make_shared<pwcpp::filter::PropertyDef<int, my_data_type>>(
          "test", "34", static_cast<spa_prop>(66),
          [&call_counter_property_handler](auto value, auto &&app) {
            call_counter_property_handler(value);
            app.user_data->value = value;
            return true;
          },
          [&call_counter_property_parser](
              spa_pod *pod, auto &&app) -> std::expected<int, pwcpp::error> {
            call_counter_property_parser(pod);
            int32_t value;
            spa_pod_get_int(pod, &value);
            return value;
          },
          [&call_counter_property_to_display](auto value) {
            call_counter_property_to_display(value);
            return std::to_string(value);
          });
  app.properties.push_back(property);

  u_int8_t buffer[4096];
  auto pod = pwcpp::spa::pod::make_props_pod(buffer, 4096, 66, 34);
  ASSERT_TRUE(pod.has_value());
  auto object = reinterpret_cast<spa_pod_object *>(pod.value());
  app.handle_property_update(object);

  ASSERT_EQ(call_counter_property_handler.call_arguments.size(), 1);
  ASSERT_EQ(call_counter_property_parser.call_arguments.size(), 1);
  ASSERT_EQ(call_counter_property_to_display.call_arguments.size(), 1);
  ASSERT_EQ(call_counter_update_property.call_arguments.size(), 1);

  ASSERT_EQ(get<0>(call_counter_property_handler.call_arguments[0]), 34);

  ASSERT_EQ(get<0>(call_counter_update_property.call_arguments[0]), "test");
  ASSERT_EQ(get<1>(call_counter_update_property.call_arguments[0]), "34");

  ASSERT_EQ(app.user_data->value, 34);
}

TEST_MAIN()
