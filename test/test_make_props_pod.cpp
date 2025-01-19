#include <microtest/microtest.h>

#include <pwcpp/spa/pod/make_props_pod.h>

#include <spa/pod/parser.h>

TEST(CreateIntParamPod) {
  u_int8_t buffer[4096];
  auto pod = pwcpp::spa::pod::make_props_pod(buffer, 4096, 0x1000042, 42);
  ASSERT_TRUE(pod.has_value());

  auto obj = reinterpret_cast<struct spa_pod_object *>(pod.value());
  struct spa_pod_prop *prop;
  unsigned int iterations(0);
  SPA_POD_OBJECT_FOREACH(obj, prop) {
    int32_t value;
    spa_pod_get_int(&prop->value, &value);
    ASSERT_EQ(value, 42);
    ASSERT_EQ(prop->key, 0x1000042);
    iterations++;
  }
  ASSERT_EQ(iterations, 1);
}

TEST_MAIN()
