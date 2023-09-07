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
  TEST_ASSERT_EQUAL(Kamstrup303WA02::Unit::Wh, actual_data_block->unit);
  TEST_ASSERT_EQUAL(0, actual_data_block->index);
  TEST_ASSERT_FALSE(actual_data_block->is_manufacturer_specific);
  TEST_ASSERT_EQUAL(4, actual_data_block->data_length);
  TEST_ASSERT_EQUAL(0x78, actual_data_block->binary_data[0]);
  TEST_ASSERT_EQUAL(0x56, actual_data_block->binary_data[1]);
  TEST_ASSERT_EQUAL(0x34, actual_data_block->binary_data[2]);
  TEST_ASSERT_EQUAL(0x12, actual_data_block->binary_data[3]);
}

void test_datablockreader_read_data_blocks_from_long_frame_single_extended_dif_and_vif_one_manufacturer_specific(void) {
  // Arrange
  DataBlockReader data_block_reader;
  uint8_t user_data[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // Fixed data header
    0x84, 0xB1, 0x20, 0x06, 0x25, 0x0F, 0x00, 0x00, // data block: instantaneous, 32 bit integer, Energy in 10^(6-3) Wh (=kWh), value 0x00000F25; storage number 0b10 = 2, tariff 0b1011 = 0xB
    0x12, 0xFF, 0x07, 0xC4, 0x81 // data block: maximum, 16 bit integer, manufacturer specific, value 0x81C4
  };
  Kamstrup303WA02::DataLinkLayer::LongFrame long_frame = {
    .l = 28,
    .c = 0x08,
    .a = 0x0A,
    .ci = 0x72,
    .user_data = user_data
  };

  // Act
  vector<Kamstrup303WA02::DataBlock*>* actual_data_blocks = data_block_reader.read_data_blocks_from_long_frame(&long_frame);

  // Assert
  TEST_ASSERT_TRUE(actual_data_blocks != nullptr);
  TEST_ASSERT_EQUAL(2, actual_data_blocks->size());

  // Block 0
  // DIF: 0b1000 0100 DIFE: 0b1011 0001 DIFE: 0b0010 0000
  //  Data length / function: 4, instantaneous
  //  Storage nr: 0b0 0000 0010 = 2
  //  Tariff: 0b1011 = 11
  // VIF: 0b0000 0110
  //  Primary VIF, Energy in Wh, 10^(6 - 3)
  Kamstrup303WA02::DataBlock *actual_data_block = actual_data_blocks->at(0);
  TEST_ASSERT_EQUAL(Kamstrup303WA02::Function::instantaneous, actual_data_block->function);
  TEST_ASSERT_EQUAL(2, actual_data_block->storage_number);
  TEST_ASSERT_EQUAL(11, actual_data_block->tariff);
  TEST_ASSERT_EQUAL(3, actual_data_block->ten_power);
  TEST_ASSERT_EQUAL(Kamstrup303WA02::Unit::Wh, actual_data_block->unit);
  TEST_ASSERT_EQUAL(0, actual_data_block->index);
  TEST_ASSERT_FALSE(actual_data_block->is_manufacturer_specific);
  TEST_ASSERT_EQUAL(4, actual_data_block->data_length);
  TEST_ASSERT_EQUAL(0x25, actual_data_block->binary_data[0]);
  TEST_ASSERT_EQUAL(0x0F, actual_data_block->binary_data[1]);
  TEST_ASSERT_EQUAL(0x00, actual_data_block->binary_data[2]);
  TEST_ASSERT_EQUAL(0x00, actual_data_block->binary_data[3]);

  // Block 1
  // DIF: 0b0001 0010
  //  Data length / function: 2, maximum
  //  Storage nr: 0
  //  Tariff: 0
  // VIF: 0b1111 1111
  //  Manufacturer specific, extended
  actual_data_block = actual_data_blocks->at(1);
  TEST_ASSERT_EQUAL(Kamstrup303WA02::Function::maximum, actual_data_block->function);
  TEST_ASSERT_EQUAL(0, actual_data_block->storage_number);
  TEST_ASSERT_EQUAL(0, actual_data_block->tariff);
  TEST_ASSERT_EQUAL(0, actual_data_block->ten_power);
  TEST_ASSERT_EQUAL(Kamstrup303WA02::Unit::manufacturer_specific, actual_data_block->unit);
  TEST_ASSERT_EQUAL(1, actual_data_block->index);
  TEST_ASSERT_TRUE(actual_data_block->is_manufacturer_specific);
  TEST_ASSERT_EQUAL(2, actual_data_block->data_length);
  TEST_ASSERT_EQUAL(0xC4, actual_data_block->binary_data[0]);
  TEST_ASSERT_EQUAL(0x81, actual_data_block->binary_data[1]);
}

