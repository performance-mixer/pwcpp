#pragma once

#include "pwcpp/filter/filter_port.h"

#include <functional>
#include <pipewire/filter.h>
#include <spa/pod/parser.h>

#include <cstddef>
#include <memory>
#include <vector>

#include <pipewire/pipewire.h>
#include <pwcpp/property/parameters_property.h>

namespace pwcpp::filter {
using FilterPortPtr = std::shared_ptr<FilterPort>;

template <typename T>
using signal_processor = std::function<void(spa_io_position *position,
                                            std::vector<FilterPortPtr> &
                                            input_ports,
                                            std::vector<FilterPortPtr> &
                                            output_ports,
                                            property::ParametersProperty &
                                            parameters, T &user_data)>;

template <typename TData>
class App {
public:
  std::vector<FilterPortPtr> in_ports;
  std::vector<FilterPortPtr> out_ports;
  pw_main_loop *loop = nullptr;
  pw_filter *filter = nullptr;
  filter::signal_processor<TData> signal_processor;
  TData user_data;
  std::shared_ptr<property::ParametersProperty> parameters_property = nullptr;

  size_t number_of_in_ports() const { return in_ports.size(); }
  size_t number_of_out_ports() const { return out_ports.size(); }

  void run(const pw_filter_flags flags) {
    if (pw_filter_connect(filter, flags, nullptr, 0) < 0) {
      fprintf(stderr, "can't connect\n");
      return;
    }

    execute();
  }

  void run() {
    if (pw_filter_connect(filter, PW_FILTER_FLAG_RT_PROCESS, nullptr, 0) < 0) {
      fprintf(stderr, "can't connect\n");
      return;
    }

    execute();
  }

  void quit_main_loop() const { pw_main_loop_quit(loop); }

  void process(spa_io_position *position) {
    signal_processor(position, in_ports, out_ports, *parameters_property,
                     user_data);
  }

private:
  void execute() const {
    pw_main_loop_run(loop);
    pw_filter_destroy(filter);
    pw_main_loop_destroy(loop);
    pw_deinit();
  }
};
} // namespace pwcpp::filter
