#pragma once

#include "pwcpp/filter/filter_port.h"
#include "spa/pod/pod.h"

#include <pipewire/filter.h>
#include <spa/param/props.h>
#include <spa/pod/parser.h>

#include <cstddef>
#include <expected>
#include <memory>
#include <vector>

#include <pipewire/loop.h>
#include <pipewire/pipewire.h>

namespace pwcpp::filter {

template <typename TApp> class PropertyDefBase {
public:
  PropertyDefBase(std::string name, std::string init, spa_prop key)
      : name(name), init(init), key(key) {}

  std::string name;
  std::string init;
  spa_prop key;

  virtual void handle_property_update(spa_pod_parser *parser, spa_pod *pod,
                                      TApp &app) = 0;
};

template <typename T> class App;

template <typename TProp, typename TData>
using property_handler = std::function<void(TProp &, App<TData> &)>;

template <typename TProp, typename TData>
using property_parser =
    std::function<TProp(spa_pod_parser *parser, spa_pod *, App<TData> &)>;

template <typename TProp, typename TData>
class PropertyDef : public PropertyDefBase<App<TData>> {
public:
  PropertyDef(std::string name, std::string init, spa_prop key,
              property_handler<TProp, TData> property_handler,
              property_parser<TProp, TData> property_parser)
      : PropertyDefBase<App<TData>>(name, init, key),
        property_handler(property_handler), property_parser(property_parser) {}

  pwcpp::filter::property_handler<TProp, TData> property_handler;
  pwcpp::filter::property_parser<TProp, TData> property_parser;

  virtual void handle_property_update(spa_pod_parser *parser, spa_pod *pod,
                                      App<TData> &app) {
    auto property_value = property_parser(parser, pod, app);
    property_handler(property_value, app);
  }
};

template <typename TApp>
using PropertyDefPtr = std::shared_ptr<PropertyDefBase<TApp>>;

using FilterPortPtr = std::shared_ptr<FilterPort>;

template <typename T>
using signal_processor =
    std::function<void(struct spa_io_position *position,
                       std::vector<FilterPortPtr> &input_ports,
                       std::vector<FilterPortPtr> &output_ports, T user_data)>;

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
        struct spa_pod_parser parser;
        spa_pod_parser_pod(&parser, &prop->value);
        int32_t value;
        auto result = spa_pod_parser_get_int(&parser, &value);
        (*property_it)->handle_property_update(&parser, &prop->value, *this);
      }
    }
  }
};

} // namespace pwcpp::filter
