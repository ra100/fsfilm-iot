#include <unity.h>
#include "../src/effects.h"

// Unity framework requires these functions
void setUp(void)
{
  // Empty setup function
}

void tearDown(void)
{
  // Empty teardown function
}

void test_interpolateColor_midpoint()
{
  CRGB a(0, 0, 0);
  CRGB b(255, 255, 255);
  CRGB mid = interpolateColor(a, b, 0.5f);
  TEST_ASSERT_EQUAL_UINT8(127, mid.r);
  TEST_ASSERT_EQUAL_UINT8(127, mid.g);
  TEST_ASSERT_EQUAL_UINT8(127, mid.b);
}

void test_getLEDPosition_and_distance()
{
  float x, y;
  int numLeds = 100;
  float radius = 10.0f;
  bool result = getLEDPosition(0, numLeds, radius, x, y);
  TEST_ASSERT_TRUE(result);
  // LED 0 should be at angle 0 -> x == radius, y == 0
  TEST_ASSERT_FLOAT_WITHIN(0.0001f, radius, x);
  TEST_ASSERT_FLOAT_WITHIN(0.0001f, 0.0f, y);
  float d = getCircleDistance(x, y);
  TEST_ASSERT_FLOAT_WITHIN(0.0001f, radius, d);

  // Test invalid parameters
  TEST_ASSERT_FALSE(getLEDPosition(-1, numLeds, radius, x, y));
  TEST_ASSERT_FALSE(getLEDPosition(0, 0, radius, x, y));
  TEST_ASSERT_FALSE(getLEDPosition(0, numLeds, -1.0f, x, y));
  TEST_ASSERT_FALSE(getLEDPosition(numLeds, numLeds, radius, x, y));
}

int main()
{
  UNITY_BEGIN();
  RUN_TEST(test_interpolateColor_midpoint);
  RUN_TEST(test_getLEDPosition_and_distance);
  return UNITY_END();
}
