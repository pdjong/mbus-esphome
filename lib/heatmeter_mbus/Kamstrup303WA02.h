#ifndef KAMSTRUP303WA02_H_
#define KAMSTRUP303WA02_H_

#ifndef UNIT_TEST
#include "esphome/core/datatypes.h"
#include "esphome/components/uart/uart.h"
#endif // UNIT_TEST

namespace esphome {
namespace heatmeter_mbus {

class UartInterface {
  public:
    virtual bool read_byte(uint8_t* data) = 0;
    virtual bool read_array(uint8_t* data, size_t len) = 0;
    virtual bool write_array(const uint8_t* data, size_t len) = 0;
    virtual int available() const = 0;
    virtual void flush() = 0;
};

class Kamstrup303WA02 {
  public:
    class DataLinkLayer {
      public:
        typedef struct LongFrame {
          uint8_t l;
          uint8_t c;
          uint8_t a;
          uint8_t ci;
          // uint8_t check_sum;
          uint8_t* user_data;
        } LongFrame;
        DataLinkLayer(UartInterface* uart_interface) : uart_interface_(uart_interface) {}
        bool req_ud2(const uint8_t address, LongFrame* response_frame);
        bool snd_nke(const uint8_t address);

      protected:
        bool try_send_short_frame(const uint8_t c, const uint8_t a);
        void flush_rx_buffer();
        void send_short_frame(const uint8_t c, const uint8_t a);
        bool wait_for_incoming_data();
        uint8_t calculate_checksum(const uint8_t* data, size_t length) const;
        
        UartInterface* uart_interface_;
        bool next_req_ud2_fcb_ { true };

      private:
        const uint8_t START_BYTE_SINGLE_CHARACTER = 0xE5;
        const uint8_t START_BYTE_SHORT_FRAME = 0x10;
        const uint8_t START_BYTE_CONTROL_AND_LONG_FRAME = 0x68;
        const uint8_t STOP_BYTE = 0x16;
        const uint8_t C_FIELD_BIT_DIRECTION = 6;
        const uint8_t C_FIELD_BIT_FCB = 5;
        const uint8_t C_FIELD_BIT_FCV = 4;
        const uint8_t C_FIELD_FUNCTION_SND_NKE = 0x0;
        const uint8_t C_FIELD_FUNCTION_REQ_UD2 = 0xB;

