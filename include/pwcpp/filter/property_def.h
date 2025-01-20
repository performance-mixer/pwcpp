#pragma once

#include "spa/pod/pod.h"

#include <pipewire/filter.h>
#include <spa/param/props.h>
#include <spa/pod/parser.h>

#include <expected>
#include <functional>
#include <string>

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

  virtual std::string handle_property_update(spa_pod *pod, TApp &app) = 0;
};

template <typename T> class App;

template <typename TProp, typename TData>
using property_handler = std::function<void(TProp &, App<TData> &)>;

template <typename TProp, typename TData>
using property_parser = std::function<TProp(spa_pod *, App<TData> &)>;

template <typename TProp>
using property_to_display = std::function<std::string(TProp &)>;

template <typename TProp, typename TData>
class PropertyDef : public PropertyDefBase<App<TData>> {
public:
  PropertyDef(std::string name, std::string init, spa_prop key,
              property_handler<TProp, TData> property_handler,
              property_parser<TProp, TData> property_parser,
              property_to_display<TProp> property_to_display)
      : PropertyDefBase<App<TData>>(name, init, key),
        property_handler(property_handler), property_parser(property_parser),
        property_to_display(property_to_display) {}

  pwcpp::filter::property_handler<TProp, TData> property_handler;
  pwcpp::filter::property_parser<TProp, TData> property_parser;
  pwcpp::filter::property_to_display<TProp> property_to_display;

  virtual std::string handle_property_update(spa_pod *pod, App<TData> &app) {
    auto property_value = property_parser(pod, app);
    property_handler(property_value, app);
    return "";
  }
};

} // namespace pwcpp::filter
