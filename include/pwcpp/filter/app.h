#pragma once

#include "pwcpp/filter/filter_port.h"
#include "pwcpp/filter/property_def.h"

#include <pipewire/filter.h>
#include <spa/param/props.h>
#include <spa/pod/parser.h>
#include <spa/pod/pod.h>

#include <cstddef>
#include <expected>
#include <memory>
#include <vector>

#include <pipewire/loop.h>
#include <pipewire/pipewire.h>

namespace pwcpp::filter {

template <typename TApp>
using PropertyDefPtr = std::shared_ptr<PropertyDefBase<TApp>>;

using FilterPortPtr = std::shared_ptr<FilterPort>;

template <typename T>
using signal_processor =
    std::function<void(struct spa_io_position *position,
                       std::vector<FilterPortPtr> &input_ports,
                       std::vector<FilterPortPtr> &output_ports, T &user_data)>;

template <typename TData> class App {
public:
  std::vector<FilterPortPtr> in_ports;
  std::vector<FilterPortPtr> out_ports;
  struct pw_main_loop *loop;
  struct pw_filter *filter;
  pwcpp::filter::signal_processor<TData> signal_processor;
  TData user_data;
  std::vector<PropertyDefPtr<App<TData>>> properties;

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
    signal_processor(position, in_ports, out_ports, user_data);
  }

  void handle_property_update(struct spa_pod_object *obj) {
    struct spa_pod_prop *prop;
    SPA_POD_OBJECT_FOREACH(obj, prop) {
      auto property_it =
          std::find_if(properties.begin(), properties.end(),
                       [&prop](auto &&p) { return p->key == prop->key; });

      if (property_it != properties.end()) {
        (*property_it)->handle_property_update(&prop->value, *this);
      }
    }
  }
};

} // namespace pwcpp::filter
