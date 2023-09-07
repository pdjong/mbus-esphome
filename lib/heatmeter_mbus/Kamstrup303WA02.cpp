#ifdef UNIT_TEST
#include <test_includes.h>
#endif // UNIT_TEST
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "Kamstrup303WA02.h"
#include <Arduino.h>

namespace esphome {
namespace warmtemetermbus {

static const char * TAG {"Kamstrup303WA02"};


Kamstrup303WA02::Kamstrup303WA02(UartInterface* uart_interface) {
  this->data_link_layer_ = new DataLinkLayer(uart_interface);
}

bool Kamstrup303WA02::read_meter_data(Kamstrup303WA02::MbusMeterData* meter_data) {
  return false;
}

bool Kamstrup303WA02::read_data(Kamstrup303WA02::MeterData * const data) {
  ESP_LOGD(TAG, "read_data - enter");
  bool isSuccessful {false};
	DataLinkLayer::LongFrame telegramData;
	if (!this->data_link_layer_->req_ud2(0x01, &telegramData)) {
	 	return false;
	}
	
	if ((telegramData.ci >> 4) != 7) {
		return false;
	}

	switch (telegramData.ci & 0x03) {
		case 0x0:
			ESP_LOGE(TAG, "General App Error!");
			break;
		case 0x1:
			ESP_LOGI(TAG, "Alarm Status!");
			break;
		case 0x2: {
			// Variable data response
      ESP_LOGV(TAG, "Variable data response");      
      VariableDataRecord dataRecord;
      uint16_t startOfDataRecordIdx {12};
      
      // Heat energy E1
      readDataRecord(&dataRecord, &telegramData, &startOfDataRecordIdx);
      switch (dataRecord.unitAndMultiplier & 0x78) {
          case 0x00:
              // 000 0nnn: 10^(nnn-3) Wh
              data->heatEnergyE1.unit = Wh;
              data->heatEnergyE1.tenPower = (dataRecord.unitAndMultiplier & 0x07) - 3;
              break;
          case 0x08:
              // 000 1nnn: 10^nnn J
              data->heatEnergyE1.unit = J;
              data->heatEnergyE1.tenPower = dataRecord.unitAndMultiplier & 0x07;
              break;
          default:
              break;
      }
      copyDataToTargetBuffer(&dataRecord, &(data->heatEnergyE1.value));
      
      // Volume V1
      readDataRecord(&dataRecord, &telegramData, &startOfDataRecordIdx);
      data->volumeV1.tenPower = (dataRecord.unitAndMultiplier & 0x07) - 6;
      copyDataToTargetBuffer(&dataRecord, &(data->volumeV1.value));
      
      // Energy E8: inlet
      readDataRecord(&dataRecord, &telegramData, &startOfDataRecordIdx);
      // See MULTICAL_303_-_Technical_Description_-_English.pdf 7.1.2:
      data->energyE8.tenPower = -2 - data->volumeV1.tenPower;
      copyDataToTargetBuffer(&dataRecord, &(data->energyE8.value));
      
      // Energy E9: outlet
      readDataRecord(&dataRecord, &telegramData, &startOfDataRecordIdx);
      // See MULTICAL_303_-_Technical_Description_-_English.pdf 7.1.2:
      data->energyE9.tenPower = -2 - data->volumeV1.tenPower;
      copyDataToTargetBuffer(&dataRecord, &(data->energyE9.value));
      
      // Operating hours
      readDataRecord(&dataRecord, &telegramData, &startOfDataRecordIdx);
      data->operatingHours.unit = static_cast<DurationUnit>(dataRecord.unitAndMultiplier & 0x03);
      // this value is stored as 32-bit. But the meter sends a 24-bit value.
      // To prevent garbage values, set the value to zero first.
      data->operatingHours.value = 0;
      copyDataToTargetBuffer(&dataRecord, &(data->operatingHours.value));
      
      // Error hour counter
      readDataRecord(&dataRecord, &telegramData, &startOfDataRecordIdx);
      data->errorHourCounter.unit = static_cast<DurationUnit>(dataRecord.unitAndMultiplier & 0x03);
      copyDataToTargetBuffer(&dataRecord, &(data->errorHourCounter.value));
      
      // T1 actual
      readDataRecord(&dataRecord, &telegramData, &startOfDataRecordIdx);
      data->t1Actual.tenPower = (dataRecord.unitAndMultiplier & 0x03) - 3;
      copyDataToTargetBuffer(&dataRecord, &(data->t1Actual.value));
      
      // T2 actual
      readDataRecord(&dataRecord, &telegramData, &startOfDataRecordIdx);
      data->t2Actual.tenPower = (dataRecord.unitAndMultiplier & 0x03) - 3;
      copyDataToTargetBuffer(&dataRecord, &(data->t2Actual.value));
      
      // T1 - T2
      readDataRecord(&dataRecord, &telegramData, &startOfDataRecordIdx);
      data->diffT1T2.tenPower = (dataRecord.unitAndMultiplier & 0x03) - 3;
      copyDataToTargetBuffer(&dataRecord, &(data->diffT1T2.value));
      
      // Power E1/E3 actual
      readDataRecord(&dataRecord, &telegramData, &startOfDataRecordIdx);
      switch (dataRecord.unitAndMultiplier & 0x78) {
        case 0x28:
          // 010 1nnn: 10^(nnn-3) W
          data->powerE1OverE3Actual.unit = W;
          data->powerE1OverE3Actual.tenPower = (dataRecord.unitAndMultiplier & 0x07) - 3;
          break;
        case 0x08:
          // 011 0nnn: 10^nnn J/h
          data->powerE1OverE3Actual.unit = JperH;
          data->powerE1OverE3Actual.tenPower = dataRecord.unitAndMultiplier & 0x07;
          break;
        default:
          break;
      }
      copyDataToTargetBuffer(&dataRecord, &(data->powerE1OverE3Actual.value));
      
      // Power max month
      readDataRecord(&dataRecord, &telegramData, &startOfDataRecordIdx);
      switch (dataRecord.unitAndMultiplier & 0x78) {
        case 0x28:
          // 010 1nnn: 10^(nnn-3) W
          data->powerMaxMonth.unit = W;
          data->powerMaxMonth.tenPower = (dataRecord.unitAndMultiplier & 0x07) - 3;
          break;
        case 0x08:
          // 011 0nnn: 10^nnn J/h
          data->powerMaxMonth.unit = JperH;
          data->powerMaxMonth.tenPower = dataRecord.unitAndMultiplier & 0x07;
          break;
        default:
          break;
      }
      copyDataToTargetBuffer(&dataRecord, &(data->powerMaxMonth.value));
      
      // Flow V1 actual
      readDataRecord(&dataRecord, &telegramData, &startOfDataRecordIdx);
      switch (dataRecord.unitAndMultiplier & 0x78) {
        case 0x38:
          data->flowV1Actual.unit = m3PerHour;
          data->flowV1Actual.tenPower = (dataRecord.unitAndMultiplier & 0x07) - 6;
          break;
        case 0x40:
          data->flowV1Actual.unit = m3PerMinute;
          data->flowV1Actual.tenPower = (dataRecord.unitAndMultiplier & 0x07) - 7;
          break;
        case 0x48:
          data->flowV1Actual.unit = m3PerSecond;
          data->flowV1Actual.tenPower = (dataRecord.unitAndMultiplier & 0x07) - 9;
          break;
        default:
          break;
      }
      copyDataToTargetBuffer(&dataRecord, &(data->flowV1Actual.value));
      
      // Flow v1 max month
      readDataRecord(&dataRecord, &telegramData, &startOfDataRecordIdx);
      switch (dataRecord.unitAndMultiplier & 0x78) {
        case 0x38:
          data->flowV1MaxMonth.unit = m3PerHour;
          data->flowV1MaxMonth.tenPower = (dataRecord.unitAndMultiplier & 0x07) - 6;
          break;
        case 0x40:
          data->flowV1MaxMonth.unit = m3PerMinute;
          data->flowV1MaxMonth.tenPower = (dataRecord.unitAndMultiplier & 0x07) - 7;
          break;
        case 0x48:
          data->flowV1MaxMonth.unit = m3PerSecond;
          data->flowV1MaxMonth.tenPower = (dataRecord.unitAndMultiplier & 0x07) - 9;
          break;
        default:
          break;
      }
      copyDataToTargetBuffer(&dataRecord, &(data->flowV1MaxMonth.value));
      
      // Info bits
      readDataRecord(&dataRecord, &telegramData, &startOfDataRecordIdx);
      copyDataToTargetBuffer(&dataRecord, &(data->infoBits));
      
      // Heat energy E1 - old
      readDataRecord(&dataRecord, &telegramData, &startOfDataRecordIdx);
      switch (dataRecord.unitAndMultiplier & 0x78) {
        case 0x00:
          // 000 0nnn: 10^(nnn-3) Wh
          data->heatEnergyE1Old.unit = Wh;
          data->heatEnergyE1Old.tenPower = (dataRecord.unitAndMultiplier & 0x07) - 3;
          break;
        case 0x08:
          // 000 1nnn: 10^nnn J
          data->heatEnergyE1Old.unit = J;
          data->heatEnergyE1.tenPower = dataRecord.unitAndMultiplier & 0x07;
          break;
        default:
          break;
      }
      copyDataToTargetBuffer(&dataRecord, &(data->heatEnergyE1Old.value));
      
      // Volume V1 - old
      readDataRecord(&dataRecord, &telegramData, &startOfDataRecordIdx);
      data->volumeV1Old.tenPower = (dataRecord.unitAndMultiplier & 0x07) - 6;
      copyDataToTargetBuffer(&dataRecord, &(data->volumeV1Old.value));
                  
      // Energy E8: inlet - old
      readDataRecord(&dataRecord, &telegramData, &startOfDataRecordIdx);
      // See MULTICAL_303_-_Technical_Description_-_English.pdf 7.1.2:
      data->energyE8Old.tenPower = -2 - data->heatEnergyE1.tenPower;
      copyDataToTargetBuffer(&dataRecord, &(data->energyE8Old.value));
                  
      // Energy E9: outlet - old
      readDataRecord(&dataRecord, &telegramData, &startOfDataRecordIdx);
      // See MULTICAL_303_-_Technical_Description_-_English.pdf 7.1.2:
      data->energyE9Old.tenPower = -2 - data->heatEnergyE1.tenPower;
      copyDataToTargetBuffer(&dataRecord, &(data->energyE9Old.value));
      
      // Power max year - old
      readDataRecord(&dataRecord, &telegramData, &startOfDataRecordIdx);
      switch (dataRecord.unitAndMultiplier & 0x78) {
        case 0x28:
          // 010 1nnn: 10^(nnn-3) W
          data->powerMaxYear.unit = W;
          data->powerMaxYear.tenPower = (dataRecord.unitAndMultiplier & 0x07) - 3;
          break;
        case 0x08:
          // 011 0nnn: 10^nnn J/h
          data->powerMaxYear.unit = JperH;
          data->powerMaxYear.tenPower = dataRecord.unitAndMultiplier & 0x07;
          break;
        default:
          break;
      }
      copyDataToTargetBuffer(&dataRecord, &(data->powerMaxYear.value));
      
      // Flow V1 max year
      readDataRecord(&dataRecord, &telegramData, &startOfDataRecordIdx);
      switch (dataRecord.unitAndMultiplier & 0x78) {
        case 0x38:
          data->flowV1MaxYear.unit = m3PerHour;
          data->flowV1MaxYear.tenPower = (dataRecord.unitAndMultiplier & 0x07) - 6;
          break;
        case 0x40:
          data->flowV1MaxYear.unit = m3PerMinute;
          data->flowV1MaxYear.tenPower = (dataRecord.unitAndMultiplier & 0x07) - 7;
          break;
        case 0x48:
          data->flowV1MaxYear.unit = m3PerSecond;
          data->flowV1MaxYear.tenPower = (dataRecord.unitAndMultiplier & 0x07) - 9;
          break;
        default:
          break;  
      }
      copyDataToTargetBuffer(&dataRecord, &(data->flowV1MaxYear.value));
      
      // Date and Time (logged)
      readDataRecord(&dataRecord, &telegramData, &startOfDataRecordIdx);
      // Bits 0-4: day
      // Bits 8-11: month
      // Bits 5-7 & 12-15: year
      const uint16_t * const dateTimeBits {reinterpret_cast<uint16_t*>(dataRecord.data)};
      data->dateTimeLogged.day = *dateTimeBits & 0x001F;
      data->dateTimeLogged.month = (*dateTimeBits & 0x0F00) >> 8;
      data->dateTimeLogged.year = ((*dateTimeBits & 0xF000) >> 9) | ((*dateTimeBits & 0x00E0) >> 5);
      
      isSuccessful = true;
			break;
		}
		case 0x3:
			ESP_LOGV(TAG, "Static data response");
			break;
		default:
      ESP_LOGW(TAG, "Unknown response to REQ_UD2");
			break;
	}
  ESP_LOGD(TAG, "read_data - exit");
	return isSuccessful;	
}

void Kamstrup303WA02::readDataRecord(VariableDataRecord * const dataRecord, const DataLinkLayer::LongFrame * const userData, uint16_t * const startOfDataRecordIdx) {
  const uint8_t dif {userData->user_data[*startOfDataRecordIdx]};

  dataRecord->dataType = dif & 0x0F;
  dataRecord->function = (dif & (3 << 4)) >> 4;
  dataRecord->storageNumber = (dif & (1 << 6)) >> 6;
  
  dataRecord->subUnit = 0;
  dataRecord->tariff = 0;
  
  bool isExtended {dif & (1 << 7)};
  uint16_t edifIdx {static_cast<uint16_t>(*startOfDataRecordIdx + 1)};
  uint8_t edifNr {0};
  while (isExtended) {
      uint8_t edif {userData->user_data[edifIdx++]};
      
      uint32_t subUnitInEdif {static_cast<uint32_t>((edif | (1 << 6)) >> 6)};
      subUnitInEdif <<= edifNr;
      dataRecord->subUnit |= subUnitInEdif;
      
      uint32_t tariffInEdif {static_cast<uint32_t>((edif | (3 << 4)) >> 4)};
      tariffInEdif <<= (edifNr * 2);
      dataRecord->tariff |= tariffInEdif;
      
      uint64_t storageNrInEdif {static_cast<uint64_t>(edif | 0xF)};
      storageNrInEdif <<= (edifNr * 4 + 1);
      dataRecord->storageNumber |= storageNrInEdif;
      
      isExtended = edif & (1 << 7);
      ++edifNr;
  }
  
  // Now read the VIB - start with VIF
  uint16_t vifIdx {edifIdx};
  const uint8_t vif {userData->user_data[vifIdx]};

  dataRecord->unitAndMultiplier = vif & 0x7F;

  isExtended = vif & (1 << 7);
  uint16_t evifIdx {static_cast<uint16_t>(vifIdx + 1)};
  while (isExtended) {
      const uint8_t evif {userData->user_data[evifIdx++]};
      isExtended = evif & (1 << 7);
  }

  // Find out data length. Number of bytes depends on DIF data field.
  uint16_t dataIdx {evifIdx};
  uint8_t dataLength {0};

  switch (dataRecord->dataType) {
      case 0x0:
         [[fallthrough]];
      case 0x1:
         [[fallthrough]];
      case 0x2:
         [[fallthrough]];
      case 0x3:
         [[fallthrough]];
      case 0x4:
          dataLength = dataRecord->dataType;
          break;
      case 0x5:
          dataLength = 4;
          break;
      case 0x6:
          dataLength = 6;
          break;
      case 0x7:
          dataLength = 8;
          break;
      case 0x9:
          dataLength = 1;
          break;
      case 0xA:
          dataLength = 2;
          break;
      case 0xB:
          dataLength = 3;
          break;
      case 0xC:
          dataLength = 4;
          break;
      case 0xD:
          dataLength = userData->user_data[dataIdx++];
          break;
      case 0xE:
          dataLength = 3;
          break;
      default:
          dataLength = 1;
          break;
  }    
  dataRecord->dataLength = dataLength;
  
  // Read data
  // For now store at max 8 bytes
  // Initialize data to 0!
  for (uint8_t i {0}; i < 8; ++i) {
    dataRecord->data[i] = 0;
  }
  for (uint8_t i {0}; i < dataLength; ++i) {
      const uint8_t currentByte {userData->user_data[dataIdx + i]};
      if (i < 8) {
          dataRecord->data[i] = currentByte;
      }
  }

  *startOfDataRecordIdx = dataIdx + dataLength;
}

void Kamstrup303WA02::copyDataToTargetBuffer(VariableDataRecord* dataRecord, void* targetBuffer) {
    uint8_t *byteTargetBuffer {reinterpret_cast<uint8_t*>(targetBuffer)};
    for (uint8_t i {0}; i < dataRecord->dataLength; ++i) {
        *byteTargetBuffer++ = dataRecord->data[i];
    }
}

bool Kamstrup303WA02::DataLinkLayer::req_ud2(const uint8_t address, LongFrame* response_frame) {
  bool success { false };

  if (!this->meter_is_initialized_) {
    if (this->snd_nke(address)) {
      this->meter_is_initialized_ = true;
    } else {
      ESP_LOGI(TAG, "Could not initialize meter");
      return false;
    }
  }
  const uint8_t fcb = this->next_req_ud2_fcb_ ? 1u : 0u;
  const uint8_t c = (1 << C_FIELD_BIT_DIRECTION) | (fcb << C_FIELD_BIT_FCB) | (1 << C_FIELD_BIT_FCV) | C_FIELD_FUNCTION_REQ_UD2;
  bool received_response_to_request = this->try_send_short_frame(c, address);
  if (received_response_to_request) {
    const bool received_sane_response = this->parse_long_frame_response(response_frame);
    if (received_sane_response && response_frame->a == address) {
      success = true;
    }
  }

  if (success) {
    this->next_req_ud2_fcb_ = !this->next_req_ud2_fcb_;
  }
  return success;
}

bool Kamstrup303WA02::DataLinkLayer::parse_long_frame_response(Kamstrup303WA02::DataLinkLayer::LongFrame* long_frame) {
  long_frame->user_data = nullptr;

  uint8_t current_byte { 0 };

  // Check start byte
  if (!this->read_next_byte(&current_byte) || (current_byte != START_BYTE_CONTROL_AND_LONG_FRAME)) {
    this->flush_rx_buffer();
    return false;
  }

  // Check two identical L fields
  uint8_t first_l_field { 0 };
  uint8_t second_l_field { 0 };
  if (!this->read_next_byte(&first_l_field) || !this->read_next_byte(&second_l_field)) {
    this->flush_rx_buffer();
    return false;
  }
  if (first_l_field != second_l_field) {
    this->flush_rx_buffer();
    return false;
  }
  long_frame->l = first_l_field;

  // Check 2nd start byte
  if (!this->read_next_byte(&current_byte) || (current_byte != START_BYTE_CONTROL_AND_LONG_FRAME)) {
    this->flush_rx_buffer();
    return false;
  }
  
  // Check C field
  if (!this->read_next_byte(&long_frame->c) || ((long_frame->c & 0x0F) != 0x08) || ((long_frame->c & 0xC0) != 0x00)) {
    this->flush_rx_buffer();
    return false;
  }

  // Read A field
  if (!this->read_next_byte(&long_frame->a)) {
    this->flush_rx_buffer();
    return false;
  }

  // Read CI field
  if (!this->read_next_byte(&long_frame->ci)) {
    this->flush_rx_buffer();
    return false;
  }

  // Read user data
  // Expected amount of user data: L - 3 (3 for C, A, CI)
  const uint8_t user_data_len = long_frame->l - 3;
  long_frame->user_data = new uint8_t[user_data_len];
  for (uint8_t i { 0 }; i < user_data_len; ++i) {
    if (!this->read_next_byte(&current_byte)) {
      this->flush_rx_buffer();
      return false;
    }
    long_frame->user_data[i] = current_byte;
  }

  // Calculate, read and check the check sum
  const uint8_t calculated_check_sum = this->calculate_checksum(long_frame);
  if (!this->read_next_byte(&long_frame->check_sum) || (long_frame->check_sum != calculated_check_sum)) {
    this->flush_rx_buffer();
    return false;
  }

  // Check stop byte
  if (!this->read_next_byte(&current_byte) || (STOP_BYTE != current_byte)) {
    this->flush_rx_buffer();
    return false;
  }

  // Flip the FCB bit to use for next REQ_UD2 message (see 5.5)
  this->next_req_ud2_fcb_ = !this->next_req_ud2_fcb_;
  return true;
}

bool Kamstrup303WA02::DataLinkLayer::read_next_byte(uint8_t* received_byte) {
  const uint32_t time_before_starting_to_wait { millis() };
	while (this->uart_interface_->available() == 0) {
    delay(1);
    if (millis() - time_before_starting_to_wait > 150) {
      ESP_LOGE(TAG, "No data available after timeout");
      return false;
    }
  }
  this->uart_interface_->read_byte(received_byte);
	return true;
}

bool Kamstrup303WA02::DataLinkLayer::snd_nke(const uint8_t address) {
  bool success { false };

  const uint8_t c = (1 << C_FIELD_BIT_DIRECTION) | (C_FIELD_FUNCTION_SND_NKE);
  bool received_response_to_short_frame = try_send_short_frame(c, address);
  if (received_response_to_short_frame) {
    uint8_t received_byte { 0 };
    this->uart_interface_->read_byte(&received_byte);
    if (START_BYTE_SINGLE_CHARACTER == received_byte) {
      success = true;
      this->next_req_ud2_fcb_ = true;
    } else {
      ESP_LOGE(TAG, "Wrong answer to SND_NKE: %X", received_byte);
    }
  } else {
    ESP_LOGE(TAG, "No response to SND_NKE");
  }

  return success;
}

// Slave must wait at least 11 bit times, and at max 330 bit times + 50ms before answering.
// In case no answer within that time, retry at most twice.
// (see 5.4 Communication Process)
bool Kamstrup303WA02::DataLinkLayer::try_send_short_frame(const uint8_t c, const uint8_t a) {
  bool success { false };
  bool dataIsReceived { false };
  flush_rx_buffer();
  for (uint8_t transmitAttempt {0}; transmitAttempt < 3 && !dataIsReceived; ++transmitAttempt) {
    if (transmitAttempt > 0) {
      ESP_LOGD(TAG, "Retry transmit short frame");
    }
    send_short_frame(c, a);
    // Sending takes about 4,58ms per byte. Short frame takes about 23ms to send.
    vTaskDelay(25 / portTICK_PERIOD_MS);
    dataIsReceived = wait_for_incoming_data();
  }
  success = dataIsReceived;
  return success;
}

void Kamstrup303WA02::DataLinkLayer::flush_rx_buffer() {
  while (this->uart_interface_->available()) {
    int32_t byteCountInBuffer {this->uart_interface_->available()};
    if (byteCountInBuffer > 255) {
      byteCountInBuffer = 255;
    }
    uint8_t bytesInBuffer[byteCountInBuffer];
    this->uart_interface_->read_array(bytesInBuffer, byteCountInBuffer);
  }
}

void Kamstrup303WA02::DataLinkLayer::send_short_frame(const uint8_t c, const uint8_t a) {
  const uint8_t data[] = { c, a };
  const uint8_t checksum { calculate_checksum(data, 2) };
  const uint8_t short_frame[] = { START_BYTE_SHORT_FRAME, c, a, checksum, STOP_BYTE };
  this->uart_interface_->write_array(short_frame, 5);
  delay(1);
  this->uart_interface_->flush();
  delay(1);
}

// TODO: rename to wait_for_incoming_telegram
bool Kamstrup303WA02::DataLinkLayer::wait_for_incoming_data() {
  bool dataReceived {false};
  // 330 bits + 50ms = 330 * 1000 / 2400 + 50 ms = 187,5 ms
  // Wait at least 11 bit times = 5ms
  delay(138);
  for (uint16_t i {0}; i < 500; ++i) {
    if (this->uart_interface_->available() > 0) {
      dataReceived = true;
      break;
    }
    delay(1);
  }
  if (!dataReceived) {
    ESP_LOGE(TAG, "waitForIncomingData - exit - No data received");
  }
  return dataReceived;
}

uint8_t Kamstrup303WA02::DataLinkLayer::calculate_checksum(const LongFrame* long_frame) const {
  const uint8_t user_data_len = long_frame->l - 3;
  uint8_t checksum = this->calculate_checksum(long_frame->user_data, user_data_len);
  checksum += long_frame->c;
  checksum += long_frame->a;
  checksum += long_frame->ci;
  return checksum;
}

uint8_t Kamstrup303WA02::DataLinkLayer::calculate_checksum(const uint8_t* data, size_t length) const {
  uint8_t checksum { 0 };
  for (size_t i = 0; i < length; ++i) {
    checksum += data[i];
  }
  return checksum;
}

} //namespace warmtemetermbus
} //namespace esphome