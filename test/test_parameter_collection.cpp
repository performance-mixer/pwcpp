#include "pwcpp/filter/parameter_collection.h"
#include <microtest/microtest.h>
#include <spa/control/control.h>
#include <spa/debug/pod.h>
#include <spa/pod/builder.h>
#include <spa/pod/iter.h>
#include <spa/pod/parser.h>
#include <spa/pod/pod.h>
#include <spa/utils/dict.h>
#include <string>

TEST(ParseParametersMessage) {
  const std::size_t buffer_size = 4096;
  char *buffer[buffer_size];

  struct spa_pod_builder builder;
  spa_pod_builder_init(&builder, buffer, buffer_size);

  struct spa_pod_frame struct_frame;
  spa_pod_builder_push_struct(&builder, &struct_frame);
  spa_pod_builder_string(&builder, "hello");
  spa_pod_builder_string(&builder, "world");

  spa_pod_builder_string(&builder, "buddy");
  spa_pod_builder_int(&builder, 42);

  spa_pod_builder_string(&builder, "order");
  spa_pod_builder_double(&builder, 66.0);

  spa_pod_builder_string(&builder, "length");
  spa_pod_builder_float(&builder, 12.5);

  auto pod =
      static_cast<spa_pod *>(spa_pod_builder_pop(&builder, &struct_frame));

  auto result = pwcpp::filter::ParameterCollection::parse(pod);

  ASSERT_EQ(result->size(), 4);

  ASSERT_STREQ(get<0>(result->at(0)), "hello");
  ASSERT_STREQ(get<0>(result->at(1)), "buddy");
  ASSERT_STREQ(get<0>(result->at(2)), "order");
  ASSERT_STREQ(get<0>(result->at(3)), "length");

  ASSERT_TRUE(std::holds_alternative<std::string>(get<1>(result->at(0))));
  ASSERT_TRUE(std::holds_alternative<int>(get<1>(result->at(1))));
  ASSERT_TRUE(std::holds_alternative<double>(get<1>(result->at(2))));
  ASSERT_TRUE(std::holds_alternative<float>(get<1>(result->at(3))));
}

TEST(HandleParameterUpdates) {
  pwcpp::filter::ParameterCollection parameter_collection;
  parameter_collection.parameters.push_back(
      pwcpp::filter::Parameter("hello", 42));
  parameter_collection.parameters.push_back(
      pwcpp::filter::Parameter("world", 66.0));
  parameter_collection.parameters.push_back(
      pwcpp::filter::Parameter("buddy", "hello"));
  parameter_collection.parameters.push_back(
      pwcpp::filter::Parameter("order", static_cast<float>(12.5)));

  std::vector<std::tuple<std::string, pwcpp::filter::variant_type>> updates{
      std::make_tuple("hello", pwcpp::filter::variant_type(12)),
  };

  auto result = parameter_collection.handle_parameter_updates(updates);
  ASSERT_TRUE(result);

  ASSERT_EQ(get<int>(parameter_collection.parameters[0].value), 12);
  ASSERT_EQ(get<double>(parameter_collection.parameters[1].value), 66.0);
  ASSERT_EQ(get<std::string>(parameter_collection.parameters[2].value),
            "hello");
  ASSERT_EQ(get<float>(parameter_collection.parameters[3].value), 12.5);
}

TEST(ToDisplay) {
  pwcpp::filter::ParameterCollection parameter_collection;
  parameter_collection.parameters.push_back(
      pwcpp::filter::Parameter("hello", 42));
  parameter_collection.parameters.push_back(
      pwcpp::filter::Parameter("world", 66.5));
  parameter_collection.parameters.push_back(
      pwcpp::filter::Parameter("buddy", "hello"));
  parameter_collection.parameters.push_back(
      pwcpp::filter::Parameter("order", static_cast<float>(12.5)));

  auto result = parameter_collection.to_display();

  ASSERT_STREQ(result,
               "{ hello = 42, world = 66.5, buddy = hello, order = 12.5 }");
}

TEST_MAIN()
