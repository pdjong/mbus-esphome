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

typedef struct FakeUartInterfaceTaskArgs {
  FakeUartInterface* uart_interface;
  const uint8_t respond_to_nth_write;
  const uint8_t delay_in_ms;
  const uint8_t* data_to_return;
  const size_t len_of_data_to_return;
} FakeUartInterfaceTaskArgs;

void fake_uart_interface_task(void* param) {
  FakeUartInterfaceTaskArgs *args = reinterpret_cast<FakeUartInterfaceTaskArgs*>(param);
  FakeUartInterface *uartInterface = args->uart_interface;
  while (uartInterface->write_array_call_count() < args->respond_to_nth_write) {
    vTaskDelay(1 / portTICK_PERIOD_MS);
  }
  delay(args->delay_in_ms);
  //uint8_t *fake_data = new uint8_t[1] { 0xE5 };
  //uartInterface->set_fake_data_to_return(fake_data, 1);
  uartInterface->set_fake_data_to_return(args->data_to_return, args->len_of_data_to_return);
  vTaskDelete(NULL);
}

void test_data_link_layer_try_send_short_frame_reply_to_first_request(void) {
  // Arrange

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
  //  2. In the test function, the testable DataLinkLayer can be created with the fake uart interface.
  //     Then it can use call_try_send_short_frame(), and remember the return value.
  //     Then it waits for the call to finish. After that, it can assert everything is okay.

  FakeUartInterface uart_interface;
  const uint8_t fake_data[] = { 0xE5 };
  FakeUartInterfaceTaskArgs args = { 
    .uart_interface = &uart_interface, 
    .respond_to_nth_write = 1, 
    .delay_in_ms  = 5,
    .data_to_return = fake_data,
    .len_of_data_to_return = 1
  };
  xTaskCreatePinnedToCore(fake_uart_interface_task,
                    "fake_uart_interface_task", // name
                    20000,                      // stack size (in words)
                    &args,                      // input params
                    1,                          // priority
                    nullptr,                    // Handle, not needed
                    0                           // core
  );

  TestableDataLinkLayer dataLinkLayer(&uart_interface);

  // Act
  const uint8_t c = 0x40;
  const uint8_t a = 0x54;
  const bool actual_return_value = dataLinkLayer.call_try_send_short_frame(c, a);

  // Assert
  TEST_ASSERT_TRUE(actual_return_value);
  // Check the sent data: should be a short frame!
  // Start:     0x10
  // C:         0x40
  // A:         0x54
  // Check sum: 0x94
  // Stop:      0x16
  TEST_ASSERT_EQUAL(1, uart_interface.written_arrays_.size());
  FakeUartInterface::WrittenArray actual_written_array = uart_interface.written_arrays_[0];
  TEST_ASSERT_EQUAL(5, actual_written_array.len);
  TEST_ASSERT_EQUAL(0x10, actual_written_array.data[0]);
  TEST_ASSERT_EQUAL(0x40, actual_written_array.data[1]);
  TEST_ASSERT_EQUAL(0x54, actual_written_array.data[2]);
  TEST_ASSERT_EQUAL(0x94, actual_written_array.data[3]);
  TEST_ASSERT_EQUAL(0x16, actual_written_array.data[4]);
}

