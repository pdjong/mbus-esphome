#include "Kamstrup303WA02.h"

namespace esphome {
namespace warmtemetermbus {

static const char * TAG = "Kamstrup303WA02";


bool Kamstrup303WA02::readData(Kamstrup303WA02::MeterData* data) {
  ESP_LOGD(TAG, "readData - enter");
  bool isSuccessful = false;
	DataLinkLayer::TelegramData telegramData;
	if (!dataLinkLayer.reqUd2(0x01, &telegramData)) {
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
      ESP_LOGI(TAG, "Variable data response");      
      VariableDataRecord dataRecord;
      uint16_t startOfDataRecordIdx = 12;
      
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
      uint16_t* dateTimeBits = reinterpret_cast<uint16_t*>(dataRecord.data);
      data->dateTimeLogged.day = *dateTimeBits & 0x001F;
      data->dateTimeLogged.month = (*dateTimeBits & 0x0F00) >> 8;
      data->dateTimeLogged.year = ((*dateTimeBits & 0xF000) >> 9) | ((*dateTimeBits & 0x00E0) >> 5);
      
      isSuccessful = true;
			break;
		}
		case 0x3:
			ESP_LOGD(TAG, "Static data response");
			break;
		default:
      ESP_LOGW(TAG, "Unknown response to REQ_UD2");
			break;
	}
  ESP_LOGD(TAG, "readData - exit");
	return isSuccessful;	
}

void Kamstrup303WA02::readDataRecord(VariableDataRecord* dataRecord, DataLinkLayer::TelegramData* userData, uint16_t* startOfDataRecordIdx) {
  uint8_t dif = userData->data[*startOfDataRecordIdx];

  dataRecord->dataType = dif & 0x0F;
  dataRecord->function = (dif & (3 << 4)) >> 4;
  dataRecord->storageNumber = (dif & (1 << 6)) >> 6;
  
  dataRecord->subUnit = 0;
  dataRecord->tariff = 0;
  
  bool isExtended = dif & (1 << 7);
  uint8_t edifIdx = *startOfDataRecordIdx + 1;
  uint8_t edifNr = 0;
  while (isExtended) {
      uint8_t edif = userData->data[edifIdx++];
      
      unsigned long subUnitInEdif = (edif | (1 << 6)) >> 6;
      subUnitInEdif <<= edifNr;
      dataRecord->subUnit |= subUnitInEdif;
      
      unsigned long tariffInEdif = (edif | (3 << 4)) >> 4;
      tariffInEdif <<= (edifNr * 2);
      dataRecord->tariff |= tariffInEdif;
      
      unsigned long long storageNrInEdif = edif | 0xF;
      storageNrInEdif <<= (edifNr * 4 + 1);
      dataRecord->storageNumber |= storageNrInEdif;
      
      isExtended = edif & (1 << 7);
      ++edifNr;
  }
  
  // Now read the VIB - start with VIF
  uint8_t vifIdx = edifIdx;
  uint8_t vif = userData->data[vifIdx];

  dataRecord->unitAndMultiplier = vif & 0x7F;

  isExtended = vif & (1 << 7);
  uint8_t evifIdx = vifIdx + 1;
  while (isExtended) {
      uint8_t evif = userData->data[evifIdx++];
      isExtended = evif & (1 << 7);
  }

  // Find out data length. Number of bytes depends on DIF data field.
  uint8_t dataIdx = evifIdx;
  uint8_t dataLength = 0;

  switch (dataRecord->dataType) {
      case 0x0:
      case 0x1:
      case 0x2:
      case 0x3:
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
          dataLength = userData->data[dataIdx++];
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
  for (uint8_t i = 0; i < 8; ++i) {
    dataRecord->data[i] = 0;
  }
  for (uint8_t i = 0; i < dataLength; ++i) {
      uint8_t currentByte = userData->data[dataIdx + i];
      if (i < 8) {
          dataRecord->data[i] = currentByte;
      }
  }

  *startOfDataRecordIdx = dataIdx + dataLength;
}

void Kamstrup303WA02::copyDataToTargetBuffer(VariableDataRecord* dataRecord, void* targetBuffer) {
    uint8_t *byteTargetBuffer = reinterpret_cast<uint8_t*>(targetBuffer);
    for (uint8_t i = 0; i < dataRecord->dataLength; ++i) {
        *byteTargetBuffer++ = dataRecord->data[i];
    }
}

bool Kamstrup303WA02::DataLinkLayer::reqUd2(uint8_t address, Kamstrup303WA02::DataLinkLayer::TelegramData * const dataBuffer) {
  ESP_LOGD(TAG, "reqUd2 - enter");
	bool success = false;
	// Initialize slave if required, and check for successful init
	if (!slaveInitialized) {
		slaveInitialized = sndNke(address);
		if (!slaveInitialized) {
      ESP_LOGE(TAG, "SND_NKE: No or no correct answer!");
			return false;
		} else {
      ESP_LOGD(TAG, "SND_NKE: Success");
    }
	}

	// Short Frame, expect RSP_UD
	uint8_t c = (1 << CFieldBitDirection) | (nextReqUd2Fcb << CFieldBitFCB) | (1 << CFieldBitFCV) | (CFieldFunctionReqUd2);
  ESP_LOGD(TAG, "About to send short frame");
	bool dataIsReceived = trySendShortFrame(c, address);
	ESP_LOGD(TAG, "returned from sending short frame");

	if (dataIsReceived) {
		// We expect either a Control Frame, or a Long Frame. Handle as the same!
		// Expected fields:
		// Start, L, L, Start, C, A, CI, Checksum, Stop
		uint8_t receivedByte {0};
    uartDevice->read_byte(&receivedByte);
		if (StartByteControlAndLongFrame != receivedByte) {
      ESP_LOGE(TAG, "Incorrect start byte! %dX", receivedByte);
			return false;
		}
    ESP_LOGD(TAG, "Correct start byte");

		uint8_t receivedL {0};
    if (!readNextByte(&receivedL)) {
      return false;
    }
		dataBuffer->dataLength = receivedL - 3;
		if (!readNextByte(&receivedByte) || receivedByte != receivedL) {
			return false;
		}
		if (!readNextByte(&receivedByte) || StartByteControlAndLongFrame != receivedByte) {
			return false;
		}
		if (!readNextByte(&dataBuffer->c)) {
      return false;
    }
		if (!readNextByte(&dataBuffer->a) || dataBuffer->a != address) {
			return false;
		}
		if (!readNextByte(&dataBuffer->ci)) {
      return false;
    }

    ESP_LOGD(TAG, "Reading telegram data bytes");
    for (uint8_t userDataByteIdx = 0; userDataByteIdx < receivedL - 3; ++userDataByteIdx) {
			if (!readNextByte(&dataBuffer->data[userDataByteIdx])) {
        ESP_LOGE(TAG, "Could not read next byte");
        return false;
      }
		}

		uint8_t calculatedChecksum = calculateChecksum(reinterpret_cast<uint8_t*>(dataBuffer), receivedL);
		uint8_t receivedChecksum {0};
		if (!readNextByte(&receivedChecksum) || calculatedChecksum != receivedChecksum) {
      ESP_LOGE(TAG, "Received (or not) checksum: %X, calculated checksum: %X", receivedChecksum, calculatedChecksum);
			return false;
		}
		if (!readNextByte(&receivedByte) || StopByte != receivedByte) {
      ESP_LOGE(TAG, "Received (or not) stop byte: %X", receivedByte);
			return false;
		}
		
		success = true;
		nextReqUd2Fcb = !nextReqUd2Fcb;
	}
	
	return success;
}

bool Kamstrup303WA02::DataLinkLayer::readNextByte(uint8_t* pReceivedByte) {
  uint32_t timeBeforeStartingToWait = millis();
	while (uartDevice->available() == 0) {
    delay(1);
    if (millis() - timeBeforeStartingToWait > 150) {
      ESP_LOGE(TAG, "NO DATA AVAILABLE AFTER TIMEOUT");
      return false;
    }
  }
  uartDevice->read_byte(pReceivedByte);
	return true;
}

bool Kamstrup303WA02::DataLinkLayer::sndNke(uint8_t address) {
  bool success = false;
  // Short Frame, expect 0xE5
  uint8_t c = (1 << CFieldBitDirection) | CFieldFunctionSndNke;
  bool dataIsReceived = trySendShortFrame(c, address);
  if (dataIsReceived) {
    uint8_t receivedByte {0};
    uartDevice->read_byte(&receivedByte);
    success = StartByteSingleCharacter == receivedByte;
    if (!success) {
      ESP_LOGI(TAG, "WRONG ANSWER TO SND_NKE!");
    }
  } else {
    ESP_LOGI(TAG, "No answer to SND_NKE!");
  }
  nextReqUd2Fcb = true;
  nextSndUdFcb = true;
  return success;
}

bool Kamstrup303WA02::DataLinkLayer::trySendShortFrame(uint8_t c, uint8_t a) {
  bool success = false;
  
  bool dataIsReceived = false;
  flushRxBuffer();
  for (uint8_t transmitAttempt = 0; transmitAttempt < 3 && !dataIsReceived; ++transmitAttempt) {
    if (transmitAttempt > 0) {
      ESP_LOGD(TAG, "RETRY TRANSMIT SHORT FRAME");
    }
    sendShortFrame(c, a);
    ESP_LOGD(TAG, "Sent short frame. Now waiting for incoming data");
    dataIsReceived = waitForIncomingData();
  }
  success = dataIsReceived;
  return success;
}

void Kamstrup303WA02::DataLinkLayer::flushRxBuffer() {
  while (uartDevice->available()) {
    uint8_t byteCountInBuffer = uartDevice->available();
    uint8_t bytesInBuffer[byteCountInBuffer];
    uartDevice->read_array(bytesInBuffer, byteCountInBuffer);
  }
}

void Kamstrup303WA02::DataLinkLayer::sendShortFrame(uint8_t c, uint8_t a) {
  uint8_t data[] = { c, a };
  uint8_t checksum = calculateChecksum(data, 2);
  uint8_t shortFrame[] = { StartByteShortFrame, c, a, checksum, StopByte };
  uartDevice->write_array(shortFrame, 5);
  delay(1);
  uartDevice->flush();
  delay(1);
}

bool Kamstrup303WA02::DataLinkLayer::waitForIncomingData() {
  bool dataReceived = false;
  // 330 bits + 50ms = 330 * 1000 / 2400 + 50 ms = 187,5 ms
  delay(138);
  ESP_LOGD(TAG, "waitForIncomingData - after long delay");
  for (uint16_t i = 0; i < 500; ++i) {
    if (uartDevice->available() > 0) {
      dataReceived = true;
      ESP_LOGD(TAG, "waitForIncomingData - loop - data received!");
      break;
    } else {
      ESP_LOGD(TAG, "waitForIncomingData - loop - NO data received");
    }
    delay(1);
  }
  if (!dataReceived) {
    ESP_LOGE(TAG, "waitForIncomingData - exit - NO data received...");
  }
  return dataReceived;
}

uint8_t Kamstrup303WA02::DataLinkLayer::calculateChecksum(const uint8_t data[], uint8_t length) {
  uint8_t checksum = 0;
  for (uint8_t i = 0; i < length; ++i) {
    checksum += data[i];
  }
  return checksum;
}

} //namespace warmtemetermbus
} //namespace esphome