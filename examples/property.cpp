#include <iostream>
#include <pwcpp/filter/app_builder.h>

struct my_data {
  int example_property;
};

int main(int argc, char *argv[]) {
  pwcpp::filter::AppBuilder<my_data> builder;
  builder.set_filter_name("property")
      .set_media_type("Midi")
      .set_media_class("Midi/Sink")
      .add_arguments(argc, argv)
      .add_property<int>(
          "example.property", "42", static_cast<spa_prop>(0x1000042),
          [](int &property_value, pwcpp::filter::App<my_data> &app) {
            app.user_data.example_property = property_value;
            return true;
          },
          [](spa_pod *pod, pwcpp::filter::App<my_data> &app)
              -> std::expected<int, pwcpp::error> {
            int32_t value;
            spa_pod_get_int(pod, &value);
            return value;
          },
          [](auto &value) { return std::to_string(value); })
      .add_signal_processor(
          [](auto position, auto in_ports, auto out_ports, my_data) {});

  auto filter_app = builder.build();
  if (filter_app.has_value()) {
    filter_app.value()->run();
  } else {
    std::cout << "Error: " << filter_app.error().message << std::endl;
  }
}
