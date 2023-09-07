#ifdef UNIT_TEST
#include <test_includes.h>
#endif // UNIT_TEST

#include <Arduino.h>
#include "DataBlockReader.h"

using std::vector;

namespace esphome {
namespace warmtemetermbus {

static const char * TAG {"DataBlockReader"};

vector<Kamstrup303WA02::DataBlock*>* DataBlockReader::read_data_blocks_from_long_frame(Kamstrup303WA02::DataLinkLayer::LongFrame* long_frame) {
  auto *data_blocks = new vector<Kamstrup303WA02::DataBlock*>();

  this->long_frame_ = long_frame;
  current_position_in_user_data_ = Kamstrup303WA02::FIXED_DATA_HEADER_SIZE;
  const uint8_t user_data_length = long_frame->l - 3;

  uint8_t data_block_index = 0;
  while (current_position_in_user_data_ < user_data_length) {
     Kamstrup303WA02::DataBlock *next_data_block = this->read_next_data_block();
     next_data_block->index = data_block_index++;
     data_blocks->push_back(next_data_block);
  }

  return data_blocks;
}

Kamstrup303WA02::DataBlock* DataBlockReader::read_next_data_block() {
  auto *data_block = new Kamstrup303WA02::DataBlock();

  this->read_dif_into_block(data_block);
  this->read_vif_into_block(data_block);
  this->read_data_into_block(data_block);

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
      ESP_LOGI(TAG, "Data Field %x not supported", data_field);
      data_block->data_length = 0;
      data_block->binary_data = nullptr;
      break;
  }
  bool dif_extended = (dif & (1 << DIF_BIT_EXTENDED)) >> DIF_BIT_EXTENDED;
  uint8_t extension_byte_index = 0;
  while (dif_extended) {
    uint8_t dife = this->read_next_byte();
    
    uint8_t storage_number_shift = 4 * extension_byte_index + 1;
    uint64_t storage_number_extension = (dife & DIFE_BITS_STORAGE_NUMBER) << storage_number_shift;
    data_block->storage_number |= storage_number_extension;

    uint8_t tariff_shift = 2 * extension_byte_index;
    uint32_t tariff_extension = ((dife & DIFE_BITS_TARIFF) >> DIFE_BIT_TARIFF_LOW_BIT) << tariff_shift;
    data_block->tariff |= tariff_extension;
    
    dif_extended = (dife & (1 << DIF_BIT_EXTENDED)) >> DIF_BIT_EXTENDED;
    ++extension_byte_index;
  }
}

void DataBlockReader::read_vif_into_block(Kamstrup303WA02::DataBlock* data_block) {
  uint8_t vif = this->read_next_byte();
  uint8_t unit_and_multiplier = vif & VIF_BITS_UNIT_AND_MULTIPLIER;
  
  bool vif_is_primary = (unit_and_multiplier <= 0b01111011); // See 6.3.2, Value Information Block (VIB)
  bool vif_is_manufacturer_specific = (unit_and_multiplier == 0b01111111);
  if (vif_is_primary) {
    if ((unit_and_multiplier & 0b1111000) == 0b0000000) {
      // Energy in Wh
      data_block->ten_power = (unit_and_multiplier & 0b111) - 3;
      data_block->unit = Kamstrup303WA02::Unit::Wh;
    } else if ((unit_and_multiplier & 0b1111000) == 0b0001000) {
      // Energy in J
      data_block->ten_power = unit_and_multiplier & 0b111;
      data_block->unit = Kamstrup303WA02::Unit::J;
    } else {
      ESP_LOGI(TAG, "Primary VIF with unit and multiplier %x not yet supported", unit_and_multiplier);
    }
  } else if (vif_is_manufacturer_specific) {
    data_block->unit = Kamstrup303WA02::Unit::manufacturer_specific;
    data_block->is_manufacturer_specific = true;
  } else {
    ESP_LOGI(TAG, "Only primary VIF or manufacturer specific supported. VIF %x unsupported.", vif);
  }

  // Ignore the VIF extension; just skip over it
  uint8_t vif_is_extend = (vif & (1 << VIF_BIT_EXTENDED)) >> VIF_BIT_EXTENDED;
  while (vif_is_extend) {
    uint8_t vif_extension = this->read_next_byte();
    vif_is_extend = (vif_extension & (1 << VIF_BIT_EXTENDED)) >> VIF_BIT_EXTENDED;
  }
}

void DataBlockReader::read_data_into_block(Kamstrup303WA02::DataBlock* data_block) {
  for (uint8_t i = 0; i < data_block->data_length; ++i) {
    data_block->binary_data[i] = this->read_next_byte();
  }
}

uint8_t DataBlockReader::read_next_byte() {
  return this->long_frame_->user_data[this->current_position_in_user_data_++];
}

} //namespace warmtemetermbus
} //namespace esphome