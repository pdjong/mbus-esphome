#ifdef UNIT_TEST
#include <test_includes.h>
#endif // UNIT_TEST

#include "DataBlockReader.h"

using std::vector;

namespace esphome {
namespace warmtemetermbus {

vector<Kamstrup303WA02::DataBlock*>* DataBlockReader::read_data_blocks_from_long_frame(Kamstrup303WA02::DataLinkLayer::LongFrame* long_frame) {
  auto *data_blocks = new vector<Kamstrup303WA02::DataBlock*>();

  this->long_frame_ = long_frame;
  current_position_in_user_data_ = Kamstrup303WA02::FIXED_DATA_HEADER_SIZE;
  const uint8_t user_data_length = long_frame->l - 3;

  while (current_position_in_user_data_ < user_data_length) {
    Kamstrup303WA02::DataBlock *next_data_block = this->read_next_data_block();
    data_blocks->push_back(next_data_block);
  }

  return data_blocks;
}

Kamstrup303WA02::DataBlock* DataBlockReader::read_next_data_block() {
  auto *data_block = new Kamstrup303WA02::DataBlock();

  // Read DIF / DIFE
  this->read_dif_into_block(data_block);
  // Read VIF / VIFE
  // Read data

  return data_block;
}

void DataBlockReader::read_dif_into_block(Kamstrup303WA02::DataBlock* data_block) {
  uint8_t dif = this->read_next_byte();
  data_block->storage_number = (dif & (1 << DIF_BIT_LSB_STORAGE_NUMBER)) >> DIF_BIT_LSB_STORAGE_NUMBER;
  data_block->function = static_cast<Kamstrup303WA02::Function>((dif & (0b11u << DIF_BIT_FUNCTION_FIELD_LOW_BIT)) >> DIF_BIT_FUNCTION_FIELD_LOW_BIT);
  uint8_t data_field = dif & DIF_BITS_DATA_FIELD;
  switch (data_field) {
    case 0:
      data_block->data_length = 0;
      break;
    case 1:
      data_block->data_length = 1;
      data_block->binary_data = new uint8_t[1];
      break;
    case 2:
      data_block->data_length = 2;
      data_block->binary_data = new uint8_t[2];
      break;
    case 3:
      data_block->data_length = 3;
      data_block->binary_data = new uint8_t[3];
      break;
    case 4:
      data_block->data_length = 4;
      data_block->binary_data = new uint8_t[4];
      break;
    default:
      data_block->data_length = 0;
      break;
  }
  bool dif_extended = (dif & (1 << DIF_BIT_EXTENDED)) >> DIF_BIT_EXTENDED;
  uint8_t extension_byte_index = 0;
  while (dif_extended) {
    uint8_t dif_extension = this->read_next_byte();
    
    uint8_t storage_number_shift = 4 * extension_byte_index + 1;
    uint64_t storage_number_extension = (dif_extension & DIFE_BITS_STORAGE_NUMBER) << storage_number_shift;
    data_block->storage_number |= storage_number_extension;

    uint8_t tariff_shift = 2 * extension_byte_index;
    uint32_t tariff_extension = ((dif_extended & DIFE_BITS_TARIFF) >> DIFE_BIT_TARIFF_LOW_BIT) << tariff_shift;
    data_block->tariff |= tariff_extension;
    
    ++extension_byte_index;
  }
}

uint8_t DataBlockReader::read_next_byte() {
  return this->long_frame_->user_data[this->current_position_in_user_data_++];
}

} //namespace warmtemetermbus
} //namespace esphome