void test_datablockreader_read_data_blocks_from_long_frame_single_block_dif_minimum_8_bit(void) {
  // Arrange
  DataBlockReader data_block_reader;
  uint8_t user_data[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // Fixed data header
    0x21, 0x05, 0x12 // data block: minimum, 8 bit integer, Energy in 10^(5-3) Wh (=kWh), value 0x12
  };
  Kamstrup303WA02::DataLinkLayer::LongFrame long_frame = {
    .l = 18,
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
  TEST_ASSERT_EQUAL(Kamstrup303WA02::Function::minimum, actual_data_block->function);
  TEST_ASSERT_EQUAL(0, actual_data_block->storage_number);
  TEST_ASSERT_EQUAL(0, actual_data_block->tariff);
  TEST_ASSERT_EQUAL(2, actual_data_block->ten_power);
  TEST_ASSERT_EQUAL(Kamstrup303WA02::Unit::Wh, actual_data_block->unit);
  TEST_ASSERT_EQUAL(0, actual_data_block->index);
  TEST_ASSERT_FALSE(actual_data_block->is_manufacturer_specific);
  TEST_ASSERT_EQUAL(1, actual_data_block->data_length);
  TEST_ASSERT_EQUAL(0x12, actual_data_block->binary_data[0]);
}

void test_datablockreader_read_data_blocks_from_long_frame_single_block_dif_during_error_state_24_bit(void) {
  // Arrange
  DataBlockReader data_block_reader;
  uint8_t user_data[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // Fixed data header
    0x33, 0x05, 0x12, 0x34, 0x56 // data block: during error state, 24 bit integer, Energy in 10^(5-3) Wh (=kWh), value 0x563412
  };
  Kamstrup303WA02::DataLinkLayer::LongFrame long_frame = {
    .l = 20,
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
  TEST_ASSERT_EQUAL(Kamstrup303WA02::Function::during_error_state, actual_data_block->function);
  TEST_ASSERT_EQUAL(0, actual_data_block->storage_number);
  TEST_ASSERT_EQUAL(0, actual_data_block->tariff);
  TEST_ASSERT_EQUAL(2, actual_data_block->ten_power);
  TEST_ASSERT_EQUAL(Kamstrup303WA02::Unit::Wh, actual_data_block->unit);
  TEST_ASSERT_EQUAL(0, actual_data_block->index);
  TEST_ASSERT_FALSE(actual_data_block->is_manufacturer_specific);
  TEST_ASSERT_EQUAL(3, actual_data_block->data_length);
  TEST_ASSERT_EQUAL(0x12, actual_data_block->binary_data[0]);
  TEST_ASSERT_EQUAL(0x34, actual_data_block->binary_data[1]);
  TEST_ASSERT_EQUAL(0x56, actual_data_block->binary_data[2]);
}

int runUnityTests(void) {
  UNITY_BEGIN();
  RUN_TEST(test_datablockreader_read_data_blocks_from_long_frame_single_not_extended_dif_and_vif);
  RUN_TEST(test_datablockreader_read_data_blocks_from_long_frame_single_extended_dif_and_vif_one_manufacturer_specific);
  RUN_TEST(test_datablockreader_read_data_blocks_from_long_frame_single_block_dif_minimum_8_bit);
  RUN_TEST(test_datablockreader_read_data_blocks_from_long_frame_single_block_dif_during_error_state_24_bit);
  return UNITY_END();
}

void setup() {
  // Wait 2 seconds before the Unity test runner
  // establishes connection with a board Serial interface
  delay(2000);

  runUnityTests();
}

void loop() {}
