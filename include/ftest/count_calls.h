#pragma once

#include <tuple>
#include <vector>

namespace ftest {

template <class... Args> class CountCalls {
public:
  void operator()(Args... args) {
    call_arguments.push_back(std::make_tuple(args...));
  }

  std::vector<std::tuple<Args...>> call_arguments;
};

} // namespace ftest
