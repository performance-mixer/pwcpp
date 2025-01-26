#pragma once

#include "pwcpp/filter/filter_port.h"
#include "pwcpp/filter/parameter_collection.h"
#include "pwcpp/filter/property_def.h"

#include <functional>
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

/*! \brief A pointer to a property definition base. */
template <typename TApp>
using PropertyDefPtr = std::shared_ptr<PropertyDefBase<TApp>>;

/*! \brief A pointer to a filter port. */
using FilterPortPtr = std::shared_ptr<FilterPort>;

/*! \brief A signal processor.
 *
 * The signal processor processes the input ports and writes the result to the
 * output ports. The user data is passed to the signal processor and can be used
 * to store state.
 * Additionally, the signal processor can access the parameters of the filter.
 *
 * \tparam T The type of the user data.
 */
template <typename T>
using signal_processor =
    std::function<void(struct spa_io_position *position,
                       std::vector<FilterPortPtr> &input_ports,
                       std::vector<FilterPortPtr> &output_ports, T &user_data,
                       std::vector<Parameter> &parameters)>;

/*! \brief A pipewire filter
 *
 * The filter::App encapsulates a pipewire filter. It keeps all filter related
 * pipewire objects and provides the callbacks to process the input and outputs
 * and to handle property and parameter updates.
 * Filters are configured and instantiated using the filter::AppBuilder.
 *
 * \tparam TData The type of the user data.
 *
 */
template <typename TData> class App {
public:
  using property_update_function =
      std::function<void(PropertyDefBase<App<TData>> &property,
                         std::string value, App<TData> &app)>;
  std::vector<FilterPortPtr> in_ports;
  std::vector<FilterPortPtr> out_ports;
  struct pw_main_loop *loop;
  struct pw_filter *filter;
  pwcpp::filter::signal_processor<TData> signal_processor;
  TData user_data;
  std::vector<PropertyDefPtr<App<TData>>> properties;
  App::property_update_function update_property;
  ParameterCollection parameter_collection;

  /*! \brief Construct a filter app.
   *
   * The filter app is constructed empty and needs to be prepared before use.
   * The constructor should not be called directly, instead the
   * filter::AppBuilder should be used to create a filter app.
   */
  App()
      : update_property([](PropertyDefBase<App<TData>> &property,
                           std::string value, App<TData> &app) {
          struct spa_dict_item items[1];
          items[0] = SPA_DICT_ITEM_INIT(property.name.c_str(), value.c_str());
          struct spa_dict update_dict = SPA_DICT_INIT(items, 1);
          pw_filter_update_properties(app.filter, nullptr, &update_dict);
        }) {}

  /*! \brief Get the number of input ports.
   *
   * \return The number of input ports.
   */
  size_t number_of_in_ports() { return in_ports.size(); }

  /*! \brief Get the number of output ports.
   *
   * \return The number of output ports.
   */
  size_t number_of_out_ports() { return out_ports.size(); }

  /*! \brief Run the filter.
   *
   * Runs the filter, i.e. the pipewire main loop, and blocks until the filter
   * is stopped. The filter app builder registers the signals SIGINT and SIGTERM
   * to stop the filter, so the filter can be stopped using Ctrl+C.
   */
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

  /*! \brief Quit the main loop.
   *
   * Quits the main loop of the filter. This is usually called from the signal
   * handler for SIGINT and SIGTERM.
   */
  void quit_main_loop() { pw_main_loop_quit(loop); }

  /*! \brief Process the input ports.
   *
   * Processes the input ports and writes the result to the output ports. The
   * method is called by the pipewire process callback. The pipewire process is
   * called in a real-time context and care should be taken to avoid memory
   * allocations and other blocking operations.
   *
   * \param position The position of the input ports.
   */
  void process(struct spa_io_position *position) {
    signal_processor(position, in_ports, out_ports, user_data,
                     parameter_collection.parameters);
  }

  /*! \brief Handle a property update.
   *
   * Handles a property update. The method is called by the pipewire property
   * callback and handles property and parameter updates.
   *
   * \param obj The property object.
   */
  void handle_property_update(const spa_pod_object *obj) {
    struct spa_pod_prop *prop;
    SPA_POD_OBJECT_FOREACH(obj, prop) {
      auto property_it =
          std::find_if(properties.begin(), properties.end(),
                       [&prop](auto &&p) { return p->key == prop->key; });

      if (property_it != properties.end()) {
        auto result =
            (*property_it)->handle_property_update(&prop->value, *this);
        if (result.has_value()) {
          update_property(**property_it, result.value(), *this);
        }
      }
    }
  }
};

} // namespace pwcpp::filter