        bool parse_long_frame_response(LongFrame* longFrame);
        bool read_next_byte(uint8_t* received_byte);
    };

};

// class Kamstrup303WA02 {
// public:
//   typedef struct VariableDataHeader {
//       uint32_t identNr;
//       uint16_t manufacturer;
//       uint8_t version;
//       uint8_t medium;
//       uint8_t accessNr;
//       uint8_t status;
//       uint16_t signature;
//   } VariableDataHeader;

//   typedef struct VariableDataRecord {
//     unsigned function : 2;
//     unsigned dataType : 4;
//     unsigned long long storageNumber : 41;
//     unsigned long tariff : 20;
//     unsigned subUnit : 10;
//     unsigned unitAndMultiplier : 7;
//     uint8_t data[8];
//     uint8_t dataLength;
//   } VariableDataRecord;

//   typedef enum EnergyUnit {
//       Wh,
//       J
//   } EnergyUnit;

//   typedef enum PowerUnit {
//       W,
//       JperH
//   } PowerUnit;

//   typedef enum DurationUnit {
//     seconds = 0x0,
//     minutes = 0x1,
//     hours = 0x2,
//     days = 0x3
//   } DurationUnit;

//   typedef enum VolumeFlowUnit {
//       m3PerHour,
//       m3PerMinute,
//       m3PerSecond
//   } VolumeFlowUnit;
      
//   typedef struct EnergyValue {
//       uint32_t value;
//       EnergyUnit unit;
//       int8_t tenPower;
//   } EnergyValue;

//   typedef struct VolumeValue {
//       uint32_t value;
//       // No unit required; always m3
//       int8_t tenPower;
//   } VolumeValue;

//   typedef struct EnergyE8E9Value {
//       uint32_t value;
//       // No unit required; always cubic m * deg C
//       int8_t tenPower;
//   } EnergyE8E9Value;

//   typedef struct DurationValue {
//       uint32_t value;
//       DurationUnit unit;
//       // No multiplier!
//   } DurationValue;

//   typedef struct TemperatureValue {
//       int16_t value;
//       // No unit required; always deg C
//       int8_t tenPower;
//   } TemperatureValue;

//   typedef struct PowerValue {
//       uint16_t value;
//       PowerUnit unit;
//       int8_t tenPower;
//   } PowerValue;

//   typedef struct VolumeFlowValue {
//       uint16_t value;
//       VolumeFlowUnit unit;
//       int8_t tenPower;
//   } VolumeFlowValue;

//   typedef struct InfoBitsValue {
//       unsigned noVoltageSupply : 1;
//       unsigned lowBatteryLevel : 1;
//       unsigned notUsed1 : 1;
//       unsigned t1AboveRangeOrDisconnected : 1;
//       unsigned t2AboveRangeOrDisconnected : 1;
//       unsigned t1BelowRangeOrShirtCircuited : 1;
//       unsigned t2BelowRangeOrShirtCircuited : 1;
//       unsigned invalidTempDifference : 1;
//       unsigned v1Air : 1;
//       unsigned v1WrongFlowDirection : 1;
//       unsigned notUsed2 : 1;
//       unsigned v1GreaterThanQsMoreThanHour : 1;
//   } InfoBitsValue;

//   typedef struct DateValue {
//       uint8_t day;
//       uint8_t month;
//       uint8_t year;
//   } DateValue;

//   typedef struct MeterData {
//     EnergyValue heatEnergyE1;
//     VolumeValue volumeV1;
//     EnergyE8E9Value energyE8;
//     EnergyE8E9Value energyE9;
//     DurationValue operatingHours;
//     DurationValue errorHourCounter;
//     TemperatureValue t1Actual;
//     TemperatureValue t2Actual;
//     TemperatureValue diffT1T2;
//     PowerValue powerE1OverE3Actual;
//     PowerValue powerMaxMonth;
//     VolumeFlowValue flowV1Actual;
//     VolumeFlowValue flowV1MaxMonth;
//     InfoBitsValue infoBits;
//     EnergyValue heatEnergyE1Old;
//     VolumeValue volumeV1Old;
//     EnergyE8E9Value energyE8Old;
//     EnergyE8E9Value energyE9Old;
//     PowerValue powerMaxYear;
//     VolumeFlowValue flowV1MaxYear;
//     DateValue dateTimeLogged;
//   } MeterData;

// private:
//   class DataLinkLayer {
//   public:
//     typedef struct TelegramData {
//       uint8_t c;
//       uint8_t a;
//       uint8_t ci;
//       uint8_t data[246];
//       uint8_t dataLength;
//     } TelegramData;

//   public:
//     DataLinkLayer(uart::UARTDevice* _uartDevice) : uartDevice(_uartDevice) {}

//   private:
//     // Constants
//     const uint8_t CFieldBitDirection = 6;
//     const uint8_t CFieldBitFCB = 5;
//     const uint8_t CFieldBitFCV = 4;
//     const uint8_t CFieldFunctionSndNke = 0x0;
//     const uint8_t CFieldFunctionSndUd = 0x3;
//     const uint8_t CFieldFunctionReqUd2 = 0xB;
//     const uint8_t CFieldFunctionReqUd1 = 0xA;
//     const uint8_t CFieldFunctionRspUd = 0x8;
//     const uint8_t StartByteSingleCharacter = 0xE5;
//     const uint8_t StartByteShortFrame = 0x10;
//     const uint8_t StartByteControlAndLongFrame = 0x68;
//     const uint8_t StopByte = 0x16;

//     // Fields
//     bool slaveInitialized {false};
//     bool nextReqUd2Fcb { false };
//     bool nextSndUdFcb { false };
//     uart::UARTDevice* uartDevice;

//     // Methods
//     bool readNextByte(uint8_t * const receivedByte);
//     bool sndNke(const uint8_t address);
//     bool trySendShortFrame(const uint8_t c, const uint8_t a);
//     void sendShortFrame(const uint8_t c, const uint8_t a);
//     bool waitForIncomingData();
//     void flushRxBuffer();
//     uint8_t calculateChecksum(const uint8_t data[], const uint8_t length);

//   public:
//     bool reqUd2(const uint8_t address, TelegramData * const dataBuffer);
//   };

// public:
//   Kamstrup303WA02(uart::UARTDevice* _uartDevice) : dataLinkLayer(_uartDevice) {}

//   bool readData(MeterData * const data);

// private:
//   DataLinkLayer dataLinkLayer;

//   void readDataRecord(VariableDataRecord * const dataRecord, const DataLinkLayer::TelegramData * const userData, uint16_t * const startOfDataRecordIdx);
//   void copyDataToTargetBuffer(VariableDataRecord* dataRecord, void* targetBuffer);
// };

} //namespace heatmeter_mbus
} //namespace esphome

#endif // KAMSTRUP303WA02_H_