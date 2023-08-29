#include <Arduino.h>
#include <test_includes.h>
#include <TestableDataLinkLayer.h>
#include <unity.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

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

void fake_uart_interface_task(void* param) {
  FakeUartInterface *uartInterface = reinterpret_cast<FakeUartInterface*>(param);
  while (!uartInterface->is_write_array_called()) {
    vTaskDelay(1 / portTICK_PERIOD_MS);
  }
  uint8_t *fake_data = new uint8_t[1] { 0xE5 };
  uartInterface->set_fake_data_to_return(fake_data, 1);
  vTaskDelete(NULL);
}

void test_data_link_layer_try_send_short_frame(void) {
  // After the request is sent, the fake slave responds after the minimum time.
  // What can be checked: 
  //  - return value (should be true)
  //  - sent data (should contain a full Short Frame: start, C, A, check sum, stop)
  //
  // For this to work, the following scenario is required:
  //  1. In a separate task, the fake uart interface is configured.
  //     It should do a delay of just over 11 bits (11 bits * 1000 ms/s / 2400 bit/s = 4.58ms)
  //     After that delay it should prepare some return data (single byte should be okay)
  //     and set the return value of available() to > 0.
  //  2. In another task, the testable DataLinkLayer can be created with the fake uart interface.
  //     Then it can call call_try_send_short_frame(), and remember the return value;
  //     When that is done, it can set a flag to indicate that the test can do its asserts.
  //  3. Directly in the test function, these two tasks are created.
  //     Then it waits for the flag. After that, it can assert everything is okay.

  FakeUartInterface uartInterface;

  xTaskCreatePinnedToCore(fake_uart_interface_task,
                    "fake_uart_interface_task", // name
                    20000,                      // stack size (in words)
                    &uartInterface,             // input params
                    1,                          // priority
                    nullptr,                    // Handle, not needed
                    0                           // core
  );

  TestableDataLinkLayer dataLinkLayer(&uartInterface);

  const uint8_t c = 0x40;
  const uint8_t a = 0x54;
  const bool actual_return_value = dataLinkLayer.call_try_send_short_frame(c, a);

  // Assert
  TEST_ASSERT_TRUE(actual_return_value);
}

int runUnityTests(void) {
  UNITY_BEGIN();
  RUN_TEST(test_data_link_layer_calculate_checksum);
  RUN_TEST(test_data_link_layer_try_send_short_frame);
  return UNITY_END();
}

void setup() {
  // Wait 2 seconds before the Unity test runner
  // establishes connection with a board Serial interface
  delay(2000);

  runUnityTests();
}

void loop() {}
