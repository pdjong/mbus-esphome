#include <Arduino.h>
#include <test_includes.h>
#include <TestableKamstrup303WA02.h>
#include <unity.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

using namespace esphome::warmtemetermbus;

void setUp(void) {}
void tearDown(void) {}

void test_datablockreader_read_data_blocks_from_long_frame(void) {
}

int runUnityTests(void) {
  UNITY_BEGIN();
  RUN_TEST(test_datablockreader_read_data_blocks_from_long_frame);
  return UNITY_END();
}

void setup() {
  // Wait 2 seconds before the Unity test runner
  // establishes connection with a board Serial interface
  delay(2000);

  runUnityTests();
}

void loop() {}
