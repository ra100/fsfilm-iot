#include <unity.h>
#include "../src/debounce.h"

// Unity framework requires these functions
void setUp(void)
{
  // Empty setup function
}

void tearDown(void)
{
  // Empty teardown function
}

void test_debounce_stable_change()
{
  Debounce db(50);
  unsigned long t = 0;
  // initial reading HIGH
  TEST_ASSERT_FALSE(db.sample(HIGH, t));
  // bounce to LOW briefly
  t += 10;
  TEST_ASSERT_FALSE(db.sample(LOW, t));
  // back to HIGH and enough time passes -> no change
  t += 40;
  TEST_ASSERT_FALSE(db.sample(HIGH, t));
  // remain HIGH long enough to settle
  t += 100;
  TEST_ASSERT_FALSE(db.sample(HIGH, t));
  TEST_ASSERT_EQUAL(HIGH, db.getState());

  // now change to LOW and hold
  t += 10;
  TEST_ASSERT_FALSE(db.sample(LOW, t));
  t += 60;
  // now it should report a stable change
  TEST_ASSERT_TRUE(db.sample(LOW, t));
  TEST_ASSERT_EQUAL(LOW, db.getState());
}

int main()
{
  UNITY_BEGIN();
  RUN_TEST(test_debounce_stable_change);
  return UNITY_END();
}
