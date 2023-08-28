#include <Arduino.h>
#include <test_includes.h>
#include <bollocks.h>
#include <TestableDataLinkLayer.h>
#include <unity.h>

void setUp(void) {}
void tearDown(void) {}

void test_bollocks_do_something_with_two_numbers(void) {
  Bollocks bollocks;
  TEST_ASSERT_EQUAL(0, bollocks.do_something_with_two_numbers(0, 0));
  
  TEST_ASSERT_EQUAL(2, bollocks.do_something_with_two_numbers(1, 0));
  TEST_ASSERT_EQUAL(4, bollocks.do_something_with_two_numbers(2, 0));

  TEST_ASSERT_EQUAL(1, bollocks.do_something_with_two_numbers(0, 1));
  TEST_ASSERT_EQUAL(2, bollocks.do_something_with_two_numbers(0, 2));

  TEST_ASSERT_EQUAL(3, bollocks.do_something_with_two_numbers(1, 1));
  TEST_ASSERT_EQUAL(6, bollocks.do_something_with_two_numbers(2, 2));
}

void test_data_link_layer_calculate_checksum(void) {
  FakeUartInterface fakeUartInterface;
  TestableDataLinkLayer dataLinkLayer(&fakeUartInterface);

  uint8_t data[] { 0, 0 };
  TEST_ASSERT_EQUAL(0, dataLinkLayer.call_calculate_checksum(0, 0));
}

int runUnityTests(void) {
  UNITY_BEGIN();
  RUN_TEST(test_bollocks_do_something_with_two_numbers);
  RUN_TEST(test_data_link_layer_calculate_checksum);
  return UNITY_END();
}

void setup() {
  // Wait 2 seconds before the Unity test runner
  // establishes connection with a board Serial interface
  delay(2000);

  runUnityTests();
}

void loop() {}
