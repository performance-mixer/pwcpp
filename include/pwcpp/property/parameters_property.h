#pragma once

#include "pwcpp/property/property.h"

#include <string>
#include <tuple>
#include <vector>

namespace pwcpp::property {
class ParametersProperty : public Property {
public:
  explicit ParametersProperty(
    std::shared_ptr<std::vector<std::tuple<std::string, property_value_type>>>
    parameters) : Property(SPA_PROP_params) {
    _parameters = parameters;
  }

  ~ParametersProperty() override = default;

  std::expected<void, error>
  add_to_pod_object(spa_pod_builder *builder) override {
    if (!_parameters->empty()) {
      spa_pod_builder_prop(builder, _key, 0);
      spa_pod_frame frame{};
      spa_pod_builder_push_struct(builder, &frame);
      for (auto &parameter : *_parameters) {
        spa_pod_builder_string(builder, std::get<0>(parameter).c_str());
        if (auto result = write_property_value(builder, std::get<1>(parameter));
          !result.has_value()) {
          return std::unexpected(result.error());
        }
      }
      spa_pod_builder_pop(builder, &frame);
    }

    return {};
  };

  template <typename T>
  std::expected<void, error> update(std::string name, T &value) {
    const auto existing_param = std::find_if(_parameters->begin(),
                                             _parameters->end(),
                                             [&name](auto &param) {
                                               return std::get<0>(param) ==
                                                 name;
                                             });

    if (existing_param == _parameters->end()) {
      return std::unexpected(error::parameter_not_found(name));
    }

    get<1>(*existing_param) = value;
    return {};
  }

  std::expected<void, error>
  update_from_pod(const spa_pod *pod) {
    auto updates = parse(pod);
    if (!updates.has_value()) {
      return std::unexpected(updates.error());
    }

    for (auto &update : updates.value()) {
      auto existing_param = std::find_if(_parameters->begin(),
                                         _parameters->end(),
                                         [&update](auto &param) {
                                           return std::get<0>(param) == std::get
                                             <0>(update);
                                         });
      if (existing_param != _parameters->end()) {
        std::get<1>(*existing_param) = std::get<1>(update);
      } else {
        _parameters->emplace_back(update);
      }
    }

    return {};
  }

  std::span<const std::tuple<std::string, property_value_type>>
  parameters() const {
    return *_parameters;
  }

private:
  std::shared_ptr<std::vector<std::tuple<std::string, property_value_type>>>
  _parameters{};

  static std::expected<std::vector<std::tuple<std::string, property_value_type>>
                       , error>
  parse(const spa_pod *pod) {
    std::size_t count(0);
    std::vector<std::string> keys;
    std::vector<property_value_type> values;
    void *struct_field_void;
    SPA_POD_STRUCT_FOREACH(pod, struct_field_void) {
      auto struct_field = reinterpret_cast<spa_pod*>(struct_field_void);
      if (count % 2 == 0) {
        const char *key;
        spa_pod_get_string(struct_field, &key);
        keys.emplace_back(key);
      } else {
        auto type = SPA_POD_TYPE(struct_field);
        switch (type) {
        case SPA_TYPE_Int:
          int int_value;
          spa_pod_get_int(struct_field, &int_value);
          values.emplace_back(int_value);
          break;
        case SPA_TYPE_Long:
          long long_value;
          spa_pod_get_long(struct_field, &long_value);
          values.emplace_back(long_value);
          break;
        case SPA_TYPE_Float:
          float float_value;
          spa_pod_get_float(struct_field, &float_value);
          values.emplace_back(float_value);
          break;
        case SPA_TYPE_Double:
          double double_value;
          spa_pod_get_double(struct_field, &double_value);
          values.emplace_back(double_value);
          break;
        case SPA_TYPE_String:
          {
            const char *string_value;
            spa_pod_get_string(struct_field, &string_value);
            std::string value(string_value);
            if (value == "null") {
              values.emplace_back(std::nullopt);
            } else {
              values.emplace_back(value);
            }
            break;
          }
        default: keys.pop_back();
        }
      }

      count++;
    }

    std::vector<std::tuple<std::string, property_value_type>> result;
    for (std::size_t i = 0; i < keys.size() && i < values.size(); ++i) {
      result.emplace_back(keys[i], values[i]);
    }

    return result;
  }
};
}
