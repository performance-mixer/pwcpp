#include <iostream>
#include <pwcpp/filter/app_builder.h>

struct my_data {
  int example_property;
};

int main(int argc, char *argv[]) {
  pwcpp::filter::AppBuilder<my_data> builder;
  builder.set_filter_name("property").set_media_type("Midi").
          set_media_class("Midi/Sink").add_arguments(argc, argv).
          add_signal_processor([](auto position, const auto &in_ports,
                                  const auto &out_ports, my_data) {});

  auto filter_app = builder.build();
  if (filter_app.has_value()) {
    filter_app.value()->run();
  } else {
    std::cout << "Error: " << filter_app.error().message << std::endl;
  }
}
