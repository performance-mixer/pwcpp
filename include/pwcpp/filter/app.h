#pragma once

#include "pipewire/filter.h"
#include "pwcpp/filter/filter_port.h"

#include <cstddef>
#include <expected>
#include <memory>
#include <vector>

#include <pipewire/loop.h>
#include <pipewire/pipewire.h>

namespace pwcpp::filter {

using FilterPortPtr = std::shared_ptr<FilterPort>;

using signal_processor = std::function<void(
    struct spa_io_position *position, std::vector<FilterPortPtr> &input_ports,
    std::vector<FilterPortPtr> &output_ports)>;

class App {
public:
  std::vector<FilterPortPtr> in_ports;
  std::vector<FilterPortPtr> out_ports;
  struct pw_main_loop *loop;
  struct pw_filter *filter;
  pwcpp::filter::signal_processor signal_processor;

  size_t number_of_in_ports() { return in_ports.size(); }
  size_t number_of_out_ports() { return out_ports.size(); }

  void run() {
    if (pw_filter_connect(filter, PW_FILTER_FLAG_RT_PROCESS, NULL, 0) < 0) {
      fprintf(stderr, "can't connect\n");
      return;
    }

    pw_main_loop_run(loop);
    pw_filter_destroy(filter);
    pw_main_loop_destroy(loop);
    pw_deinit();
  }

  void quit_main_loop() { pw_main_loop_quit(loop); }

  void process(struct spa_io_position *position) {
    signal_processor(position, in_ports, out_ports);
  }
};

} // namespace pwcpp::filter
