#include <Arduino.h>
#include <test_includes.h>
#include <TestableDataLinkLayer.h>
#include <unity.h>

void setUp(void) {}
void tearDown(void) {}

void test_data_link_layer_calculate_checksum(void) {
  FakeUartInterface fakeUartInterface;
  TestableDataLinkLayer dataLinkLayer(&fakeUartInterface);
  
  uint8_t *data = new uint8_t[2] { 0, 0 };
  TEST_ASSERT_EQUAL(0, dataLinkLayer.call_calculate_checksum(data, 2));
  delete[] data;

  data = new uint8_t[2] { 0, 1 };
  TEST_ASSERT_EQUAL(1, dataLinkLayer.call_calculate_checksum(data, 2));
  delete[] data;
  data = new uint8_t[2] { 1, 0 };
  TEST_ASSERT_EQUAL(1, dataLinkLayer.call_calculate_checksum(data, 2));
  delete[] data;
  
  data = new uint8_t[2] { 1, 1 };
  TEST_ASSERT_EQUAL(2, dataLinkLayer.call_calculate_checksum(data, 2));
  delete[] data;
  
  data = new uint8_t[2] { 0, 255 };
  TEST_ASSERT_EQUAL(255, dataLinkLayer.call_calculate_checksum(data, 2));
  delete[] data;
  data = new uint8_t[2] { 1, 255 };
  TEST_ASSERT_EQUAL(0, dataLinkLayer.call_calculate_checksum(data, 2));
  delete[] data;
  data = new uint8_t[2] { 255, 1 };
  TEST_ASSERT_EQUAL(0, dataLinkLayer.call_calculate_checksum(data, 2));
  delete[] data;
  data = new uint8_t[2] { 255, 2 };
  TEST_ASSERT_EQUAL(1, dataLinkLayer.call_calculate_checksum(data, 2));
  delete[] data;

  data = new uint8_t[5] { 8, 128, 200, 0, 12 };
  TEST_ASSERT_EQUAL(92, dataLinkLayer.call_calculate_checksum(data, 5));
  delete[] data;
}

int runUnityTests(void) {
  UNITY_BEGIN();
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
