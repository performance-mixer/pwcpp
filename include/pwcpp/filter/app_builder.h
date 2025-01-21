#pragma once

#include "pwcpp/error.h"
#include "pwcpp/filter/app.h"
#include "pwcpp/filter/filter_port.h"

#include <algorithm>
#include <expected>
#include <functional>
#include <iterator>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include <pipewire/filter.h>
#include <pipewire/loop.h>
#include <pipewire/pipewire.h>
#include <pipewire/port.h>
#include <pipewire/properties.h>

#include <spa/node/io.h>

namespace pwcpp::filter {

struct port_def {
  std::string name;
  std::string dsp_format;
};

template <typename TData> class AppBuilder {
public:
  using FilterAppPtr = std::shared_ptr<App<TData>>;
  using PipewireInitialization = std::function<void(int, char *[])>;
  using PortBuilder =
      std::function<port *(std::string, std::string, struct pw_filter *)>;
  using FilterAppBuilder =
      std::function<std::tuple<struct pw_main_loop *, struct pw_filter *>(
          std::string, std::string, std::string,
          std::vector<PropertyDefPtr<App<TData>>> &, FilterAppPtr)>;

  AppBuilder()
      : pipewire_initialization(
            [](int argc, char *argv[]) { pw_init(&argc, &argv); }),
        filter_app_builder([](auto name, auto media_type, auto media_class,
                              auto properties, auto filter_app) {
          auto loop = pw_main_loop_new(NULL);

          pw_loop_add_signal(
              pw_main_loop_get_loop(loop), SIGINT,
              [](void *user_data, int signal_number) {
                auto filter_app = static_cast<App<TData> *>(user_data);
                filter_app->quit_main_loop();
              },
              filter_app.get());

          pw_loop_add_signal(
              pw_main_loop_get_loop(loop), SIGTERM,
              [](void *user_data, int signal_number) {
                auto filter_app = static_cast<App<TData> *>(user_data);
                filter_app->quit_main_loop();
              },
              filter_app.get());

          auto initial_properties = pw_properties_new(
              PW_KEY_MEDIA_TYPE, media_type.c_str(), PW_KEY_MEDIA_CATEGORY,
              "Filter", PW_KEY_MEDIA_CLASS, media_class.c_str(),
              PW_KEY_MEDIA_ROLE, "DSP", NULL);

          pw_filter *filter = nullptr;
          if (!properties.empty()) {
            auto additional_properties_dict_items =
                std::make_unique<spa_dict_item[]>(properties.size());

            size_t i = 0;
            for (auto &&property : properties) {
              additional_properties_dict_items[i] = SPA_DICT_ITEM_INIT(
                  property->name.c_str(), property->init.c_str());
              i++;
            }

            struct spa_dict additional_properties_dict = SPA_DICT_INIT(
                additional_properties_dict_items.get(), properties.size());

            pw_properties_add(initial_properties, &additional_properties_dict);

            constexpr static const pw_filter_events filter_events = {
                .version = PW_VERSION_FILTER_EVENTS,
                .param_changed =
                    [](void *user_data, void *port_data, uint32_t parameter_id,
                       const struct spa_pod *pod) {
                      auto filter_app = static_cast<App<TData> *>(user_data);
                      filter_app->handle_property_update(
                          reinterpret_cast<const spa_pod_object *>(pod));
                    },
                .process =
                    [](void *user_data, struct spa_io_position *position) {
                      auto filter_app = static_cast<App<TData> *>(user_data);
                      filter_app->process(position);
                    }};

            filter = pw_filter_new_simple(pw_main_loop_get_loop(loop),
                                          name.c_str(), initial_properties,
                                          &filter_events, filter_app.get());
          } else {
            constexpr static const pw_filter_events filter_events = {
                .version = PW_VERSION_FILTER_EVENTS,
                .process = [](void *user_data,
                              struct spa_io_position *position) {
                  auto filter_app = static_cast<App<TData> *>(user_data);
                  filter_app->process(position);
                }};

            filter = pw_filter_new_simple(pw_main_loop_get_loop(loop),
                                          name.c_str(), initial_properties,
                                          &filter_events, filter_app.get());
          }

          return std::make_tuple(loop, filter);
        }),
        in_port_builder([](auto name, auto dsp_format, auto filter) {
          return static_cast<struct port *>(pw_filter_add_port(
              filter, PW_DIRECTION_INPUT, PW_FILTER_PORT_FLAG_MAP_BUFFERS,
              sizeof(struct port),
              pw_properties_new(PW_KEY_FORMAT_DSP, dsp_format.c_str(),
                                PW_KEY_PORT_NAME, name.c_str(), NULL),
              NULL, 0));
        }),
        out_port_builder([](auto name, auto dsp_format, auto filter) {
          return static_cast<struct port *>(pw_filter_add_port(
              filter, PW_DIRECTION_OUTPUT, PW_FILTER_PORT_FLAG_MAP_BUFFERS,
              sizeof(struct port),
              pw_properties_new(PW_KEY_FORMAT_DSP, dsp_format.c_str(),
                                PW_KEY_PORT_NAME, name.c_str(), NULL),
              NULL, 0));
        }) {}

  AppBuilder(PipewireInitialization pipewire_initialization,
             FilterAppBuilder filter_app_builder, PortBuilder in_port_builder,
             PortBuilder out_port_builder)
      : pipewire_initialization(pipewire_initialization),
        filter_app_builder(filter_app_builder),
        in_port_builder(in_port_builder), out_port_builder(out_port_builder) {}

  AppBuilder &add_input_port(std::string name, std::string dsp_format) {
    input_ports.push_back(port_def{name, dsp_format});
    return *this;
  }

  AppBuilder &add_output_port(std::string name, std::string dsp_format) {
    output_ports.push_back(port_def{name, dsp_format});
    return *this;
  }

  AppBuilder &set_filter_name(std::string name) {
    filter_name = name;
    return *this;
  }

  AppBuilder &set_media_type(std::string type) {
    media_type = type;
    return *this;
  }

  AppBuilder &set_media_class(std::string media_class) {
    this->media_class = media_class;
    return *this;
  }

  AppBuilder &add_arguments(int argc, char *argv[]) {
    this->argc = argc;
    this->argv = argv;
    return *this;
  }

  AppBuilder &add_signal_processor(
      pwcpp::filter::signal_processor<TData> signal_processor) {
    this->signal_processor = signal_processor;
    return *this;
  }

  template <typename TProp>
  AppBuilder &add_property(std::string name, std::string init, spa_prop key,
                           property_handler<TProp, TData> property_handler,
                           property_parser<TProp, TData> property_parser,
                           property_to_display<TProp> property_to_display) {
    properties.push_back(std::make_shared<PropertyDef<TProp, TData>>(
        name, init, key, property_handler, property_parser,
        property_to_display));
    return *this;
  }

  std::expected<FilterAppPtr, error> build() {
    if (filter_name.empty() || media_type.empty() || media_class.empty() ||
        !signal_processor.has_value()) {
      return std::unexpected(error::configuration());
    }

    pipewire_initialization(argc, argv);

    auto filter_app = std::make_shared<App<TData>>();

    auto pw_filter_data = filter_app_builder(
        filter_name, media_type, media_class, properties, filter_app);

    filter_app->loop = get<0>(pw_filter_data);
    filter_app->filter = get<1>(pw_filter_data);
    filter_app->signal_processor = signal_processor.value();

    std::ranges::transform(
        input_ports, std::back_inserter(filter_app->in_ports),
        [this, &pw_filter_data](auto &&port_def) {
          auto pw_port = in_port_builder(port_def.name, port_def.dsp_format,
                                         get<1>(pw_filter_data));
          return std::make_shared<FilterPort>(pw_port);
        });

    std::ranges::transform(
        output_ports, std::back_inserter(filter_app->out_ports),
        [this, &pw_filter_data](auto &&port_def) {
          auto pw_port = out_port_builder(port_def.name, port_def.dsp_format,
                                          get<1>(pw_filter_data));
          return std::make_shared<FilterPort>(pw_port);
        });

    if (properties.size() > 0) {
      filter_app->properties = properties;
    }

    return filter_app;
  };

private:
  std::vector<port_def> input_ports;
  std::vector<port_def> output_ports;
  PipewireInitialization pipewire_initialization;
  FilterAppBuilder filter_app_builder;
  PortBuilder in_port_builder;
  PortBuilder out_port_builder;
  int argc = 0;
  char **argv = nullptr;
  std::string filter_name;
  std::string media_type;
  std::string media_class;
  std::optional<pwcpp::filter::signal_processor<TData>> signal_processor;
  std::vector<PropertyDefPtr<App<TData>>> properties;
};

} // namespace pwcpp::filter
