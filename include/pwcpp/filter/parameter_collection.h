#pragma once

#include "pwcpp/error.h"
#include "pwcpp/filter/parameter.h"

#include <cstddef>
#include <expected>
#include <span>
#include <sstream>
#include <string>
#include <vector>

#include <spa/control/control.h>
#include <spa/debug/pod.h>
#include <spa/pod/builder.h>
#include <spa/pod/iter.h>
#include <spa/pod/parser.h>
#include <spa/pod/pod.h>
#include <spa/utils/dict.h>

namespace pwcpp::filter {

class ParameterCollection {
public:
  static std::expected<std::vector<std::tuple<std::string, variant_type>>,
                       error>
  parse(spa_pod *pod) {
    std::size_t count(0);
    std::vector<std::string> keys;
    std::vector<variant_type> values;
    void *struct_field_void;
    SPA_POD_STRUCT_FOREACH(pod, struct_field_void) {
      auto struct_field = reinterpret_cast<spa_pod *>(struct_field_void);
      if (count % 2 == 0) {
        const char *key;
        spa_pod_get_string(struct_field, &key);
        keys.push_back(std::string(key));
      } else {
        auto type = SPA_POD_TYPE(struct_field);
        switch (type) {
        case SPA_TYPE_Int:
          int int_value;
          spa_pod_get_int(struct_field, &int_value);
          values.push_back(variant_type(int_value));
          break;
        case SPA_TYPE_Float:
          float float_value;
          spa_pod_get_float(struct_field, &float_value);
          values.push_back(variant_type(float_value));
          break;
        case SPA_TYPE_Double:
          double double_value;
          spa_pod_get_double(struct_field, &double_value);
          values.push_back(variant_type(double_value));
          break;
        case SPA_TYPE_String:
          const char *string_value;
          spa_pod_get_string(struct_field, &string_value);
          values.push_back(variant_type(std::string(string_value)));
          break;
        }
      }

      count++;
    }

    std::vector<std::tuple<std::string, variant_type>> result;
    for (std::size_t i = 0; i < keys.size() && i < values.size(); ++i) {
      result.emplace_back(keys[i], values[i]);
    }

    return result;
  }

  bool handle_parameter_updates(
      std::span<std::tuple<std::string, variant_type>> updates) {
    for (auto &&update : updates) {
      auto parameter = std::ranges::find_if(parameters, [&](auto &&parameter) {
        return parameter.key == std::get<0>(update);
      });

      if (parameter == parameters.end()) {
        continue;
      }

      parameter->value = std::get<1>(update);
    }
    return true;
  }

  std::string to_display() {
    std::ostringstream os;
    os << "{ ";
    for (auto &&parameter : parameters) {
      if (std::holds_alternative<std::string>(parameter.value)) {
        os << parameter.key << " = " << get<std::string>(parameter.value)
           << ", ";
      } else if (std::holds_alternative<int>(parameter.value)) {
        os << parameter.key << " = " << get<int>(parameter.value) << ", ";
      } else if (std::holds_alternative<float>(parameter.value)) {
        os << parameter.key << " = " << get<float>(parameter.value) << ", ";
      } else if (std::holds_alternative<double>(parameter.value)) {
        os << parameter.key << " = " << get<double>(parameter.value) << ", ";
      }
    }
    os.seekp(-2, std::ios_base::end);
    os << " }";
    return os.str();
  }

  std::vector<Parameter> parameters;
};

} // namespace pwcpp::filter
