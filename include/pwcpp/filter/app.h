#pragma once

#include "pwcpp/filter/filter_port.h"
#include "pwcpp/filter/parameter_collection.h"
#include "pwcpp/filter/property_def.h"

#include <functional>
#include <pipewire/filter.h>
#include <spa/pod/parser.h>
#include <spa/pod/pod.h>

#include <cstddef>
#include <expected>
#include <memory>
#include <vector>

#include <pipewire/pipewire.h>

namespace pwcpp::filter {
template <typename TApp>
using PropertyDefPtr = std::shared_ptr<PropertyDefBase<TApp>>;

using FilterPortPtr = std::shared_ptr<FilterPort>;

template <typename T>
using signal_processor = std::function<void(spa_io_position *position,
                                            std::vector<FilterPortPtr> &
                                            input_ports,
                                            std::vector<FilterPortPtr> &
                                            output_ports, T &user_data,
                                            std::vector<Parameter> &parameters)>
;

template <typename TData>
class App {
public:
  using property_update_function = std::function<void(
    PropertyDefBase<App> &property, std::string value, App &app)>;
  std::vector<FilterPortPtr> in_ports;
  std::vector<FilterPortPtr> out_ports;
  pw_main_loop *loop = nullptr;
  pw_filter *filter = nullptr;
  filter::signal_processor<TData> signal_processor;
  TData user_data;
  std::vector<PropertyDefPtr<App<TData>>> properties;
  property_update_function update_property;
  ParameterCollection parameter_collection;

  App()
    : update_property([](PropertyDefBase<App> &property,
                         std::string value, App &app) {
      spa_dict_item items[1];
      items[0] = SPA_DICT_ITEM_INIT(property.name.c_str(), value.c_str());
      auto update_dict = SPA_DICT_INIT(items, 1);
      pw_filter_update_properties(app.filter, nullptr, &update_dict);
    }) {}

  size_t number_of_in_ports() { return in_ports.size(); }
  size_t number_of_out_ports() { return out_ports.size(); }

  void run(pw_filter_flags flags) {
    if (pw_filter_connect(filter, flags, NULL, 0) < 0) {
      fprintf(stderr, "can't connect\n");
      return;
    }

    execute();
  }

  void run() {
    if (pw_filter_connect(filter, PW_FILTER_FLAG_RT_PROCESS, NULL, 0) < 0) {
      fprintf(stderr, "can't connect\n");
      return;
    }

    execute();
  }

  void quit_main_loop() { pw_main_loop_quit(loop); }

  void process(spa_io_position *position) {
    signal_processor(position, in_ports, out_ports, user_data,
                     parameter_collection.parameters);
  }

  void handle_property_update(const spa_pod_object *obj) {
    spa_pod_prop *prop;
    SPA_POD_OBJECT_FOREACH(obj, prop) {
      auto property_it = std::find_if(properties.begin(), properties.end(),
                                      [&prop](auto &&p) {
                                        return p->key == prop->key;
                                      });

      if (property_it != properties.end()) {
        auto result = (*property_it)->handle_property_update(
          &prop->value, *this);
        if (result.has_value()) {
          update_property(**property_it, result.value(), *this);
        }
      }
    }
  }

private:
  void execute() {
    pw_main_loop_run(loop);
    pw_filter_destroy(filter);
    pw_main_loop_destroy(loop);
    pw_deinit();
  }
};
} // namespace pwcpp::filter