void test_data_link_layer_try_send_short_frame_reply_to_second_request(void) {
  // Arrange

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
  //  2. In the test function, the testable DataLinkLayer can be created with the fake uart interface.
  //     Then it can use call_try_send_short_frame(), and remember the return value.
  //     Then it waits for the call to finish. After that, it can assert everything is okay.

  FakeUartInterface uart_interface;
  const uint8_t fake_data[] = { 0xE5 };
  FakeUartInterfaceTaskArgs args = { 
    .uart_interface = &uart_interface, 
    .respond_to_nth_write = 2, 
    .delay_in_ms  = 5,
    .data_to_return = fake_data,
    .len_of_data_to_return = 1
  };
  xTaskCreatePinnedToCore(fake_uart_interface_task,
                    "fake_uart_interface_task", // name
                    20000,                      // stack size (in words)
                    &args,                      // input params
                    1,                          // priority
                    nullptr,                    // Handle, not needed
                    0                           // core
  );

  TestableDataLinkLayer dataLinkLayer(&uart_interface);

  // Act
  const uint8_t c = 0x40;
  const uint8_t a = 0x54;
  const bool actual_return_value = dataLinkLayer.call_try_send_short_frame(c, a);

  // Assert
  TEST_ASSERT_TRUE(actual_return_value);
  // Check the sent data: should be a short frame!
  // Start:     0x10
  // C:         0x40
  // A:         0x54
  // Check sum: 0x94
  // Stop:      0x16
  TEST_ASSERT_EQUAL(2, uart_interface.written_arrays_.size());
  FakeUartInterface::WrittenArray actual_written_array = uart_interface.written_arrays_[1];
  TEST_ASSERT_EQUAL(5, actual_written_array.len);
  TEST_ASSERT_EQUAL(0x10, actual_written_array.data[0]);
  TEST_ASSERT_EQUAL(0x40, actual_written_array.data[1]);
  TEST_ASSERT_EQUAL(0x54, actual_written_array.data[2]);
  TEST_ASSERT_EQUAL(0x94, actual_written_array.data[3]);
  TEST_ASSERT_EQUAL(0x16, actual_written_array.data[4]);
}

void test_data_link_layer_snd_nke_correct_response(void) {
  // What can be tested?
  //  - C field in short frame:
  //    FCB / FCV fields in C field: both should be 0
  //    Function field in C field: should be 0
  //    Bit 6 in C field: should be 1
  //    In other words: C should be 0x40
  //  - A field in short frame
  //  - Rest of sent short frame
  //  - Return value: only true if the correct response is received
  //  - Next-to-use value for FCB field for req_ud2 should be 1

  // Arrange
  FakeUartInterface uart_interface;
  const uint8_t fake_data[] = { 0xE5 };
  FakeUartInterfaceTaskArgs args = { 
    .uart_interface = &uart_interface,
    .respond_to_nth_write = 1,
    .delay_in_ms  = 5,
    .data_to_return = fake_data,
    .len_of_data_to_return = 1
  };
  xTaskCreatePinnedToCore(fake_uart_interface_task,
                    "fake_uart_interface_task", // name
                    20000,                      // stack size (in words)
                    &args,                      // input params
                    1,                          // priority
                    nullptr,                    // Handle, not needed
                    0                           // core
  );

  TestableDataLinkLayer dataLinkLayer(&uart_interface);

  // Act
  const uint8_t a = 0xB2;
  const bool actual_return_value = dataLinkLayer.call_snd_nke(a);

  // Assert
  TEST_ASSERT_TRUE(actual_return_value);
  
  // Check the sent data: should be a short frame!
  // Start:     0x10
  // C:         0x40
  // A:         0xB2
  // Check sum: 0xF2
  // Stop:      0x16
  TEST_ASSERT_EQUAL(1, uart_interface.written_arrays_.size());
  FakeUartInterface::WrittenArray actual_written_array = uart_interface.written_arrays_[0];
  TEST_ASSERT_EQUAL(5, actual_written_array.len);
  TEST_ASSERT_EQUAL(0x10, actual_written_array.data[0]);
  TEST_ASSERT_EQUAL(0x40, actual_written_array.data[1]);
  TEST_ASSERT_EQUAL(0xB2, actual_written_array.data[2]);
  TEST_ASSERT_EQUAL(0xF2, actual_written_array.data[3]);
  TEST_ASSERT_EQUAL(0x16, actual_written_array.data[4]);

  // Test the next-to-use FCB bit for req_ud2
  TEST_ASSERT_TRUE(dataLinkLayer.get_next_req_ud2_fcb());
}

int runUnityTests(void) {
  UNITY_BEGIN();
  RUN_TEST(test_data_link_layer_calculate_checksum);
  RUN_TEST(test_data_link_layer_try_send_short_frame_reply_to_first_request);
  RUN_TEST(test_data_link_layer_try_send_short_frame_reply_to_second_request);
  RUN_TEST(test_data_link_layer_snd_nke_correct_response);
  return UNITY_END();
}

void setup() {
  // Wait 2 seconds before the Unity test runner
  // establishes connection with a board Serial interface
  delay(2000);

  runUnityTests();
}

void loop() {}
