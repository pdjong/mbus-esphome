#include <Arduino.h>
#include <test_includes.h>
#include <TestableKamstrup303WA02.h>
#include <DataBlockReader.h>
#include <unity.h>
#include <vector>

using std::vector;
using namespace esphome::warmtemetermbus;

void setUp(void) {}
void tearDown(void) {}

void test_datablockreader_read_data_blocks_from_long_frame_single_not_extended_dif_and_vif(void) {
  // Arrange
  DataBlockReader data_block_reader;
  uint8_t user_data[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // Fixed data header
    0x04, 0x06, 0x78, 0x56, 0x34, 0x12 // data block: instantaneous, 32 bit integer, Energy in 10^(6-3) Wh (=kWh), value 0x12345678
  };
  Kamstrup303WA02::DataLinkLayer::LongFrame long_frame = {
    .l = 21,
    .c = 0x08,
    .a = 0x0A,
    .ci = 0x72,
    .user_data = user_data
  };

  // Act
  vector<Kamstrup303WA02::DataBlock*>* actual_data_blocks = data_block_reader.read_data_blocks_from_long_frame(&long_frame);

  // Assert
  TEST_ASSERT_TRUE(actual_data_blocks != nullptr);
  TEST_ASSERT_EQUAL(1, actual_data_blocks->size());
  Kamstrup303WA02::DataBlock *actual_data_block = actual_data_blocks->at(0);
  TEST_ASSERT_EQUAL(Kamstrup303WA02::Function::instantaneous, actual_data_block->function);
  TEST_ASSERT_EQUAL(0, actual_data_block->storage_number);
  TEST_ASSERT_EQUAL(0, actual_data_block->tariff);
  TEST_ASSERT_EQUAL(3, actual_data_block->ten_power);
  TEST_ASSERT_EQUAL(0, actual_data_block->index);
  TEST_ASSERT_FALSE(actual_data_block->is_manufacturer_specific);
  TEST_ASSERT_EQUAL(4, actual_data_block->data_length);
  TEST_ASSERT_EQUAL(0x78, actual_data_block->binary_data[0]);
  TEST_ASSERT_EQUAL(0x56, actual_data_block->binary_data[1]);
  TEST_ASSERT_EQUAL(0x34, actual_data_block->binary_data[2]);
  TEST_ASSERT_EQUAL(0x12, actual_data_block->binary_data[3]);
}

int runUnityTests(void) {
  UNITY_BEGIN();
  RUN_TEST(test_datablockreader_read_data_blocks_from_long_frame_single_not_extended_dif_and_vif);
  return UNITY_END();
}

void setup() {
  // Wait 2 seconds before the Unity test runner
  // establishes connection with a board Serial interface
  delay(2000);

  runUnityTests();
}

void loop() {}
